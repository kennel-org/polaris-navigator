/*
 * celestial_math.h
 * 
 * Celestial calculations for Polaris Navigator
 * Handles calculations for celestial pole, sun, and moon positions
 */

#ifndef CELESTIAL_MATH_H
#define CELESTIAL_MATH_H

// Time-related functions
// Julian date with fractional day component
double getJulianDate(int year, int month, int day);
double getJulianCentury(double jd);
double getSiderealTime(double jd, double longitude);

// Celestial pole calculations
void calculatePolePosition(float latitude, float longitude, float *azimuth, float *altitude);

// Sun position calculations
void calculateSunPosition(float latitude, float longitude, 
                          int year, int month, int day, 
                          int hour, int minute, int second,
                          float *azimuth, float *altitude);

// Moon position calculations
void calculateMoonPosition(float latitude, float longitude, 
                           int year, int month, int day, 
                           int hour, int minute, int second,
                           float *azimuth, float *altitude, float *phase);

// Sunrise and sunset calculations
void calculateSunriseSunset(float latitude, float longitude, int year, int month, int day,
                           int* sunriseHour, int* sunriseMinute, int* sunsetHour, int* sunsetMinute);

// Moonrise and moonset calculations
void calculateMoonriseMoonset(float latitude, float longitude, int year, int month, int day,
                             int* moonriseHour, int* moonriseMinute, int* moonsetHour, int* moonsetMinute);

// Moon phase calculations
float calculateMoonPhase(int year, int month, int day);

// Utility functions
float calculateMagneticDeclination(float latitude, float longitude);
void applyMagneticDeclination(float *heading, float declination);

#endif // CELESTIAL_MATH_H
