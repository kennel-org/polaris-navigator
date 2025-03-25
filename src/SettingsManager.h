/*
 * SettingsManager.h
 * 
 * Manages user settings for the Polaris Navigator
 * Handles settings storage and retrieval
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <M5Unified.h>
#include <Preferences.h>

// Display brightness levels
enum BrightnessLevel {
  BRIGHTNESS_LOW,
  BRIGHTNESS_MEDIUM,
  BRIGHTNESS_HIGH
};

// Location source
enum LocationSource {
  LOCATION_GPS,
  LOCATION_MANUAL
};

// Time source
enum TimeSource {
  TIME_GPS,
  TIME_MANUAL,
  TIME_NTP
};

// Settings structure
struct UserSettings {
  // Display settings
  BrightnessLevel brightness;
  bool nightMode;
  
  // Location settings
  LocationSource locationSource;
  float manualLatitude;
  float manualLongitude;
  float manualAltitude;
  
  // Time settings
  TimeSource timeSource;
  int timeZoneOffset; // in minutes
  bool useDST;
  
  // Compass settings
  bool useNorthReference; // true = true north, false = magnetic north
  float manualDeclination; // manual magnetic declination in degrees
  
  // Power settings
  int sleepTimeout; // in seconds, 0 = never sleep
  bool enableBluetooth;
  
  // Debug settings
  bool enableDebugOutput;
  bool enableDataLogging;
};

class SettingsManager {
public:
  // Constructor
  SettingsManager();
  
  // Initialize settings manager
  void begin();
  
  // Load settings from storage
  bool loadSettings();
  
  // Save settings to storage
  bool saveSettings();
  
  // Reset settings to defaults
  void resetSettings();
  
  // Get current settings
  UserSettings getSettings();
  
  // Update settings
  void updateSettings(UserSettings newSettings);
  
  // Individual setting getters
  BrightnessLevel getBrightness();
  bool getNightMode();
  LocationSource getLocationSource();
  float getManualLatitude();
  float getManualLongitude();
  float getManualAltitude();
  TimeSource getTimeSource();
  int getTimeZoneOffset();
  bool getUseDST();
  bool getUseNorthReference();
  float getManualDeclination();
  int getSleepTimeout();
  bool getEnableBluetooth();
  bool getEnableDebugOutput();
  bool getEnableDataLogging();
  
  // Individual setting setters
  void setBrightness(BrightnessLevel brightness);
  void setNightMode(bool nightMode);
  void setLocationSource(LocationSource source);
  void setManualLocation(float latitude, float longitude, float altitude);
  void setTimeSource(TimeSource source);
  void setTimeZoneOffset(int offset);
  void setUseDST(bool useDST);
  void setUseNorthReference(bool useNorthReference);
  void setManualDeclination(float declination);
  void setSleepTimeout(int timeout);
  void setEnableBluetooth(bool enable);
  void setEnableDebugOutput(bool enable);
  void setEnableDataLogging(bool enable);
  
  // Apply settings
  void applySettings();
  
private:
  // Settings data
  UserSettings _settings;
  
  // Storage for settings
  Preferences _preferences;
  
  // Helper methods
  void applyDisplaySettings();
  void applyPowerSettings();
};

#endif // SETTINGS_MANAGER_H
