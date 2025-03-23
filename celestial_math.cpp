/*
 * celestial_math.cpp
 * 
 * Implementation of celestial calculations for Polaris Navigator
 */

#include "celestial_math.h"
#include <Arduino.h>
#include <math.h>

// Constants
#define DEG_TO_RAD (PI / 180.0)
#define RAD_TO_DEG (180.0 / PI)

// Time-related functions
unsigned long getJulianDate(int year, int month, int day) {
  // Julian Date calculation
  // Valid for dates after 1582-10-15
  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + 12 * a - 3;
  
  unsigned long jd = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
  
  return jd;
}

double getJulianCentury(unsigned long jd) {
  // Julian Century since J2000.0
  return (jd - 2451545.0) / 36525.0;
}

double getSiderealTime(double jd, double longitude) {
  // Calculate local sidereal time
  double T = getJulianCentury(jd);
  
  // Greenwich Mean Sidereal Time in degrees
  double GMST = 280.46061837 + 360.98564736629 * (jd - 2451545.0) +
                0.000387933 * T * T - T * T * T / 38710000.0;
  
  // Normalize to 0-360 degrees
  GMST = fmod(GMST, 360.0);
  if (GMST < 0) GMST += 360.0;
  
  // Local Sidereal Time
  double LST = GMST + longitude;
  
  // Normalize to 0-360 degrees
  LST = fmod(LST, 360.0);
  if (LST < 0) LST += 360.0;
  
  return LST;
}

// Celestial pole calculations
void calculatePolePosition(float latitude, float longitude, float *azimuth, float *altitude) {
  // For the celestial pole, the altitude is equal to the latitude in the northern hemisphere
  // In the southern hemisphere, it's the negative of the latitude
  
  if (latitude >= 0) {
    // Northern hemisphere - North Celestial Pole
    *altitude = latitude;
    *azimuth = 0.0; // True North
  } else {
    // Southern hemisphere - South Celestial Pole
    *altitude = -latitude;
    *azimuth = 180.0; // True South
  }
}

