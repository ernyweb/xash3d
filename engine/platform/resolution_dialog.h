/*
resolution_dialog.h - Simple resolution selection dialog for startup
*/

#ifndef RESOLUTION_DIALOG_H
#define RESOLUTION_DIALOG_H

typedef struct {
    int width;
    int height;
    const char *name;
} ResolutionOption;

// Resolution presets
static const ResolutionOption resolution_options[] = {
    { 1920, 1080, "1920x1080 - Full HD (High-end)" },
    { 1280, 720,  "1280x720 - HD (Mid-range)" },
    { 1024, 768,  "1024x768 - XGA (Balanced)" },
    { 960, 540,   "960x540 - QHD (Medium)" },
    { 854, 480,   "854x480 - 854p (Lower resource)" },
    { 800, 480,   "800x480 - WVGA (Mobile standard)" },
    { 768, 432,   "768x432 - Half-HD (Compact)" },
    { 720, 480,   "720x480 - NTSC (Standard)" },
    { 640, 480,   "640x480 - VGA (Retro)" },
    { 640, 360,   "640x360 - nHD (Minimal)" },
    { 480, 270,   "480x270 - HVGA (Very low)" },
    { 0, 0, NULL }  // Terminator
};

#define NUM_RESOLUTION_OPTIONS 11

#endif // RESOLUTION_DIALOG_H
