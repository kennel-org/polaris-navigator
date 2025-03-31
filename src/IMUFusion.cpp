/*
 * IMUFusion.cpp
 * 
 * Implementation of sensor fusion for BMI270 and BMM150
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "IMUFusion.h"
#include <math.h>
#include <M5Unified.h>  // M5.update()を使用するために必要

// Constants
#define RAD_TO_DEG 57.2957795131  // 180/PI
#define DEG_TO_RAD 0.01745329251  // PI/180

IMUFusion::IMUFusion(BMI270 *bmi270, BMM150class *bmm150) {
  _bmi270 = bmi270;
  _bmm150 = bmm150;
  
  // Initialize quaternion to identity
  _q0 = 1.0f;
  _q1 = 0.0f;
  _q2 = 0.0f;
  _q3 = 0.0f;
  
  // Initialize Euler angles
  _yaw = 0.0f;
  _pitch = 0.0f;
  _roll = 0.0f;
  
  // Default filter parameters
  _filterGain = 0.05f;  // 95% gyro, 5% accel/mag
  _magDeclination = 0.0f;
  
  _isCalibrated = false;
  
  // Initialize complementary filter values
  _alpha = 0.98f;  // 98% gyro, 2% accel/mag for complementary filter
  _lastUpdate = 0;
}

void IMUFusion::begin() {
  // Initialize with current sensor data
  _bmi270->readAcceleration();
  _bmm150->readMagnetometer();
  
  // Calculate initial orientation from accelerometer
  float accX = _bmi270->acc_x;
  float accY = _bmi270->acc_y;
  float accZ = _bmi270->acc_z;
  
  // Normalize accelerometer data
  float norm = sqrt(accX * accX + accY * accY + accZ * accZ);
  if (norm > 0.0f) {
    accX /= norm;
    accY /= norm;
    accZ /= norm;
  }
  
  // Initial pitch and roll from accelerometer
  _pitch = asin(-accX) * RAD_TO_DEG;
  _roll = atan2(accY, accZ) * RAD_TO_DEG;
  
  // Initial yaw from magnetometer with tilt compensation
  _yaw = _bmm150->calculateTiltCompensatedHeading(_pitch, _roll);
  
  // Convert Euler angles to quaternion
  float cosYaw = cos(_yaw * DEG_TO_RAD * 0.5f);
  float sinYaw = sin(_yaw * DEG_TO_RAD * 0.5f);
  float cosPitch = cos(_pitch * DEG_TO_RAD * 0.5f);
  float sinPitch = sin(_pitch * DEG_TO_RAD * 0.5f);
  float cosRoll = cos(_roll * DEG_TO_RAD * 0.5f);
  float sinRoll = sin(_roll * DEG_TO_RAD * 0.5f);
  
  _q0 = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
  _q1 = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
  _q2 = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
  _q3 = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
  
  normalizeQuaternion();
  
  _lastUpdate = millis();
}

void IMUFusion::update(float deltaTime) {
  // If deltaTime is not provided, calculate it
  if (deltaTime <= 0) {
    unsigned long now = millis();
    deltaTime = (now - _lastUpdate) / 1000.0f;
    _lastUpdate = now;
    
    // Sanity check for deltaTime
    if (deltaTime <= 0 || deltaTime > 1.0f) {
      deltaTime = 0.01f;  // Default to 10ms if time is unreasonable
    }
  }
  
  // Read latest sensor data
  _bmi270->readAcceleration();
  _bmi270->readGyro();
  _bmm150->readMagnetometer();
  
  // Get gyroscope data (in rad/s)
  float gx = _bmi270->gyr_x * DEG_TO_RAD;
  float gy = _bmi270->gyr_y * DEG_TO_RAD;
  float gz = _bmi270->gyr_z * DEG_TO_RAD;
  
  // Get accelerometer data
  float ax = _bmi270->acc_x;
  float ay = _bmi270->acc_y;
  float az = _bmi270->acc_z;
  
  // Get magnetometer data
  float mx = _bmm150->mag_x;
  float my = _bmm150->mag_y;
  float mz = _bmm150->mag_z;
  
  // Normalize accelerometer data
  float normAcc = sqrt(ax * ax + ay * ay + az * az);
  if (normAcc > 0.0f) {
    ax /= normAcc;
    ay /= normAcc;
    az /= normAcc;
  }
  
  // Normalize magnetometer data
  float normMag = sqrt(mx * mx + my * my + mz * mz);
  if (normMag > 0.0f) {
    mx /= normMag;
    my /= normMag;
    mz /= normMag;
  }
  
  // Calculate pitch and roll from accelerometer (for complementary filter)
  float accelPitch = asin(-ax) * RAD_TO_DEG;
  float accelRoll = atan2(ay, az) * RAD_TO_DEG;
  
  // Update quaternion using gyroscope data
  float halfT = deltaTime * 0.5f;
  float q0_dot = 0.5f * (-_q1 * gx - _q2 * gy - _q3 * gz);
  float q1_dot = 0.5f * (_q0 * gx + _q2 * gz - _q3 * gy);
  float q2_dot = 0.5f * (_q0 * gy - _q1 * gz + _q3 * gx);
  float q3_dot = 0.5f * (_q0 * gz + _q1 * gy - _q2 * gx);
  
  _q0 += q0_dot * deltaTime;
  _q1 += q1_dot * deltaTime;
  _q2 += q2_dot * deltaTime;
  _q3 += q3_dot * deltaTime;
  
  normalizeQuaternion();
  
  // Convert quaternion to Euler angles
  updateEulerAngles();
  
  // Apply complementary filter for pitch and roll
  _pitch = _alpha * _pitch + (1.0f - _alpha) * accelPitch;
  _roll = _alpha * _roll + (1.0f - _alpha) * accelRoll;
  
  // Get tilt-compensated heading from magnetometer
  float magHeading = _bmm150->calculateTiltCompensatedHeading(_pitch, _roll);
  
  // Apply complementary filter for yaw
  // Calculate the difference between current yaw and mag heading
  float yawDiff = magHeading - _yaw;
  
  // Normalize the difference to -180 to +180
  if (yawDiff > 180.0f) yawDiff -= 360.0f;
  if (yawDiff < -180.0f) yawDiff += 360.0f;
  
  // Apply the filter with a lower weight for magnetometer
  _yaw += yawDiff * (1.0f - _alpha) * 0.5f;
  
  // Ensure yaw is in 0-360 range
  while (_yaw < 0.0f) _yaw += 360.0f;
  while (_yaw >= 360.0f) _yaw -= 360.0f;
}

float IMUFusion::getYaw() {
  // Apply magnetic declination correction
  float correctedYaw = _yaw + _magDeclination;
  
  // Ensure yaw is in 0-360 range
  while (correctedYaw < 0.0f) correctedYaw += 360.0f;
  while (correctedYaw >= 360.0f) correctedYaw -= 360.0f;
  
  return correctedYaw;
}

float IMUFusion::getPitch() {
  return _pitch;
}

float IMUFusion::getRoll() {
  return _roll;
}

void IMUFusion::getQuaternion(float *q0, float *q1, float *q2, float *q3) {
  *q0 = _q0;
  *q1 = _q1;
  *q2 = _q2;
  *q3 = _q3;
}

void IMUFusion::calibrateMagnetometer() {
  // 新しいステップベースのキャリブレーション方式を使用
  Serial.println("IMUFusion::calibrateMagnetometer() - Using step-based calibration");
  
  // 初期化ステップを実行
  _bmm150->calibrateStep(true);
  
  // キャリブレーションが完了するまでループ
  bool isComplete = false;
  while (!isComplete) {
    // ボタン入力をチェック（キャンセル用）
    M5.update();
    if (M5.BtnA.wasPressed()) {
      // キャリブレーションをキャンセル
      Serial.println("IMUFusion::calibrateMagnetometer() - Calibration cancelled by user");
      return;
    }
    
    // 1ステップ実行
    isComplete = _bmm150->calibrateStep(false);
    
    // 少し待機
    delay(50);
  }
  
  _isCalibrated = true;
  Serial.println("IMUFusion::calibrateMagnetometer() - Calibration completed successfully");
}

bool IMUFusion::isCalibrated() {
  return _isCalibrated;
}

void IMUFusion::setFilterGain(float gain) {
  if (gain >= 0.0f && gain <= 1.0f) {
    _filterGain = gain;
    // Also update complementary filter alpha
    _alpha = 1.0f - gain;
    if (_alpha > 0.99f) _alpha = 0.99f;
    if (_alpha < 0.5f) _alpha = 0.5f;
  }
}

void IMUFusion::setMagneticDeclination(float declination) {
  _magDeclination = declination;
}

void IMUFusion::updateEulerAngles() {
  // Convert quaternion to Euler angles
  _roll = atan2(2.0f * (_q0 * _q1 + _q2 * _q3), 1.0f - 2.0f * (_q1 * _q1 + _q2 * _q2)) * RAD_TO_DEG;
  _pitch = asin(2.0f * (_q0 * _q2 - _q3 * _q1)) * RAD_TO_DEG;
  
  // Calculate yaw from quaternion
  float yaw = atan2(2.0f * (_q0 * _q3 + _q1 * _q2), 1.0f - 2.0f * (_q2 * _q2 + _q3 * _q3)) * RAD_TO_DEG;
  
  // Ensure yaw is in 0-360 range
  if (yaw < 0.0f) yaw += 360.0f;
  
  // We don't update _yaw here as we use the complementary filter with magnetometer
}

void IMUFusion::normalizeQuaternion() {
  float norm = sqrt(_q0 * _q0 + _q1 * _q1 + _q2 * _q2 + _q3 * _q3);
  if (norm > 0.0f) {
    _q0 /= norm;
    _q1 /= norm;
    _q2 /= norm;
    _q3 /= norm;
  }
}
