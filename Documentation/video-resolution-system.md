# Video Resolution System Guide

## Overview

Xash3D now includes a comprehensive video resolution system that works across all platforms, including Android. This system allows users to select from preset resolutions or input custom resolutions manually.

## Features

### Android Video Resolution Support

- **Preset Resolutions**: Common mobile resolutions (1920x1080, 1280x720, 1024x768, 800x480, 640x480, etc.)
- **Custom Resolutions**: Support for manual custom resolution input
- **Dynamic Resolution Scaling**: Resolution changes are applied in real-time without needing to restart the game
- **Screen Size Awareness**: Automatically filters out resolutions larger than the actual screen

### Desktop Platforms

- All existing resolution management features work as before
- Support for desktop display mode detection and enumeration
- Windowed, borderless fullscreen, and fullscreen modes

## Console Commands

### `vid_mode` - Set Video Mode

#### Usage

```
vid_mode <modenum>|<width height>|list
```

#### Examples

**List all available video modes:**
```
vid_mode list
vid_mode ?
```

**Set resolution by preset mode number:**
```
vid_mode 0     # Sets first preset resolution
vid_mode 5     # Sets sixth preset resolution
```

**Set custom resolution by width and height:**
```
vid_mode 1920 1080   # Set to 1920x1080
vid_mode 640 480     # Set to 640x480
vid_mode 854 480     # Set to 854x480
```

### `vid_info` - Display Video Information

Displays current video configuration including:
- Window size and render resolution
- Window position
- Window mode (windowed, fullscreen, fullscreen desktop)
- Display mode information
- Video driver information

## Console Variables (CVars)

### Video Mode Settings

- **`width`** - Screen width (read/write)
- **`height`** - Screen height (read/write)
- **`fullscreen`** - Fullscreen mode (0=windowed, 1=fullscreen, 2=borderless)
- **`vid_fullscreen`** - Current fullscreen state display
- **`vid_width`** - Actual viewport width (read-only)
- **`vid_height`** - Actual viewport height (read-only)

### Window Position

- **`_window_xpos`** - Window X position
- **`_window_ypos`** - Window Y position

### Display Settings

- **`vid_scale`** - Pixel scale factor (default: 1.0)
- **`vid_rotate`** - Screen rotation (0-3, platform-dependent)
- **`vid_maximized`** - Window maximized state (read-only)

## Preset Mobile Resolutions

The following resolutions are available on Android and other mobile platforms:

| Resolution | Type | Aspect Ratio |
|-----------|------|--------------|
| 1920x1080 | Full HD | 16:9 |
| 1280x720  | HD | 16:9 |
| 1024x768  | XGA | 4:3 |
| 960x540   | QHD | 16:9 |
| 854x480   | 854p | 16:9 |
| 800x480   | WVGA | 5:3 |
| 768x432   | Half-HD | 16:9 |
| 720x480   | NTSC | 3:2 |
| 640x480   | VGA | 4:3 |
| 640x360   | nHD | 16:9 |
| 480x270   | HVGA | 16:9 |

**Note**: On mobile platforms, only resolutions that fit within the actual screen size are shown as available options.

## Configuration Files

### video.cfg

The `video.cfg` configuration file contains video settings and is loaded at engine startup. You can create or modify this file to set your preferred video settings:

```
// Example video.cfg
width "1920"
height "1080"
fullscreen "1"
vid_scale "1.0"
```

## Android-Specific Information

### Platform Differences

- Android always runs in fullscreen mode (landscape orientation)
- Window positioning and windowed mode are not available
- Resolution changes require reloading the video subsystem

### Setting Resolution on Android

To change resolution on Android:

1. **Using Console**:
   ```
   vid_mode 1920 1080
   ```

2. **Using Configuration File**:
   Edit `video.cfg` and set width/height values

3. **Using Game Menu** (if integrated):
   Settings → Video → Resolution

### Performance Considerations

- Lower resolutions may improve performance on older devices
- Higher resolutions require more GPU/CPU resources
- Consider device capabilities when selecting resolution

## Troubleshooting

### Resolution Not Changing

1. **Check console for errors**: Use `vid_info` to verify current settings
2. **Verify resolution is valid**: Run `vid_mode list` to see available options
3. **Check Android logcat**: Look for any SDL or rendering errors
4. **Restart the application**: Some changes may require full restart

### Invalid Resolution Error

- **Error Message**: "Invalid resolution: WIDTHxHEIGHT"
- **Solution**: Resolution must be at least 320x200. Enter a larger resolution.

### Mode Not Supported

- **Error Message**: "unable to set mode, backend returned null"
- **Solution**: The selected resolution is not supported on your device. Try a different resolution from the list.

## Development Information

### Modifying Available Resolutions

The preset resolutions for mobile platforms are defined in:

- `/engine/platform/sdl2/vid_sdl2.c` (SDL2 backend)
- `/engine/platform/sdl3/vid_sdl3.c` (SDL3 backend)
- `/engine/platform/sdl1/vid_sdl1.c` (SDL1 backend)

To add custom resolutions, modify the `mobile_modes` array in `R_InitVideoModes()` function.

### Compilation Flags

For Android builds, the system automatically uses mobile-optimized resolution handling through the `XASH_MOBILE_PLATFORM` preprocessor flag.

## Related Documentation

- [Engine Features](enginefeatures.md)
- [Environment Variables](environment-variables.md)
- [GoldSrc Protocol Support](goldsrc-protocol-support.md)
