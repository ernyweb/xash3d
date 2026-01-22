/*
vid_common.c - common vid component
Copyright (C) 2018 a1batross, Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#include "client.h"
#include "mod_local.h"
#include "input.h"
#include "vid_common.h"
#include "platform/platform.h"

static CVAR_DEFINE_AUTO( vid_mode, "0", FCVAR_RENDERINFO, "current video mode index (used only for storage)" );
static CVAR_DEFINE_AUTO( vid_rotate, "0", FCVAR_RENDERINFO|FCVAR_VIDRESTART, "screen rotation (0-3)" );
static CVAR_DEFINE_AUTO( vid_scale, "1.0", FCVAR_RENDERINFO|FCVAR_VIDRESTART, "pixel scale" );

CVAR_DEFINE_AUTO( vid_maximized, "0", FCVAR_RENDERINFO, "window maximized state, read-only" );
CVAR_DEFINE( vid_fullscreen, "fullscreen", DEFAULT_FULLSCREEN, FCVAR_RENDERINFO|FCVAR_VIDRESTART, "fullscreen state (0 windowed, 1 fullscreen, 2 borderless)" );
CVAR_DEFINE( window_width, "width", "0", FCVAR_RENDERINFO|FCVAR_VIDRESTART, "screen width" );
CVAR_DEFINE( window_height, "height", "0", FCVAR_RENDERINFO|FCVAR_VIDRESTART, "screen height" );
CVAR_DEFINE( window_xpos, "_window_xpos", "-1", FCVAR_RENDERINFO, "window position by horizontal" );
CVAR_DEFINE( window_ypos, "_window_ypos", "-1", FCVAR_RENDERINFO, "window position by vertical" );
CVAR_DEFINE( vid_width, "vid_width", "0", FCVAR_READ_ONLY, "actual window viewport size" );
CVAR_DEFINE( vid_height, "vid_height", "0", FCVAR_READ_ONLY, "actual window viewport size" );

glwstate_t	glw_state;

/*
=================
R_SaveVideoMode
=================
*/
void R_SaveVideoMode( int w, int h, int render_w, int render_h, qboolean maximized )
{
	string temp;

	Con_Reportf( "R_SaveVideoMode: Saving mode w=%d h=%d render_w=%d render_h=%d maximized=%d\n", w, h, render_w, render_h, maximized );

	if( !w || !h || !render_w || !render_h )
	{
		Con_Reportf( S_ERROR "R_SaveVideoMode: Invalid dimensions, bailing out\n" );
		host.renderinfo_changed = false;
		return;
	}

	host.window_center_x = w / 2;
	host.window_center_y = h / 2;

	Q_snprintf( temp, sizeof( temp ), "%d", w );
	Cvar_DirectSet( &window_width, temp );

	Q_snprintf( temp, sizeof( temp ), "%d", h );
	Cvar_DirectSet( &window_height, temp );

	Q_snprintf( temp, sizeof( temp ), "%d", render_w );
	Cvar_FullSet( "vid_width", temp, vid_width.flags );

	Q_snprintf( temp, sizeof( temp ), "%d", render_h );
	Cvar_FullSet( "vid_height", temp, vid_width.flags  );

	Cvar_DirectSet( &vid_maximized, maximized ? "1" : "0" );
	
	// immediately drop changed state or we may trigger
	// video subsystem to reapply settings
	host.renderinfo_changed = false;

	if( refState.width == render_w && refState.height == render_h )
	{
		Con_Reportf( "R_SaveVideoMode: Resolution already matches, skipping SCR_VidInit\n" );
		return;
	}

	refState.scale_x = (float)render_w / w;
	refState.scale_y = (float)render_h / h;

	refState.width = render_w;
	refState.height = render_h;

	// check for 4:3 or 5:4
	refState.wideScreen = render_w * 3 != render_h * 4 && render_w * 4 != render_h * 5;

	SCR_VidInit(); // tell client.dll that vid_mode has changed
}

/*
=================
VID_GetModeString
=================
*/
const char *VID_GetModeString( int vid_mode )
{
	vidmode_t *vidmode;
	if( vid_mode < 0 || vid_mode >= R_MaxVideoModes() )
		return NULL;

	if( !( vidmode = R_GetVideoMode( vid_mode ) ) )
		return NULL;

	return vidmode->desc;
}

/*
==================
VID_CheckChanges

check vid modes and fullscreen
==================
*/
void VID_CheckChanges( void )
{
	if( FBitSet( cl_allow_levelshots.flags, FCVAR_CHANGED ))
	{
		//GL_FreeTexture( cls.loadingBar );
		SCR_RegisterTextures(); // reload 'lambda' image
		ClearBits( cl_allow_levelshots.flags, FCVAR_CHANGED );
	}

	if( host.renderinfo_changed )
	{
		Con_Reportf( "VID_CheckChanges: renderinfo_changed is set, calling VID_SetMode\n" );
		if( VID_SetMode( ))
		{
			SCR_VidInit(); // tell the client.dll what vid_mode has changed
		}
		else
		{
			Sys_Error( "Can't re-initialize video subsystem\n" );
		}
		host.renderinfo_changed = false;
	}
}

