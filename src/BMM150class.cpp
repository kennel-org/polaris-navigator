/*
 * BMM150class.cpp
 * 
 * Implementation of the BMM150 magnetometer class for AtomS3R
 */

#include "BMM150class.h"
#include <math.h>

BMM150class::BMM150class() {
  // Initialize variables
  raw_mag_x = 0;
  raw_mag_y = 0;
  raw_mag_z = 0;
  
  mag_x = 0.0;
  mag_y = 0.0;
  mag_z = 0.0;
  
  // Default calibration values
  offset_x = 0.0;
  offset_y = 0.0;
  offset_z = 0.0;
  
  scale_x = 1.0;
  scale_y = 1.0;
  scale_z = 1.0;
  
  isCalibrated = false;
  
  // Initialize hard-iron correction values
  hard_iron_x = 0.0;
  hard_iron_y = 0.0;
  hard_iron_z = 0.0;
  
  // Initialize soft-iron correction matrix (identity matrix)
  soft_iron[0][0] = 1.0; soft_iron[0][1] = 0.0; soft_iron[0][2] = 0.0;
  soft_iron[1][0] = 0.0; soft_iron[1][1] = 1.0; soft_iron[1][2] = 0.0;
  soft_iron[2][0] = 0.0; soft_iron[2][1] = 0.0; soft_iron[2][2] = 1.0;
  
  // Initialize previous magnetometer values for low-pass filter
  prev_mag_x = 0.0;
  prev_mag_y = 0.0;
  prev_mag_z = 0.0;
  
  // Initialize calibration step variables
  _min_x = 32767;
  _max_x = -32768;
  _min_y = 32767;
  _max_y = -32768;
  _min_z = 32767;
  _max_z = -32768;
  _calibrationStartTime = 0;
  _sampleCount = 0;
}

int BMM150class::initialize() {
  // BMM150 is accessed through BMI270 in AtomS3R
  // Set power mode for BMI270
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(0x7D);  // Power control register
  Wire.write(0x0B);  // Power on
  Wire.endTransmission();
  delay(50);  // Wait for power-up
  
  // Enable magnetometer in BMI270
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(0x7C);  // Magnetometer control register
  Wire.write(0x01);  // Enable magnetometer
  Wire.endTransmission();
  delay(100);  // Wait for initialization
  
  return BMM150_OK;
}

void BMM150class::readMagnetometer() {
  // Read magnetometer data through BMI270
  uint8_t data[8];
  
  // Read 8 bytes starting from magnetometer data register
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(0x12);  // BMM150 data register through BMI270
  Wire.endTransmission(false);
  
  Wire.requestFrom(BMI270_I2C_ADDR, 8);
  if (Wire.available() == 8) {
    for (int i = 0; i < 8; i++) {
      data[i] = Wire.read();
    }
  }
  
  // Combine MSB and LSB bytes
  raw_mag_x = (int16_t)((data[1] << 8) | data[0]);
  raw_mag_y = (int16_t)((data[3] << 8) | data[2]);
  raw_mag_z = (int16_t)((data[5] << 8) | data[4]);
  
  // Apply calibration
  float cal_x = (raw_mag_x - offset_x) * scale_x;
  float cal_y = (raw_mag_y - offset_y) * scale_y;
  float cal_z = (raw_mag_z - offset_z) * scale_z;
  
  // Apply hard-iron correction (offset)
  cal_x -= hard_iron_x;
  cal_y -= hard_iron_y;
  cal_z -= hard_iron_z;
  
  // Apply soft-iron correction (scaling and cross-axis effects)
  mag_x = soft_iron[0][0] * cal_x + soft_iron[0][1] * cal_y + soft_iron[0][2] * cal_z;
  mag_y = soft_iron[1][0] * cal_x + soft_iron[1][1] * cal_y + soft_iron[1][2] * cal_z;
  mag_z = soft_iron[2][0] * cal_x + soft_iron[2][1] * cal_y + soft_iron[2][2] * cal_z;
  
  // Apply low-pass filter to reduce noise
  mag_x = mag_x * 0.9 + prev_mag_x * 0.1;
  mag_y = mag_y * 0.9 + prev_mag_y * 0.1;
  mag_z = mag_z * 0.9 + prev_mag_z * 0.1;
  
  // Store current values for next filter iteration
  prev_mag_x = mag_x;
  prev_mag_y = mag_y;
  prev_mag_z = mag_z;
}

