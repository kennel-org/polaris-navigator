/*
 * IMUFusion.h
 * 
 * Sensor fusion for BMI270 (accelerometer/gyroscope) and BMM150 (magnetometer)
 * Implements a complementary filter for orientation estimation
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef IMU_FUSION_H
#define IMU_FUSION_H

#include <Arduino.h>
#include "BMI270.h"
#include "BMM150class.h"

class IMUFusion {
public:
  // Constructor
  IMUFusion(BMI270 *bmi270, BMM150class *bmm150);
  
  // Initialize fusion algorithm
  void begin();
  
  // Update orientation (call this regularly)
  void update(float deltaTime);
  
  // Get orientation in Euler angles (degrees)
  float getYaw();    // Heading/Azimuth (0-360)
  float getPitch();  // Pitch (-90 to 90)
  float getRoll();   // Roll (-180 to 180)
  
  // Get orientation in quaternion
  void getQuaternion(float *q0, float *q1, float *q2, float *q3);
  
  // Calibration
  void calibrateMagnetometer();
  bool isCalibrated();
  
  // Set filter parameters
  void setFilterGain(float gain);
  
  // Apply declination correction
  void setMagneticDeclination(float declination);
  
private:
  // Sensor references
  BMI270 *_bmi270;
  BMM150class *_bmm150;
  
  // Orientation representation
  float _q0, _q1, _q2, _q3;  // Quaternion
  float _yaw, _pitch, _roll;  // Euler angles in degrees
  
  // Filter parameters
  float _filterGain;  // Complementary filter gain (0.0-1.0)
  float _magDeclination;  // Magnetic declination correction in degrees
  
  // Calibration status
  bool _isCalibrated;
  
  // Helper functions
  void updateEulerAngles();
  void normalizeQuaternion();
};

#endif // IMU_FUSION_H
