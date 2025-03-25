/*
 * SettingsManager.cpp
 * 
 * Implementation for the Settings Manager
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "SettingsManager.h"

// Constructor
SettingsManager::SettingsManager() {
  // Initialize with default settings
  resetSettings();
}

// Initialize settings manager
void SettingsManager::begin() {
  // Initialize preferences for storing settings
  _preferences.begin("polaris-set", false);
  
  // Try to load settings
  loadSettings();
  
  // Apply settings
  applySettings();
}

// Load settings from storage
bool SettingsManager::loadSettings() {
  // Check if settings exist
  if (!_preferences.isKey("settings_valid")) {
    Serial.println("No settings found, using defaults");
    return false;
  }
  
  // Load settings validity
  bool valid = _preferences.getBool("settings_valid", false);
  if (!valid) {
    Serial.println("Invalid settings, using defaults");
    return false;
  }
  
  // Load display settings
  _settings.brightness = (BrightnessLevel)_preferences.getUChar("brightness", BRIGHTNESS_MEDIUM);
  _settings.nightMode = _preferences.getBool("night_mode", false);
  
  // Load location settings
  _settings.locationSource = (LocationSource)_preferences.getUChar("loc_source", LOCATION_GPS);
  _settings.manualLatitude = _preferences.getFloat("manual_lat", 0.0);
  _settings.manualLongitude = _preferences.getFloat("manual_lon", 0.0);
  _settings.manualAltitude = _preferences.getFloat("manual_alt", 0.0);
  
  // Load time settings
  _settings.timeSource = (TimeSource)_preferences.getUChar("time_source", TIME_GPS);
  _settings.timeZoneOffset = _preferences.getInt("timezone", 0);
  _settings.useDST = _preferences.getBool("use_dst", false);
  
  // Load compass settings
  _settings.useNorthReference = _preferences.getBool("use_true_north", true);
  _settings.manualDeclination = _preferences.getFloat("declination", 0.0);
  
  // Load power settings
  _settings.sleepTimeout = _preferences.getInt("sleep_timeout", 300);
  _settings.enableBluetooth = _preferences.getBool("enable_bt", false);
  
  // Load debug settings
  _settings.enableDebugOutput = _preferences.getBool("debug_output", false);
  _settings.enableDataLogging = _preferences.getBool("data_logging", false);
  
  Serial.println("Settings loaded");
  return true;
}

// Save settings to storage
bool SettingsManager::saveSettings() {
  // Save settings validity
  _preferences.putBool("settings_valid", true);
  
  // Save display settings
  _preferences.putUChar("brightness", (uint8_t)_settings.brightness);
  _preferences.putBool("night_mode", _settings.nightMode);
  
  // Save location settings
  _preferences.putUChar("loc_source", (uint8_t)_settings.locationSource);
  _preferences.putFloat("manual_lat", _settings.manualLatitude);
  _preferences.putFloat("manual_lon", _settings.manualLongitude);
  _preferences.putFloat("manual_alt", _settings.manualAltitude);
  
  // Save time settings
  _preferences.putUChar("time_source", (uint8_t)_settings.timeSource);
  _preferences.putInt("timezone", _settings.timeZoneOffset);
  _preferences.putBool("use_dst", _settings.useDST);
  
  // Save compass settings
  _preferences.putBool("use_true_north", _settings.useNorthReference);
  _preferences.putFloat("declination", _settings.manualDeclination);
  
  // Save power settings
  _preferences.putInt("sleep_timeout", _settings.sleepTimeout);
  _preferences.putBool("enable_bt", _settings.enableBluetooth);
  
  // Save debug settings
  _preferences.putBool("debug_output", _settings.enableDebugOutput);
  _preferences.putBool("data_logging", _settings.enableDataLogging);
  
  Serial.println("Settings saved");
  return true;
}

// Reset settings to defaults
void SettingsManager::resetSettings() {
  // Display settings
  _settings.brightness = BRIGHTNESS_MEDIUM;
  _settings.nightMode = false;
  
  // Location settings
  _settings.locationSource = LOCATION_GPS;
  _settings.manualLatitude = 35.6762; // Tokyo
  _settings.manualLongitude = 139.6503;
  _settings.manualAltitude = 0.0;
  
  // Time settings
  _settings.timeSource = TIME_GPS;
  _settings.timeZoneOffset = 540; // UTC+9 (Tokyo)
  _settings.useDST = false;
  
  // Compass settings
  _settings.useNorthReference = true;
  _settings.manualDeclination = 0.0;
  
  // Power settings
  _settings.sleepTimeout = 300; // 5 minutes
  _settings.enableBluetooth = false;
  
  // Debug settings
  _settings.enableDebugOutput = false;
  _settings.enableDataLogging = false;
  
  Serial.println("Settings reset to defaults");
}

// Get current settings
UserSettings SettingsManager::getSettings() {
  return _settings;
}

// Update settings
void SettingsManager::updateSettings(UserSettings newSettings) {
  _settings = newSettings;
  applySettings();
  saveSettings();
}

// Individual setting getters
BrightnessLevel SettingsManager::getBrightness() {
  return _settings.brightness;
}

bool SettingsManager::getNightMode() {
  return _settings.nightMode;
}

LocationSource SettingsManager::getLocationSource() {
  return _settings.locationSource;
}

float SettingsManager::getManualLatitude() {
  return _settings.manualLatitude;
}

float SettingsManager::getManualLongitude() {
  return _settings.manualLongitude;
}

float SettingsManager::getManualAltitude() {
  return _settings.manualAltitude;
}

TimeSource SettingsManager::getTimeSource() {
  return _settings.timeSource;
}

int SettingsManager::getTimeZoneOffset() {
  return _settings.timeZoneOffset;
}

bool SettingsManager::getUseDST() {
  return _settings.useDST;
}

bool SettingsManager::getUseNorthReference() {
  return _settings.useNorthReference;
}

float SettingsManager::getManualDeclination() {
  return _settings.manualDeclination;
}

int SettingsManager::getSleepTimeout() {
  return _settings.sleepTimeout;
}

bool SettingsManager::getEnableBluetooth() {
  return _settings.enableBluetooth;
}

bool SettingsManager::getEnableDebugOutput() {
  return _settings.enableDebugOutput;
}

bool SettingsManager::getEnableDataLogging() {
  return _settings.enableDataLogging;
}

// Individual setting setters
void SettingsManager::setBrightness(BrightnessLevel brightness) {
  _settings.brightness = brightness;
  applyDisplaySettings();
  saveSettings();
}

void SettingsManager::setNightMode(bool nightMode) {
  _settings.nightMode = nightMode;
  applyDisplaySettings();
  saveSettings();
}

void SettingsManager::setLocationSource(LocationSource source) {
  _settings.locationSource = source;
  saveSettings();
}

void SettingsManager::setManualLocation(float latitude, float longitude, float altitude) {
  _settings.manualLatitude = latitude;
  _settings.manualLongitude = longitude;
  _settings.manualAltitude = altitude;
  saveSettings();
}

void SettingsManager::setTimeSource(TimeSource source) {
  _settings.timeSource = source;
  saveSettings();
}

void SettingsManager::setTimeZoneOffset(int offset) {
  _settings.timeZoneOffset = offset;
  saveSettings();
}

void SettingsManager::setUseDST(bool useDST) {
  _settings.useDST = useDST;
  saveSettings();
}

void SettingsManager::setUseNorthReference(bool useNorthReference) {
  _settings.useNorthReference = useNorthReference;
  saveSettings();
}

void SettingsManager::setManualDeclination(float declination) {
  _settings.manualDeclination = declination;
  saveSettings();
}

void SettingsManager::setSleepTimeout(int timeout) {
  _settings.sleepTimeout = timeout;
  applyPowerSettings();
  saveSettings();
}

void SettingsManager::setEnableBluetooth(bool enable) {
  _settings.enableBluetooth = enable;
  applyPowerSettings();
  saveSettings();
}

void SettingsManager::setEnableDebugOutput(bool enable) {
  _settings.enableDebugOutput = enable;
  saveSettings();
}

void SettingsManager::setEnableDataLogging(bool enable) {
  _settings.enableDataLogging = enable;
  saveSettings();
}

// Apply settings
void SettingsManager::applySettings() {
  applyDisplaySettings();
  applyPowerSettings();
  
  // Apply debug settings
  if (_settings.enableDebugOutput) {
    Serial.println("Debug output enabled");
  }
  
  if (_settings.enableDataLogging) {
    Serial.println("Data logging enabled");
  }
}

// Helper methods

void SettingsManager::applyDisplaySettings() {
  // Apply brightness setting
  uint8_t brightnessValue;
  switch (_settings.brightness) {
    case BRIGHTNESS_LOW:
      brightnessValue = 20;
      break;
    case BRIGHTNESS_MEDIUM:
      brightnessValue = 100;
      break;
    case BRIGHTNESS_HIGH:
      brightnessValue = 255;
      break;
    default:
      brightnessValue = 100;
      break;
  }
  
  // Set LED brightness
  // M5Unifiedでは、ディスプレイの明るさを設定する方法が異なります
  M5.Lcd.setBrightness(brightnessValue);
  
  // Apply night mode if enabled
  if (_settings.nightMode) {
    // Use red color for night mode
    // M5Unifiedでは、ディスプレイ全体を赤色に設定します
    M5.Lcd.fillScreen(0x100000); // Dim red
  }
}

void SettingsManager::applyPowerSettings() {
  // Apply sleep timeout
  if (_settings.sleepTimeout > 0) {
    // Enable sleep mode with timeout
    // Note: This is a placeholder, actual implementation depends on the hardware
    Serial.print("Sleep timeout set to ");
    Serial.print(_settings.sleepTimeout);
    Serial.println(" seconds");
  } else {
    // Disable sleep mode
    Serial.println("Sleep mode disabled");
  }
  
  // Apply Bluetooth setting
  if (_settings.enableBluetooth) {
    // Enable Bluetooth
    // Note: This is a placeholder, actual implementation depends on the hardware
    Serial.println("Bluetooth enabled");
  } else {
    // Disable Bluetooth
    Serial.println("Bluetooth disabled");
  }
}
