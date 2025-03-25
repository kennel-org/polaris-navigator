/*
 * CalibrationManager.h
 * 
 * Manages sensor calibration for the Polaris Navigator
 * Handles calibration procedures and data storage
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef CALIBRATION_MANAGER_H
#define CALIBRATION_MANAGER_H

#include <M5Unified.h>
#include <Preferences.h>
#include "BMI270.h"
#include "BMM150class.h"

// Calibration states
enum CalibrationState {
  CAL_IDLE,             // Not calibrating
  CAL_ACCEL_START,      // Starting accelerometer calibration
  CAL_ACCEL_COLLECT,    // Collecting accelerometer data
  CAL_ACCEL_COMPLETE,   // Accelerometer calibration complete
  CAL_MAG_START,        // Starting magnetometer calibration
  CAL_MAG_COLLECT,      // Collecting magnetometer data
  CAL_MAG_COMPLETE,     // Magnetometer calibration complete
  CAL_COMPLETE          // All calibration complete
};

// Calibration status structure for UI
struct CalibrationStatus {
  int stage;            // Current calibration stage (0-3)
  float progress;       // Progress within current stage (0.0-1.0)
  bool isComplete;      // Whether calibration is complete
};

// Calibration data structure
struct CalibrationData {
  // Accelerometer calibration
  float accelOffset[3];    // X, Y, Z offsets
  float accelScale[3];     // X, Y, Z scale factors
  
  // Magnetometer calibration
  float magOffset[3];      // X, Y, Z offsets
  float magScale[3];       // X, Y, Z scale factors
  
  // Calibration status
  bool accelCalibrated;
  bool magCalibrated;
  
  // Calibration timestamp
  unsigned long timestamp;
};

class CalibrationManager {
public:
  // Constructor
  CalibrationManager(BMI270* bmi270, BMM150class* bmm150);
  
  // Initialize calibration manager
  void begin();
  
  // Start calibration procedure (both accel and mag)
  void startCalibration();
  
  // Start calibration procedure with specific sensors
  void startCalibration(bool accel, bool mag);
  
  // Update calibration process
  void updateCalibration();
  
  // Cancel ongoing calibration
  void cancelCalibration();
  
  // Check if calibration is in progress
  bool isCalibrating();
  
  // Get current calibration state
  CalibrationState getCalibrationState();
  
  // Get calibration status for UI
  CalibrationStatus getCalibrationStatus();
  
  // Get calibration data
  CalibrationData getCalibrationData();
  
  // Load calibration data from storage
  bool loadCalibrationData();
  
  // Save calibration data to storage
  bool saveCalibrationData();
  
  // Apply calibration to sensors
  void applyCalibration();
  
  // Reset calibration data
  void resetCalibration();
  
  // Check if sensors are calibrated
  bool isCalibrated();
  
  // Check if accelerometer is calibrated
  bool isAccelCalibrated();
  
  // Check if magnetometer is calibrated
  bool isMagCalibrated();
  
private:
  // References to sensor objects
  BMI270* _bmi270;
  BMM150class* _bmm150;
  
  // Calibration state
  CalibrationState _calibrationState;
  unsigned long _calibrationStartTime;
  unsigned long _lastUpdateTime;
  
  // Calibration data
  CalibrationData _calibrationData;
  
  // Data collection for calibration
  float _accelMin[3];
  float _accelMax[3];
  float _magMin[3];
  float _magMax[3];
  int _sampleCount;
  int _requiredSamples;
  
  // Storage for calibration data
  Preferences _preferences;
  
  // Helper methods
  void collectAccelSample();
  void collectMagSample();
  void calculateAccelCalibration();
  void calculateMagCalibration();
};

#endif // CALIBRATION_MANAGER_H
