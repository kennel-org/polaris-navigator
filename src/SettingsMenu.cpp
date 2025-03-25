/*
 * SettingsMenu.cpp
 * 
 * Implementation for the Settings Menu
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "SettingsMenu.h"

// Constructor
SettingsMenu::SettingsMenu(SettingsManager* settingsManager) {
  _settingsManager = settingsManager;
  
  _menuActive = false;
  _menuState = MENU_STATE_CATEGORY;
  _currentCategory = MENU_DISPLAY;
  _currentSetting = 0;
  _editValue = 0;
  
  _lastUpdateTime = 0;
  _lastButtonTime = 0;
}

// Initialize menu
void SettingsMenu::begin() {
  // Nothing to initialize for now
}

// Show menu
void SettingsMenu::show() {
  _menuActive = true;
  _menuState = MENU_STATE_CATEGORY;
  _currentCategory = MENU_DISPLAY;
  _currentSetting = 0;
  
  Serial.println("Settings menu activated");
  
  // Show initial menu
  update();
}

// Update menu display
void SettingsMenu::update() {
  // Only update every 100ms
  unsigned long currentTime = millis();
  if (currentTime - _lastUpdateTime < 100) {
    return;
  }
  _lastUpdateTime = currentTime;
  
  // Update display based on current state
  switch (_menuState) {
    case MENU_STATE_CATEGORY:
      showCategoryMenu();
      break;
      
    case MENU_STATE_SETTING:
      showSettingMenu();
      break;
      
    case MENU_STATE_EDIT:
      showEditMenu();
      break;
  }
}

// Handle button press
void SettingsMenu::handleButtonPress(bool shortPress, bool longPress) {
  // Debounce button presses
  unsigned long currentTime = millis();
  if (currentTime - _lastButtonTime < 200) {
    return;
  }
  _lastButtonTime = currentTime;
  
  // Handle button press based on current state
  if (shortPress) {
    // Short press - navigate or increment
    switch (_menuState) {
      case MENU_STATE_CATEGORY:
        navigateCategory(true);
        break;
        
      case MENU_STATE_SETTING:
        navigateSetting(true);
        break;
        
      case MENU_STATE_EDIT:
        editSetting(true);
        break;
    }
  } else if (longPress) {
    // Long press - select or save
    switch (_menuState) {
      case MENU_STATE_CATEGORY:
        selectCategory();
        break;
        
      case MENU_STATE_SETTING:
        selectSetting();
        break;
        
      case MENU_STATE_EDIT:
        saveEdit();
        break;
    }
  }
  
  // Update display
  update();
}

// Check if menu is active
bool SettingsMenu::isActive() {
  return _menuActive;
}

// Exit menu
void SettingsMenu::exit() {
  _menuActive = false;
  Serial.println("Settings menu exited");
  
  // Apply any pending changes
  applyChanges();
}

// Helper methods

void SettingsMenu::showCategoryMenu() {
  // Print menu header
  printMenuHeader("Settings");
  
  // Print category options
  printMenuOption(0, "Display", _currentCategory == MENU_DISPLAY);
  printMenuOption(1, "Location", _currentCategory == MENU_LOCATION);
  printMenuOption(2, "Time", _currentCategory == MENU_TIME);
  printMenuOption(3, "Compass", _currentCategory == MENU_COMPASS);
  printMenuOption(4, "Power", _currentCategory == MENU_POWER);
  printMenuOption(5, "Debug", _currentCategory == MENU_DEBUG);
  printMenuOption(6, "System", _currentCategory == MENU_SYSTEM);
  printMenuOption(7, "Exit", _currentCategory == MENU_EXIT);
  
  // Print menu footer
  printMenuFooter();
}

void SettingsMenu::showSettingMenu() {
  char buffer[32];
  UserSettings settings = _settingsManager->getSettings();
  
  // Print menu header based on category
  switch (_currentCategory) {
    case MENU_DISPLAY:
      printMenuHeader("Display Settings");
      
      // Print display settings
      formatBrightness(buffer, settings.brightness);
      printMenuOption(0, "Brightness", _currentSetting == 0);
      Serial.println(buffer);
      
      formatBoolean(buffer, settings.nightMode);
      printMenuOption(1, "Night Mode", _currentSetting == 1);
      Serial.println(buffer);
      
      printMenuOption(2, "Back", _currentSetting == 2);
      break;
      
    case MENU_LOCATION:
      printMenuHeader("Location Settings");
      
      // Print location settings
      formatLocationSource(buffer, settings.locationSource);
      printMenuOption(0, "Source", _currentSetting == 0);
      Serial.println(buffer);
      
      formatCoordinate(buffer, settings.manualLatitude, true);
      printMenuOption(1, "Latitude", _currentSetting == 1);
      Serial.println(buffer);
      
      formatCoordinate(buffer, settings.manualLongitude, false);
      printMenuOption(2, "Longitude", _currentSetting == 2);
      Serial.println(buffer);
      
      printMenuOption(3, "Back", _currentSetting == 3);
      break;
      
    case MENU_TIME:
      printMenuHeader("Time Settings");
      
      // Print time settings
      formatTimeSource(buffer, settings.timeSource);
      printMenuOption(0, "Source", _currentSetting == 0);
      Serial.println(buffer);
      
      formatTimeZone(buffer, settings.timeZoneOffset);
      printMenuOption(1, "Time Zone", _currentSetting == 1);
      Serial.println(buffer);
      
      formatBoolean(buffer, settings.useDST);
      printMenuOption(2, "Use DST", _currentSetting == 2);
      Serial.println(buffer);
      
      printMenuOption(3, "Back", _currentSetting == 3);
      break;
      
    case MENU_COMPASS:
      printMenuHeader("Compass Settings");
      
      // Print compass settings
      formatBoolean(buffer, settings.useNorthReference);
      printMenuOption(0, "Use True North", _currentSetting == 0);
      Serial.println(buffer);
      
      formatDeclination(buffer, settings.manualDeclination);
      printMenuOption(1, "Declination", _currentSetting == 1);
      Serial.println(buffer);
      
      printMenuOption(2, "Back", _currentSetting == 2);
      break;
      
    case MENU_POWER:
      printMenuHeader("Power Settings");
      
      // Print power settings
      formatTimeout(buffer, settings.sleepTimeout);
      printMenuOption(0, "Sleep Timeout", _currentSetting == 0);
      Serial.println(buffer);
      
      formatBoolean(buffer, settings.enableBluetooth);
      printMenuOption(1, "Bluetooth", _currentSetting == 1);
      Serial.println(buffer);
      
      printMenuOption(2, "Back", _currentSetting == 2);
      break;
      
    case MENU_DEBUG:
      printMenuHeader("Debug Settings");
      
      // Print debug settings
      formatBoolean(buffer, settings.enableDebugOutput);
      printMenuOption(0, "Debug Output", _currentSetting == 0);
      Serial.println(buffer);
      
      formatBoolean(buffer, settings.enableDataLogging);
      printMenuOption(1, "Data Logging", _currentSetting == 1);
      Serial.println(buffer);
      
      printMenuOption(2, "Back", _currentSetting == 2);
      break;
      
    case MENU_SYSTEM:
      printMenuHeader("System Settings");
      
      // Print system settings
      printMenuOption(0, "Reset Settings", _currentSetting == 0);
      printMenuOption(1, "Device Info", _currentSetting == 1);
      printMenuOption(2, "Back", _currentSetting == 2);
      break;
      
    default:
      // Unknown category, go back to category menu
      _menuState = MENU_STATE_CATEGORY;
      break;
  }
  
  // Print menu footer
  printMenuFooter();
}

void SettingsMenu::showEditMenu() {
  char buffer[32];
  char valueBuffer[32];
  UserSettings settings = _settingsManager->getSettings();
  
  // Print edit menu based on category and setting
  switch (_currentCategory) {
    case MENU_DISPLAY:
      switch (_currentSetting) {
        case 0: // Brightness
          printMenuHeader("Edit Brightness");
          formatBrightness(valueBuffer, (BrightnessLevel)_editValue);
          printEditValue("Brightness", valueBuffer, true);
          break;
          
        case 1: // Night Mode
          printMenuHeader("Edit Night Mode");
          formatBoolean(valueBuffer, (bool)_editValue);
          printEditValue("Night Mode", valueBuffer, true);
          break;
      }
      break;
      
    case MENU_LOCATION:
      switch (_currentSetting) {
        case 0: // Location Source
          printMenuHeader("Edit Location Source");
          formatLocationSource(valueBuffer, (LocationSource)_editValue);
          printEditValue("Source", valueBuffer, true);
          break;
          
        case 1: // Latitude
          printMenuHeader("Edit Latitude");
          formatCoordinate(valueBuffer, settings.manualLatitude + (_editValue * 0.1), true);
          printEditValue("Latitude", valueBuffer, true);
          break;
          
        case 2: // Longitude
          printMenuHeader("Edit Longitude");
          formatCoordinate(valueBuffer, settings.manualLongitude + (_editValue * 0.1), false);
          printEditValue("Longitude", valueBuffer, true);
          break;
      }
      break;
      
    case MENU_TIME:
      switch (_currentSetting) {
        case 0: // Time Source
          printMenuHeader("Edit Time Source");
          formatTimeSource(valueBuffer, (TimeSource)_editValue);
          printEditValue("Source", valueBuffer, true);
          break;
          
        case 1: // Time Zone
          printMenuHeader("Edit Time Zone");
          formatTimeZone(valueBuffer, settings.timeZoneOffset + (_editValue * 15));
          printEditValue("Time Zone", valueBuffer, true);
          break;
          
        case 2: // Use DST
          printMenuHeader("Edit DST");
          formatBoolean(valueBuffer, (bool)_editValue);
          printEditValue("Use DST", valueBuffer, true);
          break;
      }
      break;
      
    case MENU_COMPASS:
      switch (_currentSetting) {
        case 0: // Use True North
          printMenuHeader("Edit North Reference");
          formatBoolean(valueBuffer, (bool)_editValue);
          printEditValue("Use True North", valueBuffer, true);
          break;
          
        case 1: // Declination
          printMenuHeader("Edit Declination");
          formatDeclination(valueBuffer, settings.manualDeclination + (_editValue * 0.1));
          printEditValue("Declination", valueBuffer, true);
          break;
      }
      break;
      
    case MENU_POWER:
      switch (_currentSetting) {
        case 0: // Sleep Timeout
          printMenuHeader("Edit Sleep Timeout");
          formatTimeout(valueBuffer, _editValue * 60);
          printEditValue("Sleep Timeout", valueBuffer, true);
          break;
          
        case 1: // Bluetooth
          printMenuHeader("Edit Bluetooth");
          formatBoolean(valueBuffer, (bool)_editValue);
          printEditValue("Bluetooth", valueBuffer, true);
          break;
      }
      break;
      
    case MENU_DEBUG:
      switch (_currentSetting) {
        case 0: // Debug Output
          printMenuHeader("Edit Debug Output");
          formatBoolean(valueBuffer, (bool)_editValue);
          printEditValue("Debug Output", valueBuffer, true);
          break;
          
        case 1: // Data Logging
          printMenuHeader("Edit Data Logging");
          formatBoolean(valueBuffer, (bool)_editValue);
          printEditValue("Data Logging", valueBuffer, true);
          break;
      }
      break;
  }
  
  // Print menu footer
  printMenuFooter();
}

void SettingsMenu::navigateCategory(bool next) {
  // Navigate to next/previous category
  if (next) {
    _currentCategory = (MenuCategory)((_currentCategory + 1) % MENU_EXIT);
    if (_currentCategory == MENU_EXIT) {
      _currentCategory = MENU_DISPLAY;
    }
  } else {
    if (_currentCategory == MENU_DISPLAY) {
      _currentCategory = MENU_EXIT;
    } else {
      _currentCategory = (MenuCategory)(_currentCategory - 1);
    }
  }
  
  Serial.print("Selected category: ");
  Serial.println(_currentCategory);
}

void SettingsMenu::navigateSetting(bool next) {
  // Get max setting index based on category
  int maxSetting;
  switch (_currentCategory) {
    case MENU_DISPLAY:
      maxSetting = 2; // Brightness, Night Mode, Back
      break;
    case MENU_LOCATION:
      maxSetting = 3; // Source, Latitude, Longitude, Back
      break;
    case MENU_TIME:
      maxSetting = 3; // Source, Time Zone, DST, Back
      break;
    case MENU_COMPASS:
      maxSetting = 2; // True North, Declination, Back
      break;
    case MENU_POWER:
      maxSetting = 2; // Sleep Timeout, Bluetooth, Back
      break;
    case MENU_DEBUG:
      maxSetting = 2; // Debug Output, Data Logging, Back
      break;
    case MENU_SYSTEM:
      maxSetting = 2; // Reset Settings, Device Info, Back
      break;
    default:
      maxSetting = 0;
      break;
  }
  
  // Navigate to next/previous setting
  if (next) {
    _currentSetting = (_currentSetting + 1) % (maxSetting + 1);
  } else {
    if (_currentSetting == 0) {
      _currentSetting = maxSetting;
    } else {
      _currentSetting--;
    }
  }
  
  Serial.print("Selected setting: ");
  Serial.println(_currentSetting);
}

void SettingsMenu::editSetting(bool increase) {
  // Edit current setting value
  UserSettings settings = _settingsManager->getSettings();
  
  switch (_currentCategory) {
    case MENU_DISPLAY:
      switch (_currentSetting) {
        case 0: // Brightness
          if (increase) {
            _editValue = (_editValue + 1) % 3;
          } else {
            _editValue = (_editValue + 2) % 3;
          }
          break;
          
        case 1: // Night Mode
          _editValue = !_editValue;
          break;
      }
      break;
      
    case MENU_LOCATION:
      switch (_currentSetting) {
        case 0: // Location Source
          if (increase) {
            _editValue = (_editValue + 1) % 2;
          } else {
            _editValue = (_editValue + 1) % 2;
          }
          break;
          
        case 1: // Latitude
          if (increase) {
            _editValue++;
          } else {
            _editValue--;
          }
          // Limit latitude to +/- 90 degrees
          if (settings.manualLatitude + (_editValue * 0.1) > 90.0) {
            _editValue--;
          } else if (settings.manualLatitude + (_editValue * 0.1) < -90.0) {
            _editValue++;
          }
          break;
          
        case 2: // Longitude
          if (increase) {
            _editValue++;
          } else {
            _editValue--;
          }
          // Limit longitude to +/- 180 degrees
          if (settings.manualLongitude + (_editValue * 0.1) > 180.0) {
            _editValue--;
          } else if (settings.manualLongitude + (_editValue * 0.1) < -180.0) {
            _editValue++;
          }
          break;
      }
      break;
      
    case MENU_TIME:
      switch (_currentSetting) {
        case 0: // Time Source
          if (increase) {
            _editValue = (_editValue + 1) % 3;
          } else {
            _editValue = (_editValue + 2) % 3;
          }
          break;
          
        case 1: // Time Zone
          if (increase) {
            _editValue++;
          } else {
            _editValue--;
          }
          // Limit time zone to +/- 12 hours
          if (settings.timeZoneOffset + (_editValue * 15) > 720) {
            _editValue--;
          } else if (settings.timeZoneOffset + (_editValue * 15) < -720) {
            _editValue++;
          }
          break;
          
        case 2: // Use DST
          _editValue = !_editValue;
          break;
      }
      break;
      
    case MENU_COMPASS:
      switch (_currentSetting) {
        case 0: // Use True North
          _editValue = !_editValue;
          break;
          
        case 1: // Declination
          if (increase) {
            _editValue++;
          } else {
            _editValue--;
          }
          // Limit declination to +/- 30 degrees
          if (settings.manualDeclination + (_editValue * 0.1) > 30.0) {
            _editValue--;
          } else if (settings.manualDeclination + (_editValue * 0.1) < -30.0) {
            _editValue++;
          }
          break;
      }
      break;
      
    case MENU_POWER:
      switch (_currentSetting) {
        case 0: // Sleep Timeout
          if (increase) {
            _editValue++;
          } else {
            _editValue--;
          }
          // Limit sleep timeout to 0-30 minutes
          if (_editValue < 0) {
            _editValue = 0;
          } else if (_editValue > 30) {
            _editValue = 30;
          }
          break;
          
        case 1: // Bluetooth
          _editValue = !_editValue;
          break;
      }
      break;
      
    case MENU_DEBUG:
      switch (_currentSetting) {
        case 0: // Debug Output
          _editValue = !_editValue;
          break;
          
        case 1: // Data Logging
          _editValue = !_editValue;
          break;
      }
      break;
  }
  
  Serial.print("Edit value: ");
  Serial.println(_editValue);
}

void SettingsMenu::selectCategory() {
  // Handle category selection
  if (_currentCategory == MENU_EXIT) {
    // Exit menu
    exit();
  } else {
    // Enter setting selection
    _menuState = MENU_STATE_SETTING;
    _currentSetting = 0;
    
    Serial.print("Entered category: ");
    Serial.println(_currentCategory);
  }
}

void SettingsMenu::selectSetting() {
  // Handle setting selection
  UserSettings settings = _settingsManager->getSettings();
  
  // Check if "Back" option was selected
  int backOption;
  switch (_currentCategory) {
    case MENU_DISPLAY:
    case MENU_COMPASS:
    case MENU_POWER:
    case MENU_DEBUG:
    case MENU_SYSTEM:
      backOption = 2;
      break;
    case MENU_LOCATION:
    case MENU_TIME:
      backOption = 3;
      break;
    default:
      backOption = 0;
      break;
  }
  
  if (_currentSetting == backOption) {
    // Go back to category menu
    _menuState = MENU_STATE_CATEGORY;
    
    Serial.println("Returned to category menu");
    return;
  }
  
  // Special handling for system settings
  if (_currentCategory == MENU_SYSTEM) {
    if (_currentSetting == 0) {
      // Reset settings
      _settingsManager->resetSettings();
      _settingsManager->saveSettings();
      _settingsManager->applySettings();
      
      Serial.println("Settings reset to defaults");
      
      // Go back to category menu
      _menuState = MENU_STATE_CATEGORY;
      return;
    } else if (_currentSetting == 1) {
      // Show device info
      Serial.println("=== Device Info ===");
      Serial.println("Polaris Navigator");
      Serial.println("Version: 1.0.0");
      Serial.println("Hardware: AtomS3R with AtomicBase GPS");
      Serial.print("Compiled: ");
      Serial.print(__DATE__);
      Serial.print(" ");
      Serial.println(__TIME__);
      
      // Go back to category menu
      _menuState = MENU_STATE_CATEGORY;
      return;
    }
  }
  
  // Enter edit mode
  _menuState = MENU_STATE_EDIT;
  
  // Initialize edit value based on current setting
  switch (_currentCategory) {
    case MENU_DISPLAY:
      switch (_currentSetting) {
        case 0: // Brightness
          _editValue = settings.brightness;
          break;
        case 1: // Night Mode
          _editValue = settings.nightMode;
          break;
      }
      break;
      
    case MENU_LOCATION:
      switch (_currentSetting) {
        case 0: // Location Source
          _editValue = settings.locationSource;
          break;
        case 1: // Latitude
          _editValue = 0; // Start with no change
          break;
        case 2: // Longitude
          _editValue = 0; // Start with no change
          break;
      }
      break;
      
    case MENU_TIME:
      switch (_currentSetting) {
        case 0: // Time Source
          _editValue = settings.timeSource;
          break;
        case 1: // Time Zone
          _editValue = 0; // Start with no change
          break;
        case 2: // Use DST
          _editValue = settings.useDST;
          break;
      }
      break;
      
    case MENU_COMPASS:
      switch (_currentSetting) {
        case 0: // Use True North
          _editValue = settings.useNorthReference;
          break;
        case 1: // Declination
          _editValue = 0; // Start with no change
          break;
      }
      break;
      
    case MENU_POWER:
      switch (_currentSetting) {
        case 0: // Sleep Timeout
          _editValue = settings.sleepTimeout / 60; // Convert to minutes
          break;
        case 1: // Bluetooth
          _editValue = settings.enableBluetooth;
          break;
      }
      break;
      
    case MENU_DEBUG:
      switch (_currentSetting) {
        case 0: // Debug Output
          _editValue = settings.enableDebugOutput;
          break;
        case 1: // Data Logging
          _editValue = settings.enableDataLogging;
          break;
      }
      break;
  }
  
  Serial.print("Editing setting: ");
  Serial.print(_currentCategory);
  Serial.print(".");
  Serial.println(_currentSetting);
}

void SettingsMenu::saveEdit() {
  // Save edited value
  UserSettings settings = _settingsManager->getSettings();
  
  switch (_currentCategory) {
    case MENU_DISPLAY:
      switch (_currentSetting) {
        case 0: // Brightness
          settings.brightness = (BrightnessLevel)_editValue;
          break;
        case 1: // Night Mode
          settings.nightMode = (bool)_editValue;
          break;
      }
      break;
      
    case MENU_LOCATION:
      switch (_currentSetting) {
        case 0: // Location Source
          settings.locationSource = (LocationSource)_editValue;
          break;
        case 1: // Latitude
          settings.manualLatitude += (_editValue * 0.1);
          break;
        case 2: // Longitude
          settings.manualLongitude += (_editValue * 0.1);
          break;
      }
      break;
      
    case MENU_TIME:
      switch (_currentSetting) {
        case 0: // Time Source
          settings.timeSource = (TimeSource)_editValue;
          break;
        case 1: // Time Zone
          settings.timeZoneOffset += (_editValue * 15);
          break;
        case 2: // Use DST
          settings.useDST = (bool)_editValue;
          break;
      }
      break;
      
    case MENU_COMPASS:
      switch (_currentSetting) {
        case 0: // Use True North
          settings.useNorthReference = (bool)_editValue;
          break;
        case 1: // Declination
          settings.manualDeclination += (_editValue * 0.1);
          break;
      }
      break;
      
    case MENU_POWER:
      switch (_currentSetting) {
        case 0: // Sleep Timeout
          settings.sleepTimeout = _editValue * 60; // Convert to seconds
          break;
        case 1: // Bluetooth
          settings.enableBluetooth = (bool)_editValue;
          break;
      }
      break;
      
    case MENU_DEBUG:
      switch (_currentSetting) {
        case 0: // Debug Output
          settings.enableDebugOutput = (bool)_editValue;
          break;
        case 1: // Data Logging
          settings.enableDataLogging = (bool)_editValue;
          break;
      }
      break;
  }
  
  // Update settings
  _settingsManager->updateSettings(settings);
  
  Serial.println("Setting saved");
  
  // Return to setting menu
  _menuState = MENU_STATE_SETTING;
}

void SettingsMenu::applyChanges() {
  // Apply any pending changes
  _settingsManager->applySettings();
}

// Display helpers

void SettingsMenu::printMenuHeader(const char* title) {
  Serial.println("======================");
  Serial.println(title);
  Serial.println("======================");
}

void SettingsMenu::printMenuFooter() {
  Serial.println("======================");
  Serial.println("Short press: Navigate");
  Serial.println("Long press: Select");
  Serial.println("======================");
}

void SettingsMenu::printMenuOption(int index, const char* option, bool selected) {
  if (selected) {
    Serial.print("> ");
  } else {
    Serial.print("  ");
  }
  
  Serial.print(index + 1);
  Serial.print(". ");
  Serial.print(option);
  Serial.print(": ");
}

void SettingsMenu::printEditValue(const char* name, const char* value, bool editing) {
  Serial.print(name);
  Serial.print(": ");
  
  if (editing) {
    Serial.print("[");
    Serial.print(value);
    Serial.println("]");
  } else {
    Serial.println(value);
  }
  
  Serial.println();
  Serial.println("Short press: Change value");
  Serial.println("Long press: Save");
}

// String formatting helpers

void SettingsMenu::formatBrightness(char* buffer, BrightnessLevel brightness) {
  switch (brightness) {
    case BRIGHTNESS_LOW:
      strcpy(buffer, "Low");
      break;
    case BRIGHTNESS_MEDIUM:
      strcpy(buffer, "Medium");
      break;
    case BRIGHTNESS_HIGH:
      strcpy(buffer, "High");
      break;
    default:
      strcpy(buffer, "Unknown");
      break;
  }
}

void SettingsMenu::formatLocationSource(char* buffer, LocationSource source) {
  switch (source) {
    case LOCATION_GPS:
      strcpy(buffer, "GPS");
      break;
    case LOCATION_MANUAL:
      strcpy(buffer, "Manual");
      break;
    default:
      strcpy(buffer, "Unknown");
      break;
  }
}

void SettingsMenu::formatTimeSource(char* buffer, TimeSource source) {
  switch (source) {
    case TIME_GPS:
      strcpy(buffer, "GPS");
      break;
    case TIME_MANUAL:
      strcpy(buffer, "Manual");
      break;
    case TIME_NTP:
      strcpy(buffer, "NTP");
      break;
    default:
      strcpy(buffer, "Unknown");
      break;
  }
}

void SettingsMenu::formatTimeZone(char* buffer, int offset) {
  // Format time zone as UTC+/-HH:MM
  int hours = abs(offset) / 60;
  int minutes = abs(offset) % 60;
  
  sprintf(buffer, "UTC%s%02d:%02d", (offset >= 0 ? "+" : "-"), hours, minutes);
}

void SettingsMenu::formatCoordinate(char* buffer, float value, bool isLatitude) {
  // Format coordinate as degrees with direction
  float absValue = fabs(value);
  int degrees = (int)absValue;
  float minutes = (absValue - degrees) * 60.0;
  
  char direction;
  if (isLatitude) {
    direction = (value >= 0) ? 'N' : 'S';
  } else {
    direction = (value >= 0) ? 'E' : 'W';
  }
  
  sprintf(buffer, "%d° %.1f' %c", degrees, minutes, direction);
}

void SettingsMenu::formatBoolean(char* buffer, bool value) {
  strcpy(buffer, value ? "On" : "Off");
}

void SettingsMenu::formatDeclination(char* buffer, float declination) {
  // Format declination as degrees with direction
  sprintf(buffer, "%.1f°%s", fabs(declination), (declination >= 0 ? "E" : "W"));
}

void SettingsMenu::formatTimeout(char* buffer, int timeout) {
  // Format timeout in minutes
  if (timeout == 0) {
    strcpy(buffer, "Never");
  } else {
    sprintf(buffer, "%d min", timeout / 60);
  }
}
