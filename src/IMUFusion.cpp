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
  
  // Initial yaw from magnetometer
  _yaw = _bmm150->calculateHeading();
  
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
}

void IMUFusion::update(float deltaTime) {
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
  
  // Reference direction of Earth's magnetic field (tilt-compensated)
  float hx, hy, hz;
  
  // Auxiliary variables to avoid repeated calculations
  float q0q0 = _q0 * _q0;
  float q0q1 = _q0 * _q1;
  float q0q2 = _q0 * _q2;
  float q0q3 = _q0 * _q3;
  float q1q1 = _q1 * _q1;
  float q1q2 = _q1 * _q2;
  float q1q3 = _q1 * _q3;
  float q2q2 = _q2 * _q2;
  float q2q3 = _q2 * _q3;
  float q3q3 = _q3 * _q3;
  
  // Reference direction of Earth's magnetic field
  hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
  hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
  hz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));
  
  // Estimated direction of gravity and magnetic field
  float gravityX = 2.0f * (q1q3 - q0q2);
  float gravityY = 2.0f * (q0q1 + q2q3);
  float gravityZ = q0q0 - q1q1 - q2q2 + q3q3;
  
  // Error is cross product between estimated and measured direction of gravity
  float ex = (ay * gravityZ - az * gravityY);
  float ey = (az * gravityX - ax * gravityZ);
  float ez = (ax * gravityY - ay * gravityX);
  
  // Add magnetometer error if calibrated
  if (_isCalibrated) {
    float magX = 2.0f * (hx * (0.5f - q2q2 - q3q3) + hy * (q1q2 - q0q3) + hz * (q1q3 + q0q2));
    float magY = 2.0f * (hx * (q1q2 + q0q3) + hy * (0.5f - q1q1 - q3q3) + hz * (q2q3 - q0q1));
    float magZ = 2.0f * (hx * (q1q3 - q0q2) + hy * (q2q3 + q0q1) + hz * (0.5f - q1q1 - q2q2));
    
    ex += (my * magZ - mz * magY);
    ey += (mz * magX - mx * magZ);
    ez += (mx * magY - my * magX);
  }
  
  // Apply feedback terms
  gx += _filterGain * ex;
  gy += _filterGain * ey;
  gz += _filterGain * ez;
  
  // Integrate rate of change of quaternion
  float pa = _q1;
  float pb = _q2;
  float pc = _q3;
  
  _q0 += (-_q1 * gx - _q2 * gy - _q3 * gz) * (0.5f * deltaTime);
  _q1 += (_q0 * gx + pb * gz - pc * gy) * (0.5f * deltaTime);
  _q2 += (_q0 * gy - pa * gz + pc * gx) * (0.5f * deltaTime);
  _q3 += (_q0 * gz + pa * gy - pb * gx) * (0.5f * deltaTime);
  
  // Normalize quaternion
  normalizeQuaternion();
  
  // Update Euler angles
  updateEulerAngles();
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
  // Use BMM150's calibration function
  _bmm150->calibrate();
  _isCalibrated = true;
}

bool IMUFusion::isCalibrated() {
  return _isCalibrated;
}

void IMUFusion::setFilterGain(float gain) {
  // Limit gain to 0.0-1.0 range
  if (gain < 0.0f) gain = 0.0f;
  if (gain > 1.0f) gain = 1.0f;
  
  _filterGain = gain;
}

void IMUFusion::setMagneticDeclination(float declination) {
  _magDeclination = declination;
}

void IMUFusion::updateEulerAngles() {
  // Convert quaternion to Euler angles
  _yaw = atan2(2.0f * (_q1 * _q2 + _q0 * _q3), _q0 * _q0 + _q1 * _q1 - _q2 * _q2 - _q3 * _q3) * RAD_TO_DEG;
  _pitch = -asin(2.0f * (_q1 * _q3 - _q0 * _q2)) * RAD_TO_DEG;
  _roll = atan2(2.0f * (_q0 * _q1 + _q2 * _q3), _q0 * _q0 - _q1 * _q1 - _q2 * _q2 + _q3 * _q3) * RAD_TO_DEG;
  
  // Ensure yaw is in 0-360 range
  while (_yaw < 0.0f) _yaw += 360.0f;
  while (_yaw >= 360.0f) _yaw -= 360.0f;
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
