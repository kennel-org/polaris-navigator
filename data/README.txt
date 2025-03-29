# Polaris Navigator Image Assets

This directory contains image files and assets used in the Polaris Navigator.

## Image Files

- **logo.png**: Main logo image (64x64 pixels)
  - Navigator icon displayed on the splash screen
  - Previously named `navigator_icon.png`

## SPIFFS Upload Methods

1. Use Arduino IDE: "Tools" → "ESP32 Sketch Data Upload"
   or
2. For PlatformIO: use the "platformio run --target uploadfs" command

## Notes

- Image sizes below the M5 ATOMS3 screen size (128x128) are optimal
- 16-bit color (RGB565 format) PNGs are the most efficient
- Oversized images may fail to decode due to memory limitations

## File Descriptions

- **icon_description.txt**: Description document for the logo image
- **README.txt**: This file (directory description)
