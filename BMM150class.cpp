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
}

int BMM150class::initialize() {
  // Check if BMM150 is connected
  uint8_t chipId = readRegister(BMM150_CHIP_ID);
  if (chipId != 0x32) {  // BMM150 chip ID should be 0x32
    return BMM150_ERROR;
  }
  
  // Set power mode
  writeRegister(BMM150_POWER_CONTROL, 0x01);  // Power on
  delay(10);  // Wait for power-up
  
  // Set operation mode to normal
  writeRegister(BMM150_OP_MODE, BMM150_NORMAL_MODE);
  delay(10);  // Wait for mode change
  
  return BMM150_OK;
}

void BMM150class::readMagnetometer() {
  // Read raw magnetometer data
  uint8_t data[6];
  
  // Read 6 bytes starting from DATA_X_LSB
  Wire.beginTransmission(BMM150_I2C_ADDR);
  Wire.write(BMM150_DATA_X_LSB);
  Wire.endTransmission(false);
  
  Wire.requestFrom(BMM150_I2C_ADDR, 6);
  if (Wire.available() == 6) {
    for (int i = 0; i < 6; i++) {
      data[i] = Wire.read();
    }
  }
  
  // Combine MSB and LSB bytes
  raw_mag_x = (int16_t)((data[1] << 8) | data[0]);
  raw_mag_y = (int16_t)((data[3] << 8) | data[2]);
  raw_mag_z = (int16_t)((data[5] << 8) | data[4]);
  
  // Apply calibration
  mag_x = (raw_mag_x - offset_x) * scale_x;
  mag_y = (raw_mag_y - offset_y) * scale_y;
  mag_z = (raw_mag_z - offset_z) * scale_z;
}

float BMM150class::calculateHeading() {
  // Calculate heading from magnetometer data
  // Assuming the device is flat (no tilt compensation yet)
  
  float heading = atan2(mag_y, mag_x) * 180.0 / PI;
  
  // Convert to 0-360 degrees
  if (heading < 0) {
    heading += 360.0;
  }
  
  return heading;
}

void BMM150class::calibrate() {
  // Simple calibration procedure
  // In a real implementation, you would want to collect multiple samples
  // and perform more sophisticated calibration
  
  // Collect min/max values by rotating the device
  int16_t min_x = 32767, max_x = -32768;
  int16_t min_y = 32767, max_y = -32768;
  int16_t min_z = 32767, max_z = -32768;
  
  Serial.println("Starting magnetometer calibration...");
  Serial.println("Please rotate the device in all directions for 10 seconds.");
  
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {  // 10 seconds calibration
    readMagnetometer();
    
    // Update min/max values
    if (raw_mag_x < min_x) min_x = raw_mag_x;
    if (raw_mag_x > max_x) max_x = raw_mag_x;
    
    if (raw_mag_y < min_y) min_y = raw_mag_y;
    if (raw_mag_y > max_y) max_y = raw_mag_y;
    
    if (raw_mag_z < min_z) min_z = raw_mag_z;
    if (raw_mag_z > max_z) max_z = raw_mag_z;
    
    delay(10);  // Sample every 10ms
  }
  
  // Calculate offsets (center of min/max)
  offset_x = (min_x + max_x) / 2.0;
  offset_y = (min_y + max_y) / 2.0;
  offset_z = (min_z + max_z) / 2.0;
  
  // Calculate scaling factors
  // Ideally, we want a perfect sphere, so we scale based on the average radius
  float avg_delta = ((max_x - min_x) + (max_y - min_y) + (max_z - min_z)) / 6.0;
  
  scale_x = avg_delta / (max_x - min_x);
  scale_y = avg_delta / (max_y - min_y);
  scale_z = avg_delta / (max_z - min_z);
  
  isCalibrated = true;
  
  Serial.println("Magnetometer calibration complete!");
  Serial.print("Offsets: X=");
  Serial.print(offset_x);
  Serial.print(", Y=");
  Serial.print(offset_y);
  Serial.print(", Z=");
  Serial.println(offset_z);
  
  Serial.print("Scaling: X=");
  Serial.print(scale_x);
  Serial.print(", Y=");
  Serial.print(scale_y);
  Serial.print(", Z=");
  Serial.println(scale_z);
}

uint8_t BMM150class::readRegister(uint8_t reg) {
  Wire.beginTransmission(BMM150_I2C_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(BMM150_I2C_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  
  return 0;
}

void BMM150class::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(BMM150_I2C_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}
