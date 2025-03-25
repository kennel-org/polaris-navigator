/*
 * SettingsMenu.h
 * 
 * Settings menu interface for the Polaris Navigator
 * Handles settings display and navigation
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include <M5Unified.h>
#include "SettingsManager.h"

// Menu categories
enum MenuCategory {
  MENU_DISPLAY,
  MENU_LOCATION,
  MENU_TIME,
  MENU_COMPASS,
  MENU_POWER,
  MENU_DEBUG,
  MENU_SYSTEM,
  MENU_EXIT
};

// Menu states
enum MenuState {
  MENU_STATE_CATEGORY,
  MENU_STATE_SETTING,
  MENU_STATE_EDIT
};

class SettingsMenu {
public:
  // Constructor
  SettingsMenu(SettingsManager* settingsManager);
  
  // Initialize menu
  void begin();
  
  // Show menu
  void show();
  
  // Update menu display
  void update();
  
  // Handle button press
  void handleButtonPress(bool shortPress, bool longPress);
  
  // Check if menu is active
  bool isActive();
  
  // Exit menu
  void exit();
  
private:
  // Reference to settings manager
  SettingsManager* _settingsManager;
  
  // Menu state
  bool _menuActive;
  MenuState _menuState;
  MenuCategory _currentCategory;
  int _currentSetting;
  int _editValue;
  
  // Timing
  unsigned long _lastUpdateTime;
  unsigned long _lastButtonTime;
  
  // Helper methods
  void showCategoryMenu();
  void showSettingMenu();
  void showEditMenu();
  
  void navigateCategory(bool next);
  void navigateSetting(bool next);
  void editSetting(bool increase);
  
  void selectCategory();
  void selectSetting();
  void saveEdit();
  
  void applyChanges();
  
  // Display helpers
  void printMenuHeader(const char* title);
  void printMenuFooter();
  void printMenuOption(int index, const char* option, bool selected);
  void printEditValue(const char* name, const char* value, bool editing);
  
  // String formatting helpers
  void formatBrightness(char* buffer, BrightnessLevel brightness);
  void formatLocationSource(char* buffer, LocationSource source);
  void formatTimeSource(char* buffer, TimeSource source);
  void formatTimeZone(char* buffer, int offset);
  void formatCoordinate(char* buffer, float value, bool isLatitude);
  void formatBoolean(char* buffer, bool value);
  void formatDeclination(char* buffer, float declination);
  void formatTimeout(char* buffer, int timeout);
};

#endif // SETTINGS_MENU_H
