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
  
  // Update display based on current mode
  void update(int mode, bool gpsValid, bool imuCalibrated);
  
  // Display welcome screen
  void showWelcome();
  
  // Display compass
  void showCompass(float heading, float pitch, float roll, bool gpsValid, bool imuCalibrated);
  
  // Display GPS data
  void showGPS(float latitude, float longitude, float altitude, int satellites, float hdop, bool gpsValid, bool imuCalibrated);
  
  // Display IMU data
  void showIMU();
  
  // Set IMU data
  void setIMUData(float gyroX, float gyroY, float gyroZ, 
                 float accelX, float accelY, float accelZ,
                 float magX, float magY, float magZ);
                
  // Display calibration
  void showCalibration(int calibrationStage, int8_t progress);
  
  // Display polar alignment compass
  void showPolarAlignment(float heading, float polarisAz, float polarisAlt, float pitch, float roll);
  
  // Display celestial data
  void showCelestial(float latitude, float longitude, float heading, bool gpsValid, bool imuCalibrated);
  
  // Display celestial data with sun and moon positions
  void showCelestialData(float sunAzimuth, float sunAltitude, float moonAzimuth, float moonAltitude, int moonPhase);
  
  // Display celestial overlay with current position and time
  void showCelestialOverlay(float heading, float latitude, float longitude, 
                           int year, int month, int day, int hour, int minute, int second);
  
  // Display IMU data (heading, pitch, roll)
  void showIMUData(float heading, float pitch, float roll);
  
  // Display GPS data
  void showGPSData(float latitude, float longitude, float altitude, int satellites, float hdop, int hour, int minute);
  
  // Display settings screen
  void showSettings();
  
  // Show error message
  void showError(const char* message);
  
  // Set pixel color (for RGB LED)
  void setPixelColor(uint32_t color);
  
  // Animation helpers
  void pulsePixel(uint32_t color, int duration);
  void rotatePixel(uint32_t color, int duration, int direction);
  
private:
  // Helper methods
  uint32_t getAlignmentColor(float angleDiff);
  void blinkPixel(uint32_t color1, uint32_t color2, int count, int delayMs);
  int getMoonIllumination(int moonPhase);
  
  // Celestial overlay helper
  void updateCelestialOverlay(float latitude, float longitude,
                             int year, int month, int day, int hour, int minute, int second);
  
  // State variables
  uint32_t _currentColor;
  unsigned long _lastAnimationTime;
  
  // Sprite for double-buffered rendering
  LGFX_Sprite* _sprite;
  
  // Celestial overlay
  CelestialOverlay _celestialOverlay;
  
  // Display mode
  DisplayMode _currentMode;
  
  // IMUデータ
  IMUData _imuData;
  
  // ユーティリティ関数
  void drawCompassRose(float heading);
  void drawHorizon(float pitch, float roll);
};

#endif // COMPASS_DISPLAY_H