float BMM150class::calculateHeading() {
  // Calculate heading from magnetometer data
  // This assumes the device is flat (no tilt compensation)
  
  float heading = atan2(mag_y, mag_x) * 180.0 / PI;
  
  // Convert to 0-360 degrees
  if (heading < 0) {
    heading += 360.0;
  }
  
  return heading;
}

float BMM150class::calculateTiltCompensatedHeading(float pitch, float roll) {
  // Calculate tilt-compensated heading
  // pitch and roll in radians
  
  // Convert pitch and roll to radians if they are in degrees
  float pitch_rad = pitch * PI / 180.0;
  float roll_rad = roll * PI / 180.0;
  
  // Tilt compensation
  float mag_x_comp = mag_x * cos(pitch_rad) + mag_z * sin(pitch_rad);
  float mag_y_comp = mag_x * sin(roll_rad) * sin(pitch_rad) + mag_y * cos(roll_rad) - mag_z * sin(roll_rad) * cos(pitch_rad);
  
  // Calculate heading
  float heading = atan2(mag_y_comp, mag_x_comp) * 180.0 / PI;
  
  // Convert to 0-360 degrees
  if (heading < 0) {
    heading += 360.0;
  }
  
  return heading;
}

bool BMM150class::calibrateStep(bool firstStep) {
  // 初回呼び出し時の初期化
  if (firstStep) {
    Serial.println("Starting magnetometer calibration...");
    Serial.println("Please rotate the device in a figure-8 pattern for 15 seconds.");
    Serial.println("Keep away from metal objects and electronic devices.");
    
    // 変数の初期化
    _min_x = 32767;
    _max_x = -32768;
    _min_y = 32767;
    _max_y = -32768;
    _min_z = 32767;
    _max_z = -32768;
    
    _calibrationStartTime = millis();
    _sampleCount = 0;
    
    return false; // まだ完了していない
  }
  
  // キャリブレーション完了チェック
  unsigned long elapsedTime = millis() - _calibrationStartTime;
  if (elapsedTime >= 15000) {  // 15秒経過で完了
    // キャリブレーション完了処理
    Serial.println("\nProcessing calibration data...");
    
    // Calculate hard-iron offsets (center of min/max)
    hard_iron_x = (_min_x + _max_x) / 2.0f;
    hard_iron_y = (_min_y + _max_y) / 2.0f;
    hard_iron_z = (_min_z + _max_z) / 2.0f;
    
    // Calculate soft-iron scaling (normalize to sphere)
    float range_x = (_max_x - _min_x) / 2.0f;
    float range_y = (_max_y - _min_y) / 2.0f;
    float range_z = (_max_z - _min_z) / 2.0f;
    
    // Avoid division by zero
    if (range_x < 1.0f) range_x = 1.0f;
    if (range_y < 1.0f) range_y = 1.0f;
    if (range_z < 1.0f) range_z = 1.0f;
    
    // Calculate average range for scaling
    float avg_range = (range_x + range_y + range_z) / 3.0f;
    
    // Set scaling factors
    scale_x = avg_range / range_x;
    scale_y = avg_range / range_y;
    scale_z = avg_range / range_z;
    
    // Simple quality check based on range
    bool quality_ok = true;
    
    // Check if ranges are too small (poor calibration)
    if (range_x < 100 || range_y < 100 || range_z < 100) {
      Serial.println("Warning: Calibration range too small. Please recalibrate.");
      quality_ok = false;
    }
    
    // Check if ranges are too unbalanced (poor calibration)
    float max_range = max(range_x, max(range_y, range_z));
    float min_range = min(range_x, min(range_y, range_z));
    
    if (min_range < max_range * 0.3f) {
      Serial.println("Warning: Unbalanced calibration. Please recalibrate.");
      quality_ok = false;
    }
    
    // Set calibration flag
    isCalibrated = quality_ok;
    
    // Debug output
    Serial.println("Calibration complete!");
    Serial.print("Hard-iron offsets: X=");
    Serial.print(hard_iron_x);
    Serial.print(", Y=");
    Serial.print(hard_iron_y);
    Serial.print(", Z=");
    Serial.println(hard_iron_z);
    
    Serial.print("Scaling factors: X=");
    Serial.print(scale_x);
    Serial.print(", Y=");
    Serial.print(scale_y);
    Serial.print(", Z=");
    Serial.println(scale_z);
    
    Serial.print("Quality: ");
    Serial.println(quality_ok ? "GOOD" : "POOR");
    
    return true; // キャリブレーション完了
  }
  
  // キャリブレーション中の処理
  readMagnetometer();
  
  // Update min/max values
  if (raw_mag_x < _min_x) _min_x = raw_mag_x;
  if (raw_mag_x > _max_x) _max_x = raw_mag_x;
  
  if (raw_mag_y < _min_y) _min_y = raw_mag_y;
  if (raw_mag_y > _max_y) _max_y = raw_mag_y;
  
  if (raw_mag_z < _min_z) _min_z = raw_mag_z;
  if (raw_mag_z > _max_z) _max_z = raw_mag_z;
  
  _sampleCount++;
  
  // 進捗表示（シリアルモニタ用）
  if (_sampleCount % 5 == 0) {
    Serial.print("\rCalibrating... ");
    Serial.print((elapsedTime) / 1000);
    Serial.print("s / 15s");
  }
  
  return false; // まだ完了していない
}

