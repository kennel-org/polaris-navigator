/*
 * BMM150class.h
 * 
 * Class for interfacing with the BMM150 magnetometer on AtomS3R
 */

#ifndef BMM150CLASS_H
#define BMM150CLASS_H

#include <Arduino.h>
#include <Wire.h>
#include <M5Unified.h> // M5Unifiedライブラリを追加

// BMM150 registers (for reference, accessed through BMI270)
#define BMM150_CHIP_ID          0x40
#define BMM150_DATA_X_LSB       0x42
#define BMM150_DATA_X_MSB       0x43
#define BMM150_DATA_Y_LSB       0x44
#define BMM150_DATA_Y_MSB       0x45
#define BMM150_DATA_Z_LSB       0x46
#define BMM150_DATA_Z_MSB       0x47
#define BMM150_REG_DATA_READY   0x48
#define BMM150_POWER_CONTROL    0x4B
#define BMM150_OP_MODE          0x4C

// BMI270 I2C address (used to access BMM150 in AtomS3R)
#define BMI270_I2C_ADDR         0x68

// BMM150 operation modes
#define BMM150_SLEEP_MODE       0x00
#define BMM150_NORMAL_MODE      0x01

// BMM150 I2C address (direct access, not used in AtomS3R)
#define BMM150_I2C_ADDR         0x10

// Return codes
#define BMM150_OK               0
#define BMM150_ERROR            1

class BMM150class {
public:
  BMM150class();
  
  // Initialization
  int initialize();
  
  // Read magnetometer data
  void readMagnetometer();
  
  // Calibration
  void calibrate();
  
  // Calibration with cancellation check
  bool calibrateStep(bool initialize);
  
  // Raw magnetometer data
  int16_t raw_mag_x;
  int16_t raw_mag_y;
  int16_t raw_mag_z;
  
  // Calibrated magnetometer data
  float mag_x;
  float mag_y;
  float mag_z;
  
  // Previous magnetometer values for filtering
  float prev_mag_x;
  float prev_mag_y;
  float prev_mag_z;
  
  // Heading calculation
  float calculateHeading();
  
  // Tilt-compensated heading calculation
  float calculateTiltCompensatedHeading(float pitch, float roll);
  
  // キャリブレーションデータを設定するメソッド
  void setCalibrationData(float hardIronX, float hardIronY, float hardIronZ,
                          float scaleX, float scaleY, float scaleZ);
  
  // キャリブレーション状態を取得するメソッド
  bool getCalibrationStatus();
  
  // キャリブレーション状態を設定するメソッド
  void setCalibrationStatus(bool status);
  
private:
  // I2C communication
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  
  // Calibration offsets
  float offset_x;
  float offset_y;
  float offset_z;
  
  // Calibration scaling
  float scale_x;
  float scale_y;
  float scale_z;
  
  // Hard-iron correction values
  float hard_iron_x;
  float hard_iron_y;
  float hard_iron_z;
  
  // Soft-iron correction matrix
  float soft_iron[3][3];
  
  // Flags
  bool isCalibrated;
  
  // キャリブレーション用の変数
  unsigned long _calibrationStartTime;
  int16_t _min_x, _max_x;
  int16_t _min_y, _max_y;
  int16_t _min_z, _max_z;
  int _sampleCount;
};

#endif // BMM150CLASS_H
