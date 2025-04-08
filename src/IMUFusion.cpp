/*
 * IMUFusion.cpp
 * 
 * Implementation of sensor fusion for BMI270 and BMM150
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "IMUFusion.h"
#include "myMahonyAHRS.h"
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
  
  // Initialize MahonyAHRS algorithm
  myIMU::MahonyAHRSinit();
  
  // Set default values for Mahony algorithm
  myIMU::myKp = 8.0f;  // 比例ゲイン
  myIMU::myKi = 0.0f;  // 積分ゲイン
  
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
  
  // リンク先のコードを参考にした軸調整
  float gx = _bmi270->gyr_y * DEG_TO_RAD;
  float gy = -_bmi270->gyr_x * DEG_TO_RAD;
  float gz = _bmi270->gyr_z * DEG_TO_RAD;
  
  float ax = _bmi270->acc_y;
  float ay = -_bmi270->acc_x;
  float az = _bmi270->acc_z;
  
  float mx = -_bmm150->mag_x;
  float my = _bmm150->mag_y;
  float mz = -_bmm150->mag_z;
  
  // MahonyAHRSアルゴリズムを使用して姿勢を更新
  myIMU::MahonyAHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz, deltaTime);
  
  // クォータニオンをコピー
  _q0 = myIMU::q[0];
  _q1 = myIMU::q[1];
  _q2 = myIMU::q[2];
  _q3 = myIMU::q[3];
  
  // クォータニオンからオイラー角を計算
  updateEulerAngles();
}

float IMUFusion::getYaw() {
  // リンク先のコードを参考にした方位角計算
  float yaw = atan2(2*(_q1*_q2 + _q0*_q3), _q0*_q0+_q1*_q1-_q2*_q2-_q3*_q3);
  
  // 調整（リンク先のコードと同様）
  yaw = -yaw - PI/2;
  
  // 0〜2π（0〜360度）の範囲に正規化
  if (yaw < 0) yaw += 2*PI;
  if (yaw > 2*PI) yaw -= 2*PI;
  
  // ラジアンから度に変換
  yaw *= RAD_TO_DEG;
  
  // 磁気偏角の補正を適用
  yaw += _magDeclination;
  
  // 0〜360度の範囲に保つ
  while (yaw < 0.0f) yaw += 360.0f;
  while (yaw >= 360.0f) yaw -= 360.0f;
  
  return yaw;
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
  // リンク先のコードを参考にしたオイラー角計算
  _pitch = asin(-2 * _q1 * _q3 + 2 * _q0 * _q2) * RAD_TO_DEG;
  _roll = atan2(2 * _q2 * _q3 + 2 * _q0 * _q1, -2 * _q1 * _q1 - 2 * _q2 * _q2 + 1) * RAD_TO_DEG;
  
  // ヨー角（方位角）は getYaw() メソッドで計算
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