// Sun position calculations - simplified algorithm
void calculateSunPosition(float latitude, float longitude, 
                          int year, int month, int day, 
                          int hour, int minute, int second,
                          float *azimuth, float *altitude) {
  // This is a simplified algorithm for sun position
  // For a more accurate calculation, consider using a specialized library
  
  // Calculate day of year
  int dayOfYear = 0;
  for (int i = 1; i < month; i++) {
    if (i == 2) {
      dayOfYear += (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
    } else if (i == 4 || i == 6 || i == 9 || i == 11) {
      dayOfYear += 30;
    } else {
      dayOfYear += 31;
    }
  }
  dayOfYear += day;
  
  // Calculate fractional hour
  float fractionalHour = hour + minute / 60.0 + second / 3600.0;
  
  // Calculate solar declination
  float solarDeclination = 23.45 * sin(DEG_TO_RAD * (360.0 / 365.0) * (dayOfYear - 81));
  
  // Calculate equation of time (in minutes)
  float b = 360.0 / 365.0 * (dayOfYear - 81) * DEG_TO_RAD;
  float eot = 9.87 * sin(2 * b) - 7.53 * cos(b) - 1.5 * sin(b);
  
  // Calculate solar time
  float solarTime = fractionalHour + eot / 60.0 + (longitude - 15 * (int)((fractionalHour + longitude / 15.0 + 12) / 24) * 24) / 15.0;
  
  // Calculate hour angle
  float hourAngle = (solarTime - 12) * 15.0;
  
  // Calculate solar altitude
  float sinAltitude = sin(DEG_TO_RAD * latitude) * sin(DEG_TO_RAD * solarDeclination) + 
                      cos(DEG_TO_RAD * latitude) * cos(DEG_TO_RAD * solarDeclination) * cos(DEG_TO_RAD * hourAngle);
  *altitude = asin(sinAltitude) * RAD_TO_DEG;
  
  // Calculate solar azimuth
  float cosAzimuth = (sin(DEG_TO_RAD * solarDeclination) - sin(DEG_TO_RAD * latitude) * sinAltitude) / 
                     (cos(DEG_TO_RAD * latitude) * cos(asin(sinAltitude)));
  
  // Constrain cosAzimuth to [-1, 1] to avoid NaN
  cosAzimuth = constrain(cosAzimuth, -1.0, 1.0);
  
  *azimuth = acos(cosAzimuth) * RAD_TO_DEG;
  
  // Adjust azimuth based on hour angle
  if (hourAngle > 0) {
    *azimuth = 360.0 - *azimuth;
  }
}

// Moon position calculations - simplified algorithm
void calculateMoonPosition(float latitude, float longitude, 
                           int year, int month, int day, 
                           int hour, int minute, int second,
                           float *azimuth, float *altitude, float *phase) {
  // This is a placeholder for moon calculations
  // Accurate moon position calculation is complex and may require a specialized library
  
  // For now, we'll use a very simplified approximation
  // In a real implementation, consider using a library like Ephem
  
  // Calculate Julian Date
  unsigned long jd = getJulianDate(year, month, day);
  jd += (hour - 12) / 24.0 + minute / 1440.0 + second / 86400.0;
  
  // Calculate days since J2000.0
  double d = jd - 2451545.0;
  
  // Simplified lunar calculations - these are very approximate
  // Mean orbital elements of the Moon
  double L = fmod(218.316 + 13.176396 * d, 360.0) * DEG_TO_RAD; // Mean longitude
  double M = fmod(134.963 + 13.064993 * d, 360.0) * DEG_TO_RAD; // Mean anomaly
  double F = fmod(93.272 + 13.229350 * d, 360.0) * DEG_TO_RAD;  // Mean distance
  
  // Simplified longitude and latitude of the Moon
  double lon = L + 6.289 * sin(M) * DEG_TO_RAD;
  double lat = 5.128 * sin(F) * DEG_TO_RAD;
  
  // Convert to equatorial coordinates (simplified)
  double ra = atan2(sin(lon) * cos(23.4 * DEG_TO_RAD) - tan(lat) * sin(23.4 * DEG_TO_RAD), cos(lon));
  double dec = asin(sin(lat) * cos(23.4 * DEG_TO_RAD) + cos(lat) * sin(23.4 * DEG_TO_RAD) * sin(lon));
  
  // Calculate local sidereal time
  double lst = getSiderealTime(jd, longitude) * DEG_TO_RAD;
  
  // Calculate hour angle
  double ha = lst - ra;
  
  // Convert to horizontal coordinates
  double sinAlt = sin(dec) * sin(latitude * DEG_TO_RAD) + cos(dec) * cos(latitude * DEG_TO_RAD) * cos(ha);
  *altitude = asin(sinAlt) * RAD_TO_DEG;
  
  double cosAz = (sin(dec) - sin(latitude * DEG_TO_RAD) * sinAlt) / (cos(latitude * DEG_TO_RAD) * cos(asin(sinAlt)));
  cosAz = constrain(cosAz, -1.0, 1.0);
  *azimuth = acos(cosAz) * RAD_TO_DEG;
  
  if (sin(ha) > 0) {
    *azimuth = 360.0 - *azimuth;
  }
  
  // Calculate moon phase (0 = new moon, 0.5 = full moon, 1 = new moon again)
  double age = fmod(d, 29.53); // Moon's age in days
  *phase = age / 29.53;
}

// Utility functions
float calculateMagneticDeclination(float latitude, float longitude) {
  // This is a placeholder for magnetic declination calculation
  // In a real implementation, you would use a model like the World Magnetic Model
  // or a lookup table based on the user's location
  
  // For now, we'll return a fixed value
  // In reality, magnetic declination varies by location and time
  return 0.0;
}

void applyMagneticDeclination(float *heading, float declination) {
  // Apply magnetic declination to convert magnetic heading to true heading
  *heading += declination;
  
  // Normalize to 0-360 degrees
  while (*heading < 0) *heading += 360.0;
  while (*heading >= 360.0) *heading -= 360.0;
}
