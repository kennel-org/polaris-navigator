/*
 * CompassDisplay.h
 * 
 * Compass display interface for the Polaris Navigator
 * Handles display of compass heading and celestial objects
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef COMPASS_DISPLAY_H
#define COMPASS_DISPLAY_H

#include <M5Unified.h>
// #include <FastLED.h>
#include "CelestialOverlay.h"
#include "DisplayModes.h"

// Color definitions
#define COLOR_RED    0xFF0000
#define COLOR_GREEN  0x00FF00
#define COLOR_BLUE   0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_PURPLE 0xFF00FF
#define COLOR_CYAN   0x00FFFF
#define COLOR_WHITE  0xFFFFFF
#define COLOR_BLACK  0x000000

class CompassDisplay {
public:
  // Constructor
  CompassDisplay();
  
  // Initialize display
  void begin();
  
  // Update display based on current mode
  void update(int mode, bool gpsValid, bool imuCalibrated);
  
  // Display polar alignment compass
  void showPolarAlignment(float heading, float polarisAz, float polarisAlt, 
                         float pitch, float roll);
  
  // Display GPS data
  void showGPSData(float latitude, float longitude, float altitude,
                  int satellites, float hdop, int hour, int minute);
  
  // Display IMU data
  void showIMUData(float heading, float pitch, float roll);
  
  // Display celestial data
  void showCelestialData(float sunAz, float sunAlt, float moonAz, float moonAlt, float moonPhase);
  
  // Enhanced celestial display with overlay
  void showCelestialOverlay(float heading, float latitude, float longitude,
                           int year, int month, int day, int hour, int minute, int second);
  
  // Display settings
  void showSettings();
  
  // Display calibration
  void showCalibration(int stage, float progress);
  
  // Show welcome screen
  void showWelcome();
  
  // Show error message
  void showError(const char* message);
  
private:
  // Helper methods
  uint32_t getAlignmentColor(float angleDiff);
  void setPixelColor(uint32_t color);
  void blinkPixel(uint32_t color1, uint32_t color2, int count, int delayMs);
  
  // Animation helpers
  void pulsePixel(uint32_t color, int duration);
  void rotatePixel(uint32_t color, int duration, int direction);
  
  // Celestial overlay helper
  void updateCelestialOverlay(float latitude, float longitude,
                             int year, int month, int day, int hour, int minute, int second);
  
  // State variables
  uint32_t _currentColor;
  unsigned long _lastAnimationTime;
  
  // Celestial overlay
  CelestialOverlay _celestialOverlay;
};

#endif // COMPASS_DISPLAY_H
