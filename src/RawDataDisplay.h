/*
 * RawDataDisplay.h
 * 
 * Raw data display for the Polaris Navigator
 * Handles detailed sensor data visualization
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef RAW_DATA_DISPLAY_H
#define RAW_DATA_DISPLAY_H

#include <M5Unified.h>
#include "BMI270.h"
#include "BMM150class.h"
#include "AtomicBaseGPS.h"
#include "DisplayModes.h"

class RawDataDisplay {
public:
  // Constructor
  RawDataDisplay();
  
  // Initialize display
  void begin();
  
  // Update display with raw data
  void update(RawDataMode mode);
  
  // Display raw IMU data
  void showRawIMU(BMI270* bmi270, BMM150class* bmm150, 
                 float heading, float pitch, float roll,
                 bool calibrated);
  
  // Display raw GPS data
  void showRawGPS(AtomicBaseGPS* gps, 
                 float latitude, float longitude, float altitude,
                 int satellites, float hdop, 
                 int hour, int minute, int second,
                 bool valid);
  
  // Display raw celestial data
  void showRawCelestial(float sunAz, float sunAlt, 
                       float moonAz, float moonAlt, float moonPhase,
                       float polarisAz, float polarisAlt);
  
  // Display system information
  void showSystemInfo(float batteryLevel, float temperature, 
                     unsigned long uptime, int freeMemory);
  
  // Display debug information
  void showDebugInfo(const char* debugMessage);
  
  // Toggle detailed view
  void toggleDetailedView();
  
  // Set LED color based on data quality
  void setDataQualityIndicator(float quality);
  
  // Set LED color for AtomS3R
  void setPixelColor(uint32_t color);
  
private:
  // Helper methods
  void formatFloatValue(char* buffer, float value, int precision);
  void formatTimeValue(char* buffer, int hours, int minutes, int seconds);
  void formatDateValue(char* buffer, int year, int month, int day);
  void formatCoordinateValue(char* buffer, float value, bool isLatitude);
  
  // Serial output helpers
  void printRawIMUData(BMI270* bmi270, BMM150class* bmm150);
  void printRawGPSData(AtomicBaseGPS* gps);
  void printRawCelestialData(float sunAz, float sunAlt, 
                            float moonAz, float moonAlt, float moonPhase,
                            float polarisAz, float polarisAlt);
  void printSystemInfo(float batteryLevel, float temperature, 
                      unsigned long uptime, int freeMemory);
  
  // State variables
  bool _detailedView;
  unsigned long _lastUpdateTime;
  int _currentPage;
  RawDataMode _currentMode;
};

#endif // RAW_DATA_DISPLAY_H
