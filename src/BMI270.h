/*
 * BMI270.h
 * 
 * Class for interfacing with the BMI270 IMU (accelerometer and gyroscope) on AtomS3R
 */

#ifndef BMI270_H
#define BMI270_H

#include <Arduino.h>
#include <Wire.h>

// BMI270 registers
#define BMI270_CHIP_ID          0x00
#define BMI270_ERR_REG          0x02
#define BMI270_STATUS           0x03
#define BMI270_ACC_X_LSB        0x0C
#define BMI270_ACC_X_MSB        0x0D
#define BMI270_ACC_Y_LSB        0x0E
#define BMI270_ACC_Y_MSB        0x0F
#define BMI270_ACC_Z_LSB        0x10
#define BMI270_ACC_Z_MSB        0x11
#define BMI270_GYR_X_LSB        0x12
#define BMI270_GYR_X_MSB        0x13
#define BMI270_GYR_Y_LSB        0x14
#define BMI270_GYR_Y_MSB        0x15
#define BMI270_GYR_Z_LSB        0x16
#define BMI270_GYR_Z_MSB        0x17
#define BMI270_CMD              0x7E

// BMI270 commands
#define BMI270_CMD_SOFTRESET    0xB6

// BMI270 I2C address
#define BMI270_I2C_ADDR         0x68

// Return codes
#define BMI270_OK               0
#define BMI270_ERROR            1

class BMI270 {
public:
  BMI270();
  
  // Initialization
  int begin();
  
  // Read sensor data
  void readAcceleration();
  void readGyro();
  
  // Raw sensor data
  int16_t raw_acc_x;
  int16_t raw_acc_y;
  int16_t raw_acc_z;
  
  int16_t raw_gyr_x;
  int16_t raw_gyr_y;
  int16_t raw_gyr_z;
  
  // Processed sensor data (in g for accelerometer, deg/s for gyroscope)
  float acc_x;
  float acc_y;
  float acc_z;
  
  float gyr_x;
  float gyr_y;
  float gyr_z;
  
  // Calculate orientation
  void calculateOrientation(float *pitch, float *roll);
  
private:
  // I2C communication
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  
  // Sensor configuration
  void softReset();
  void configure();
  
  // Conversion factors
  float acc_scale;  // Scale factor for accelerometer (g per LSB)
  float gyr_scale;  // Scale factor for gyroscope (deg/s per LSB)
};

#endif // BMI270_H
