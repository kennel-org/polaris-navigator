/*
 * CompassDisplay.h
 * 
 * Display interface for the Polaris Navigator
 * Handles rendering compass and alignment information on AtomS3R display
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef COMPASS_DISPLAY_H
#define COMPASS_DISPLAY_H

#include <M5AtomS3.h>
#include <FastLED.h>

// Display modes
enum DisplayMode {
  POLAR_ALIGNMENT,  // Main compass display for polar alignment
  GPS_DATA,         // GPS data display
  IMU_DATA,         // IMU data display
  CELESTIAL_DATA,   // Celestial body data
  SETTINGS,         // Settings menu
  CALIBRATION       // Calibration mode
};

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
  void update(DisplayMode mode, bool gpsValid, bool imuCalibrated);
  
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
  
  // State variables
  uint32_t _currentColor;
  unsigned long _lastAnimationTime;
};

#endif // COMPASS_DISPLAY_H
