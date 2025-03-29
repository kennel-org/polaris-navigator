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
#include "CelestialOverlay.h"
#include "DisplayModes.h"
#include "AtomicBaseGPS.h"

// Color definitions
#define COLOR_RED    0xFF0000
#define COLOR_GREEN  0x00FF00
#define COLOR_BLUE   0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_PURPLE 0xFF00FF
#define COLOR_CYAN   0x00FFFF
#define COLOR_WHITE  0xFFFFFF
#define COLOR_BLACK  0x000000

// IMUデータ構造体
struct IMUData {
  // ジャイロスコープデータ (BMI270)
  float gyroX;
  float gyroY;
  float gyroZ;
  
  // 加速度計データ (BMI270)
  float accelX;
  float accelY;
  float accelZ;
  
  // 磁力計データ (BMM150)
  float magX;
  float magY;
  float magZ;
};

class CompassDisplay {
public:
  // Constructor
  CompassDisplay();
  
  // Initialize display
  void begin();
  
  // Show welcome screen
  void showWelcome();
  
  // Display compass
  void showCompass(float heading, float pitch, float roll, bool gpsValid, bool imuCalibrated);
  
  // Display polar alignment compass
  void showPolarAlignment(float heading, float polarisAz, float polarisAlt, float pitch, float roll);
  
  // Display celestial overlay
  void showCelestialOverlay(float heading, float pitch, float roll, 
                          float sunAz, float sunAlt, 
                          float moonAz, float moonAlt, float moonPhase);
  
  // Display GPS information
  void showGPS(float latitude, float longitude, float altitude, int satellites, float hdop);
  
  // Display GPS invalid message
  void showGPSInvalid();
  
  // Display IMU data
  void showIMU();
  
  // Display error message
  void showError(const char* message);
  
private:
  // Canvas for double buffering
  M5Canvas _canvas = M5Canvas(&M5.Display);
  
  // Current animation color
  uint32_t _currentColor;
  
  // Last animation time
  unsigned long _lastAnimationTime;
  
  // Celestial overlay
  CelestialOverlay _celestialOverlay;
  
  // Draw compass rose
  void drawCompassRose(float heading);
  
  // Draw horizon line
  void drawHorizon(float pitch, float roll);
  
  // Set pixel color (for RGB LED)
  void setPixelColor(uint32_t color);
  
  // Blink NeoPixel
  void blinkPixel(uint32_t color1, uint32_t color2, int count, int duration);
  
  // State variables
  DisplayMode _currentMode;
  
  // IMUデータ
  IMUData _imuData;
  
  // Double buffering
  void swapBuffers();
};

#endif // COMPASS_DISPLAY_H
