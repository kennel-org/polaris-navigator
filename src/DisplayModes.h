/*
 * DisplayModes.h
 * 
 * Display mode definitions for the Polaris Navigator
 * 
 * Created: 2025-03-24
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef DISPLAY_MODES_H
#define DISPLAY_MODES_H

// Display modes
enum DisplayMode {
  POLAR_ALIGNMENT = 0,
  GPS_DATA = 1,
  IMU_DATA = 2,
  CELESTIAL_DATA = 3,
  RAW_DATA = 4,
  SETTINGS_MENU = 5,
  CALIBRATION_MODE = 6
};

// Raw data display modes
enum RawDataMode {
  RAW_IMU = 0,        // IMU raw data
  RAW_GPS = 1,        // GPS raw data
  RAW_CELESTIAL = 2,  // Celestial data
  RAW_SYSTEM = 3,     // System information
  DISPLAY_DEBUG = 4   // Debug mode (consistent naming)
};

#endif // DISPLAY_MODES_H
