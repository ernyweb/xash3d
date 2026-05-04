/*
license.h - client-side license state and validation helpers
Copyright (C) 2026 xash3d adaptation

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef LICENSE_H
#define LICENSE_H

#include "common.h"

typedef struct license_info_s
{
    char user_id[64];
    char plan[32];
    uint32_t expires;  // Will be XORed with validation seed
    char device_id[64];
    char nonce[64];
    uint32_t checksum; // Integrity check
    qboolean valid;
} license_info_t;

void License_Init( void );
void License_SetKey( const char *key );
const char *License_GetStatus( void );
qboolean License_ValidateOfflineToken( const char *token );
const char *License_GetDeviceId( void );
qboolean License_IsValid( void );
void License_DrawMenu( void );
void License_ProcessInput( void );
qboolean License_ShouldShowMenu( void );
void License_Enforce( void );

#endif // LICENSE_H