/*
===============
VID_SetDisplayTransform

notify ref dll about screen transformations
===============
*/
void VID_SetDisplayTransform( int *render_w, int *render_h )
{
	uint rotate = vid_rotate.value;

	if( rotate < REF_ROTATE_NONE || rotate > REF_ROTATE_CCW )
		rotate = REF_ROTATE_NONE;

	if( ref.dllFuncs.R_SetDisplayTransform( rotate, 0, 0, vid_scale.value, vid_scale.value ))
	{
		if( rotate & 1 )
		{
			int swap = *render_w;

			*render_w = *render_h;
			*render_h = swap;
		}

		*render_h /= vid_scale.value;
		*render_w /= vid_scale.value;

		ref.rotation = rotate;
	}
	else
	{
		Con_Printf( S_WARN "failed to setup screen transform\n" );

		ref.rotation = REF_ROTATE_NONE;
	}
}

static void VID_Mode_f( void )
{
	int w, h;
	char cmd[128];

	switch( Cmd_Argc() )
	{
	case 2:
	{
		vidmode_t *vidmode;
		const char *arg = Cmd_Argv( 1 );

		// Check if user wants to list available modes
		if( !Q_stricmp( arg, "list" ) || !Q_stricmp( arg, "?" ))
		{
			int i;
			int total = R_MaxVideoModes();

			Con_Printf( "\nAvailable video modes:\n" );
			for( i = 0; i < total; i++ )
			{
				vidmode_t *mode = R_GetVideoMode( i );
				if( mode )
					Con_Printf( "  %d: %s\n", i, mode->desc );
			}
			Con_Printf( "\nUsage: vid_mode <modenum>|<width height>\n" );
			Con_Printf( "Example: vid_mode 0\n" );
			Con_Printf( "Example: vid_mode 1920 1080\n" );
			return;
		}

		vidmode = R_GetVideoMode( Q_atoi( arg ) );
		if( !vidmode )
		{
			Con_Printf( S_ERROR "unable to set mode, backend returned null\n" );
			Con_Printf( "Use 'vid_mode list' to see available modes\n" );
			return;
		}

		w = vidmode->width;
		h = vidmode->height;
		break;
	}
	case 3:
	{
		w = Q_atoi( Cmd_Argv( 1 ));
		h = Q_atoi( Cmd_Argv( 2 ));

		// Validate resolution
		if( w < VID_MIN_WIDTH || h < VID_MIN_HEIGHT )
		{
			Con_Printf( S_ERROR "Invalid resolution: %dx%d\n", w, h );
			Con_Printf( "Minimum resolution: %dx%d\n", VID_MIN_WIDTH, VID_MIN_HEIGHT );
			return;
		}

		Con_Printf( "Setting custom resolution: %dx%d\n", w, h );
		break;
	}
	default:
		Msg( S_USAGE "vid_mode <modenum>|<width height>|list\n" );
		Msg( "Examples:\n" );
		Msg( "  vid_mode 0        - use mode 0\n" );
		Msg( "  vid_mode 1920 1080 - custom resolution\n" );
		Msg( "  vid_mode list     - list all modes\n" );
		return;
	}

	// Use Cbuf_AddText to execute width and height commands
	// This ensures proper cvar change handling and triggers renderinfo_changed
	Q_snprintf( cmd, sizeof( cmd ), "width %d; height %d\n", w, h );
	Con_Printf( "Applying resolution: %dx%d\n", w, h );
	Cbuf_AddText( cmd );
	
	// Immediately set the flag to ensure VID_CheckChanges catches it
	// even if command buffer hasn't been executed yet
	SetBits( host.renderinfo_changed, true );
	Con_Reportf( "VID_Mode_f: Set renderinfo_changed flag\n" );
}

void VID_Init( void )
{
	// system screen width and height (don't suppose for change from console at all)
	Cvar_RegisterVariable( &window_width );
	Cvar_RegisterVariable( &window_height );

	Cvar_RegisterVariable( &vid_mode );
	Cvar_RegisterVariable( &vid_rotate );
	Cvar_RegisterVariable( &vid_scale );
	Cvar_RegisterVariable( &vid_fullscreen );
	Cvar_RegisterVariable( &vid_maximized );
	Cvar_RegisterVariable( &vid_width );
	Cvar_RegisterVariable( &vid_height );
	Cvar_RegisterVariable( &window_xpos );
	Cvar_RegisterVariable( &window_ypos );

	// a1ba: planned to be named vid_mode for compability
	// but supported mode list is filled by backends, so numbers are not portable any more
	Cmd_AddRestrictedCommand( "vid_setmode", VID_Mode_f, "display video mode" );
	Cmd_AddCommand( "vid_info", VID_Info_f, "show vid component info" );

	V_Init(); // init gamma
	R_Init(); // init renderer
}
