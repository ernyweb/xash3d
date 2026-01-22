#!/usr/bin/env bash
# Quick Setup Guide for Xash3D Video Resolution System

# This script demonstrates how to use the new video resolution features

# 1. List all available video resolutions
echo "1. List available resolutions:"
echo "   vid_mode list"
echo ""

# 2. Set resolution by preset mode number
echo "2. Set preset resolution (e.g., mode 0):"
echo "   vid_mode 0"
echo ""

# 3. Set custom resolution manually
echo "3. Set custom resolution to 1920x1080:"
echo "   vid_mode 1920 1080"
echo ""

# 4. Set custom resolution to 854x480
echo "4. Set custom resolution to 854x480:"
echo "   vid_mode 854 480"
echo ""

# 5. Get current video information
echo "5. View current video configuration:"
echo "   vid_info"
echo ""

# 6. Set resolution via configuration file (video.cfg)
echo "6. Create video.cfg with custom settings:"
cat << 'EOF'
   width "1280"
   height "720"
   fullscreen "1"
EOF
echo ""

# Android-specific examples
echo "======== ANDROID SPECIFIC ========"
echo ""
echo "Recommended resolutions for Android devices:"
echo "  - Low-end devices:  vid_mode 640 480"
echo "  - Mid-range devices: vid_mode 1280 720"
echo "  - High-end devices: vid_mode 1920 1080"
echo ""

# Performance tips
echo "======== PERFORMANCE TIPS ========"
echo ""
echo "If experiencing performance issues:"
echo "  1. Lower the resolution: vid_mode 640 480"
echo "  2. Disable vertical sync: set gl_vsync 0"
echo "  3. Reduce scale factor: set vid_scale 1.0"
echo ""
echo "For better performance on Android:"
echo "  - Use 16:9 aspect ratio resolutions (720p, 1080p, etc.)"
echo "  - Avoid very low resolutions on high DPI displays"
echo "  - Consider device CPU/GPU capabilities"
echo ""

# Manual resolution input examples
echo "======== CUSTOM RESOLUTION EXAMPLES ========"
echo ""
echo "Common custom resolutions:"
echo "  vid_mode 2560 1440  # 2K resolution"
echo "  vid_mode 1600 1200  # UXGA"
echo "  vid_mode 1280 800   # WXGA"
echo "  vid_mode 1024 600   # nettop"
echo "  vid_mode 800 600    # SVGA"
echo "  vid_mode 640 360    # nHD"
echo ""
echo "Remember: Custom resolutions must be at least 320x200"
echo ""
