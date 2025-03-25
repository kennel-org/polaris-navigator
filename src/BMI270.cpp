/*
 * BMI270.cpp
 * 
 * Implementation of the BMI270 IMU class for AtomS3R
 */

#include "BMI270.h"
#include <math.h>

BMI270::BMI270() {
  // Initialize variables
  raw_acc_x = 0;
  raw_acc_y = 0;
  raw_acc_z = 0;
  
  raw_gyr_x = 0;
  raw_gyr_y = 0;
  raw_gyr_z = 0;
  
  acc_x = 0.0;
  acc_y = 0.0;
  acc_z = 0.0;
  
  gyr_x = 0.0;
  gyr_y = 0.0;
  gyr_z = 0.0;
  
  // Default scale factors
  // These may need adjustment based on the actual sensor configuration
  acc_scale = 2.0 / 32768.0;  // ±2g range
  gyr_scale = 250.0 / 32768.0;  // ±250 deg/s range
}

int BMI270::begin() {
  // Check if BMI270 is connected
  uint8_t chipId = readRegister(BMI270_CHIP_ID);
  if (chipId != 0x24) {  // BMI270 chip ID should be 0x24
    return BMI270_ERROR;
  }
  
  // Reset and configure the sensor
  softReset();
  delay(50);  // Wait for reset to complete
  
  configure();
  
  return BMI270_OK;
}

void BMI270::readAcceleration() {
  // Read raw accelerometer data
  uint8_t data[6];
  
  // Read 6 bytes starting from ACC_X_LSB
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(BMI270_ACC_X_LSB);
  Wire.endTransmission(false);
  
  Wire.requestFrom(BMI270_I2C_ADDR, 6);
  if (Wire.available() == 6) {
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }
  }
  
  // Combine MSB and LSB bytes
  raw_acc_x = (int16_t)((data[1] << 8) | data[0]);
  raw_acc_y = (int16_t)((data[3] << 8) | data[2]);
  raw_acc_z = (int16_t)((data[5] << 8) | data[4]);
  
  // Convert to g
  acc_x = raw_acc_x * acc_scale;
  acc_y = raw_acc_y * acc_scale;
  acc_z = raw_acc_z * acc_scale;
}

void BMI270::readGyro() {
  // Read raw gyroscope data
  uint8_t data[6];
  
  // Read 6 bytes starting from GYR_X_LSB
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(BMI270_GYR_X_LSB);
  Wire.endTransmission(false);
  
  Wire.requestFrom(BMI270_I2C_ADDR, 6);
  if (Wire.available() == 6) {
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }
  }
  
  // Combine MSB and LSB bytes
  raw_gyr_x = (int16_t)((data[1] << 8) | data[0]);
  raw_gyr_y = (int16_t)((data[3] << 8) | data[2]);
  raw_gyr_z = (int16_t)((data[5] << 8) | data[4]);
  
  // Convert to deg/s
  gyr_x = raw_gyr_x * gyr_scale;
  gyr_y = raw_gyr_y * gyr_scale;
  gyr_z = raw_gyr_z * gyr_scale;
}

void BMI270::calculateOrientation(float *pitch, float *roll) {
  // Calculate pitch and roll from accelerometer data
  // This is a simple calculation without sensor fusion
  
  // Make sure we have fresh accelerometer data
  readAcceleration();
  
  // Calculate pitch and roll in radians
  *pitch = atan2(acc_y, sqrt(acc_x * acc_x + acc_z * acc_z));
  *roll = atan2(-acc_x, acc_z);
  
  // Convert to degrees
  *pitch *= 180.0 / PI;
  *roll *= 180.0 / PI;
}

uint8_t BMI270::readRegister(uint8_t reg) {
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(BMI270_I2C_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  
  return 0;
}

void BMI270::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(BMI270_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void BMI270::softReset() {
  // Perform a soft reset
  writeRegister(BMI270_CMD, BMI270_CMD_SOFTRESET);
}

void BMI270::configure() {
  // Configure the sensor
  // This is a simplified configuration
  // In a real implementation, you would configure the sensor
  // based on your specific requirements
  
  // For now, we'll use default settings
  // Additional configuration would be added here
}