void BMM150class::calibrate() {
  // 新しいステップ方式のキャリブレーションを使用
  bool firstStep = true;
  bool isComplete = false;
  
  // 初期化ステップを実行
  calibrateStep(true);
  
  // キャリブレーションが完了するまでループ
  while (!isComplete) {
    // ボタン入力をチェック（キャンセル用）
    M5.update();
    if (M5.BtnA.wasPressed()) {
      // キャリブレーションをキャンセル
      Serial.println("\nCalibration cancelled by user");
      return;
    }
    
    // 1ステップ実行
    isComplete = calibrateStep(false);
    
    // 少し待機
    delay(50);
  }
}

uint8_t BMM150class::readRegister(uint8_t reg) {
  // For compatibility, but not used in the new implementation
  return 0;
}

void BMM150class::writeRegister(uint8_t reg, uint8_t value) {
  // For compatibility, but not used in the new implementation
}

// キャリブレーションデータを設定するメソッド
void BMM150class::setCalibrationData(float hardIronX, float hardIronY, float hardIronZ,
                                    float scaleX, float scaleY, float scaleZ) {
  // ハードアイアン補正値を設定
  hard_iron_x = hardIronX;
  hard_iron_y = hardIronY;
  hard_iron_z = hardIronZ;
  
  // スケーリング係数を設定
  scale_x = scaleX;
  scale_y = scaleY;
  scale_z = scaleZ;
  
  Serial.println("Calibration data set:");
  Serial.print("Hard-iron: X=");
  Serial.print(hard_iron_x);
  Serial.print(", Y=");
  Serial.print(hard_iron_y);
  Serial.print(", Z=");
  Serial.println(hard_iron_z);
  
  Serial.print("Scale: X=");
  Serial.print(scale_x);
  Serial.print(", Y=");
  Serial.print(scale_y);
  Serial.print(", Z=");
  Serial.println(scale_z);
}

// キャリブレーション状態を取得するメソッド
bool BMM150class::getCalibrationStatus() {
  return isCalibrated;
}

// キャリブレーション状態を設定するメソッド
void BMM150class::setCalibrationStatus(bool status) {
  isCalibrated = status;
  Serial.print("Calibration status set to: ");
  Serial.println(status ? "Calibrated" : "Not Calibrated");
}
