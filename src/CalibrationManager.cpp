/*
 * CalibrationManager.cpp
 * 
 * Implementation for the Calibration Manager
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "CalibrationManager.h"
#include <math.h>

// Constructor
CalibrationManager::CalibrationManager(BMI270* bmi270, BMM150class* bmm150) {
  _bmi270 = bmi270;
  _bmm150 = bmm150;
  
  _calibrationState = CAL_IDLE;
  _calibrationStartTime = 0;
  _lastUpdateTime = 0;
  
  // Initialize calibration data
  resetCalibration();
  
  _sampleCount = 0;
  _requiredSamples = 100; // Number of samples required for calibration
}

// Initialize calibration manager
void CalibrationManager::begin() {
  // Initialize preferences for storing calibration data
  _preferences.begin("polaris-nav", false);
  
  // Try to load calibration data
  loadCalibrationData();
}

// Start calibration procedure (both accel and mag)
void CalibrationManager::startCalibration() {
  startCalibration(true, true);
}

// Start calibration procedure with specific sensors
void CalibrationManager::startCalibration(bool accel, bool mag) {
  // Reset calibration data
  resetCalibration();
  
  // Set calibration state based on what needs to be calibrated
  if (accel) {
    _calibrationState = CAL_ACCEL_START;
    
    // Initialize min/max values for accelerometer
    for (int i = 0; i < 3; i++) {
      _accelMin[i] = 1000.0;
      _accelMax[i] = -1000.0;
    }
  } else if (mag) {
    _calibrationState = CAL_MAG_START;
    
    // Initialize min/max values for magnetometer
    for (int i = 0; i < 3; i++) {
      _magMin[i] = 1000.0;
      _magMax[i] = -1000.0;
    }
  } else {
    // Nothing to calibrate
    _calibrationState = CAL_IDLE;
    return;
  }
  
  // Record start time
  _calibrationStartTime = millis();
  _lastUpdateTime = _calibrationStartTime;
  _sampleCount = 0;
  
  Serial.println("Calibration started");
}

// Update calibration process
void CalibrationManager::updateCalibration() {
  // Check if calibration is active
  if (_calibrationState == CAL_IDLE || _calibrationState == CAL_COMPLETE) {
    return;
  }
  
  // Get current time
  unsigned long currentTime = millis();
  
  // Update based on current state
  switch (_calibrationState) {
    case CAL_ACCEL_START:
      // Transition to data collection
      _calibrationState = CAL_ACCEL_COLLECT;
      Serial.println("Collecting accelerometer data...");
      break;
      
    case CAL_ACCEL_COLLECT:
      // Collect accelerometer sample
      collectAccelSample();
      
      // Check if we have enough samples
      if (_sampleCount >= _requiredSamples) {
        // Calculate calibration parameters
        calculateAccelCalibration();
        
        // Mark accelerometer as calibrated
        _calibrationData.accelCalibrated = true;
        
        // Transition to next state
        _calibrationState = CAL_ACCEL_COMPLETE;
        Serial.println("Accelerometer calibration complete");
        
        // Reset sample count for magnetometer
        _sampleCount = 0;
      }
      break;
      
    case CAL_ACCEL_COMPLETE:
      // Transition to magnetometer calibration
      _calibrationState = CAL_MAG_START;
      
      // Initialize min/max values for magnetometer
      for (int i = 0; i < 3; i++) {
        _magMin[i] = 1000.0;
        _magMax[i] = -1000.0;
      }
      break;
      
    case CAL_MAG_START:
      // Transition to data collection
      _calibrationState = CAL_MAG_COLLECT;
      Serial.println("Collecting magnetometer data...");
      break;
      
    case CAL_MAG_COLLECT:
      // Collect magnetometer sample
      collectMagSample();
      
      // Check if we have enough samples
      if (_sampleCount >= _requiredSamples) {
        // Calculate calibration parameters
        calculateMagCalibration();
        
        // Mark magnetometer as calibrated
        _calibrationData.magCalibrated = true;
        
        // Transition to complete state
        _calibrationState = CAL_MAG_COMPLETE;
        Serial.println("Magnetometer calibration complete");
      }
      break;
      
    case CAL_MAG_COMPLETE:
      // All calibration complete
      _calibrationState = CAL_COMPLETE;
      
      // Record timestamp
      _calibrationData.timestamp = currentTime;
      
      Serial.println("Calibration complete");
      break;
  }
  
  // Update last update time
  _lastUpdateTime = currentTime;
}

// Cancel ongoing calibration
void CalibrationManager::cancelCalibration() {
  if (_calibrationState != CAL_IDLE && _calibrationState != CAL_COMPLETE) {
    _calibrationState = CAL_IDLE;
    Serial.println("Calibration cancelled");
  }
}

// Check if calibration is in progress
bool CalibrationManager::isCalibrating() {
  return (_calibrationState != CAL_IDLE && _calibrationState != CAL_COMPLETE);
}

// Get current calibration state
CalibrationState CalibrationManager::getCalibrationState() {
  return _calibrationState;
}

// Get calibration status for UI
CalibrationStatus CalibrationManager::getCalibrationStatus() {
  CalibrationStatus status;
  
  // Determine stage and progress based on current state
  switch (_calibrationState) {
    case CAL_IDLE:
      status.stage = 0;
      status.progress = 0.0;
      status.isComplete = false;
      break;
      
    case CAL_ACCEL_START:
      status.stage = 0;
      status.progress = 0.0;
      status.isComplete = false;
      break;
      
    case CAL_ACCEL_COLLECT:
      status.stage = 0;
      status.progress = (float)_sampleCount / _requiredSamples;
      status.isComplete = false;
      break;
      
    case CAL_ACCEL_COMPLETE:
      status.stage = 1;
      status.progress = 0.0;
      status.isComplete = false;
      break;
      
    case CAL_MAG_START:
      status.stage = 1;
      status.progress = 0.0;
      status.isComplete = false;
      break;
      
    case CAL_MAG_COLLECT:
      status.stage = 1;
      status.progress = (float)_sampleCount / _requiredSamples;
      status.isComplete = false;
      break;
      
    case CAL_MAG_COMPLETE:
      status.stage = 2;
      status.progress = 1.0;
      status.isComplete = false;
      break;
      
    case CAL_COMPLETE:
      status.stage = 3;
      status.progress = 1.0;
      status.isComplete = true;
      break;
  }
  
  return status;
}

// Get calibration data
CalibrationData CalibrationManager::getCalibrationData() {
  return _calibrationData;
}

// Load calibration data from storage
bool CalibrationManager::loadCalibrationData() {
  // Check if calibration data exists
  if (!_preferences.isKey("cal_valid")) {
    return false;
  }
  
  // Load calibration status
  bool isValid = _preferences.getBool("cal_valid", false);
  if (!isValid) {
    return false;
  }
  
  // Load accelerometer calibration
  _calibrationData.accelCalibrated = _preferences.getBool("accel_cal", false);
  if (_calibrationData.accelCalibrated) {
    // Load accelerometer offsets
    _calibrationData.accelOffset[0] = _preferences.getFloat("accel_ox", 0.0);
    _calibrationData.accelOffset[1] = _preferences.getFloat("accel_oy", 0.0);
    _calibrationData.accelOffset[2] = _preferences.getFloat("accel_oz", 0.0);
    
    // Load accelerometer scale factors
    _calibrationData.accelScale[0] = _preferences.getFloat("accel_sx", 1.0);
    _calibrationData.accelScale[1] = _preferences.getFloat("accel_sy", 1.0);
    _calibrationData.accelScale[2] = _preferences.getFloat("accel_sz", 1.0);
  }
  
  // Load magnetometer calibration
  _calibrationData.magCalibrated = _preferences.getBool("mag_cal", false);
  if (_calibrationData.magCalibrated) {
    // Load magnetometer offsets
    _calibrationData.magOffset[0] = _preferences.getFloat("mag_ox", 0.0);
    _calibrationData.magOffset[1] = _preferences.getFloat("mag_oy", 0.0);
    _calibrationData.magOffset[2] = _preferences.getFloat("mag_oz", 0.0);
    
    // Load magnetometer scale factors
    _calibrationData.magScale[0] = _preferences.getFloat("mag_sx", 1.0);
    _calibrationData.magScale[1] = _preferences.getFloat("mag_sy", 1.0);
    _calibrationData.magScale[2] = _preferences.getFloat("mag_sz", 1.0);
  }
  
  // Load timestamp
  _calibrationData.timestamp = _preferences.getULong("cal_time", 0);
  
  Serial.println("Loaded calibration data from storage");
  return true;
}

// Save calibration data to storage
bool CalibrationManager::saveCalibrationData() {
  // Check if calibration is valid
  if (!isCalibrated()) {
    return false;
  }
  
  // Save calibration status
  _preferences.putBool("cal_valid", true);
  
  // Save accelerometer calibration
  _preferences.putBool("accel_cal", _calibrationData.accelCalibrated);
  if (_calibrationData.accelCalibrated) {
    // Save accelerometer offsets
    _preferences.putFloat("accel_ox", _calibrationData.accelOffset[0]);
    _preferences.putFloat("accel_oy", _calibrationData.accelOffset[1]);
    _preferences.putFloat("accel_oz", _calibrationData.accelOffset[2]);
    
    // Save accelerometer scale factors
    _preferences.putFloat("accel_sx", _calibrationData.accelScale[0]);
    _preferences.putFloat("accel_sy", _calibrationData.accelScale[1]);
    _preferences.putFloat("accel_sz", _calibrationData.accelScale[2]);
  }
  
  // Save magnetometer calibration
  _preferences.putBool("mag_cal", _calibrationData.magCalibrated);
  if (_calibrationData.magCalibrated) {
    // Save magnetometer offsets
    _preferences.putFloat("mag_ox", _calibrationData.magOffset[0]);
    _preferences.putFloat("mag_oy", _calibrationData.magOffset[1]);
    _preferences.putFloat("mag_oz", _calibrationData.magOffset[2]);
    
    // Save magnetometer scale factors
    _preferences.putFloat("mag_sx", _calibrationData.magScale[0]);
    _preferences.putFloat("mag_sy", _calibrationData.magScale[1]);
    _preferences.putFloat("mag_sz", _calibrationData.magScale[2]);
  }
  
  // Save timestamp
  _preferences.putULong("cal_time", _calibrationData.timestamp);
  
  Serial.println("Saved calibration data to storage");
  return true;
}

// Apply calibration to sensors
void CalibrationManager::applyCalibration() {
  // Apply accelerometer calibration
  if (_calibrationData.accelCalibrated) {
    // TODO: Apply accelerometer calibration to BMI270
    Serial.println("Applied accelerometer calibration");
  }
  
  // Apply magnetometer calibration
  if (_calibrationData.magCalibrated) {
    // TODO: Apply magnetometer calibration to BMM150
    Serial.println("Applied magnetometer calibration");
  }
}

// Reset calibration data
void CalibrationManager::resetCalibration() {
  // Reset accelerometer calibration
  for (int i = 0; i < 3; i++) {
    _calibrationData.accelOffset[i] = 0.0;
    _calibrationData.accelScale[i] = 1.0;
  }
  _calibrationData.accelCalibrated = false;
  
  // Reset magnetometer calibration
  for (int i = 0; i < 3; i++) {
    _calibrationData.magOffset[i] = 0.0;
    _calibrationData.magScale[i] = 1.0;
  }
  _calibrationData.magCalibrated = false;
  
  // Reset timestamp
  _calibrationData.timestamp = 0;
}

// Check if sensors are calibrated
bool CalibrationManager::isCalibrated() {
  return (_calibrationData.accelCalibrated && _calibrationData.magCalibrated);
}

// Check if accelerometer is calibrated
bool CalibrationManager::isAccelCalibrated() {
  return _calibrationData.accelCalibrated;
}

// Check if magnetometer is calibrated
bool CalibrationManager::isMagCalibrated() {
  return _calibrationData.magCalibrated;
}

// Helper methods

// Collect accelerometer sample
void CalibrationManager::collectAccelSample() {
  // Get accelerometer data
  _bmi270->readAcceleration();
  float accelX = _bmi270->acc_x;
  float accelY = _bmi270->acc_y;
  float accelZ = _bmi270->acc_z;
  
  // Update min/max values
  _accelMin[0] = min(_accelMin[0], accelX);
  _accelMin[1] = min(_accelMin[1], accelY);
  _accelMin[2] = min(_accelMin[2], accelZ);
  
  _accelMax[0] = max(_accelMax[0], accelX);
  _accelMax[1] = max(_accelMax[1], accelY);
  _accelMax[2] = max(_accelMax[2], accelZ);
  
  // Increment sample count
  _sampleCount++;
}

// Collect magnetometer sample
void CalibrationManager::collectMagSample() {
  // Get magnetometer data
  _bmm150->readMagnetometer();
  float magX = _bmm150->mag_x;
  float magY = _bmm150->mag_y;
  float magZ = _bmm150->mag_z;
  
  // Update min/max values
  _magMin[0] = min(_magMin[0], magX);
  _magMin[1] = min(_magMin[1], magY);
  _magMin[2] = min(_magMin[2], magZ);
  
  _magMax[0] = max(_magMax[0], magX);
  _magMax[1] = max(_magMax[1], magY);
  _magMax[2] = max(_magMax[2], magZ);
  
  // Increment sample count
  _sampleCount++;
}

// Calculate accelerometer calibration
void CalibrationManager::calculateAccelCalibration() {
  // Calculate offsets (center point)
  for (int i = 0; i < 3; i++) {
    _calibrationData.accelOffset[i] = (_accelMin[i] + _accelMax[i]) / 2.0;
  }
  
  // Calculate scale factors
  for (int i = 0; i < 3; i++) {
    float range = _accelMax[i] - _accelMin[i];
    if (range > 0.01) {
      _calibrationData.accelScale[i] = 2.0 / range; // Scale to +/- 1g
    } else {
      _calibrationData.accelScale[i] = 1.0; // Default if range is too small
    }
  }
}

// Calculate magnetometer calibration
void CalibrationManager::calculateMagCalibration() {
  // Calculate offsets (center point)
  for (int i = 0; i < 3; i++) {
    _calibrationData.magOffset[i] = (_magMin[i] + _magMax[i]) / 2.0;
  }
  
  // Calculate scale factors
  for (int i = 0; i < 3; i++) {
    float range = _magMax[i] - _magMin[i];
    if (range > 0.01) {
      _calibrationData.magScale[i] = 2.0 / range; // Scale to +/- 1
    } else {
      _calibrationData.magScale[i] = 1.0; // Default if range is too small
    }
  }
}
