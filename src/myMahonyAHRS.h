/*
 * myMahonyAHRS.h
 * 
 * MahonyAHRS algorithm implementation for sensor fusion
 * Based on the algorithm by Sebastian Madgwick
 * 
 * Created: 2025-04-07
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef MY_MAHONY_AHRS_H
#define MY_MAHONY_AHRS_H

#include <Arduino.h>
#include <math.h>

namespace myIMU {

// Global variables for MahonyAHRS algorithm
extern float q[4];         // Quaternion
extern float myKp;         // Proportional gain
extern float myKi;         // Integral gain
extern float eInt[3];      // Integral error

// MahonyAHRS algorithm implementation
void MahonyAHRSupdate(
    float gx, float gy, float gz,     // Gyroscope data in rad/s
    float ax, float ay, float az,     // Accelerometer data (normalized)
    float mx, float my, float mz,     // Magnetometer data (normalized)
    float dt);                        // Time step in seconds

// Initialize the algorithm
void MahonyAHRSinit();

} // namespace myIMU

#endif // MY_MAHONY_AHRS_H
