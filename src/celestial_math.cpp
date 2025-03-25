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
    
    // Calculate local sidereal time (LST) for current time
    // Get current date and time from system
    unsigned long currentTime = millis() / 1000; // seconds since startup
    
    // For a real implementation, you should use a real-time clock (RTC)
    // Here we'll use a fixed date/time for demonstration
    // In practice, this should be replaced with actual date/time
    int year = 2025;
    int month = 3;
    int day = 23;
    int hour = 20; // UTC time
    int minute = 0;
    int second = 0;
    
    // Calculate Julian Date
    unsigned long jd = getJulianDate(year, month, day);
    jd += (hour / 24.0) + (minute / 1440.0) + (second / 86400.0);
    
    // Calculate Local Sidereal Time
    double lst = getSiderealTime(jd, longitude);
    
    // Polaris is at RA 02h 31m 49s and Dec +89° 15' 51" (J2000)
    // Convert to radians
    double polarisRA = (2.0 + 31.0/60.0 + 49.0/3600.0) * 15.0 * DEG_TO_RAD; // 15 deg per hour
    double polarisDec = (89.0 + 15.0/60.0 + 51.0/3600.0) * DEG_TO_RAD;
    
    // Calculate hour angle of Polaris
    double ha = lst * DEG_TO_RAD - polarisRA;
    
    // Calculate azimuth of Polaris
    double latRad = latitude * DEG_TO_RAD;
    double sinAlt = sin(polarisDec) * sin(latRad) + cos(polarisDec) * cos(latRad) * cos(ha);
    double cosAlt = sqrt(1.0 - sinAlt * sinAlt);
    double sinAz = -cos(polarisDec) * sin(ha) / cosAlt;
    double cosAz = (sin(polarisDec) - sinAlt * sin(latRad)) / (cosAlt * cos(latRad));
    
    // Calculate azimuth in degrees
    *azimuth = atan2(sinAz, cosAz) * RAD_TO_DEG;
    
    // Normalize to 0-360 degrees
    while (*azimuth < 0) *azimuth += 360.0;
    while (*azimuth >= 360.0) *azimuth -= 360.0;
    
    // Adjust altitude for atmospheric refraction (simplified)
    float trueAltitude = asin(sinAlt) * RAD_TO_DEG;
    float refraction = 0.0;
    
    if (trueAltitude > -0.575) {
      refraction = 1.02 / tan((trueAltitude + 10.3 / (trueAltitude + 5.11)) * DEG_TO_RAD) / 60.0;
    }
    
    // If we want the position of Polaris instead of the exact celestial pole
    // we would use this altitude, but for polar alignment we want the pole
    // float polarisAltitude = trueAltitude + refraction;
    
    // For polar alignment, we keep the altitude as the latitude
    // *altitude = polarisAltitude;
  } else {
    // Southern hemisphere - South Celestial Pole
    *altitude = -latitude;
    
    // In the southern hemisphere, Sigma Octantis is the closest visible star to the pole
    // but it's much fainter than Polaris and harder to use for alignment
    // For simplicity, we'll just use the true south direction
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
  // Simplified World Magnetic Model (WMM) calculation
  // This is a simplified implementation based on WMM 2020
  // Valid for approximately 2020-2025
  
  // Convert latitude and longitude to radians
  float lat_rad = latitude * DEG_TO_RAD;
  float lon_rad = longitude * DEG_TO_RAD;
  
  // Coefficients for simplified WMM 2020 (valid ~2020-2025)
  // These are simplified coefficients and will give approximate results
  const float g01 = -29404.5;  // Main dipole term
  const float g11 = -1450.7;
  const float h11 = 4652.9;
  const float g02 = -2500.0;
  const float g12 = 2982.0;
  const float h12 = -2991.6;
  
  // Calculate magnetic field components
  float P1 = 1.0;                      // Legendre polynomial P(1,0)
  float P2 = sin(lat_rad);             // Legendre polynomial P(1,1)
  float P3 = (3.0 * P1 * P1 - 1.0)/2.0; // Legendre polynomial P(2,0)
  float P4 = 3.0 * P1 * P2;            // Legendre polynomial P(2,1)
  
  // North component (X)
  float X = g01 * P1 + 
            g11 * P2 * cos(lon_rad) + 
            h11 * P2 * sin(lon_rad) +
            g02 * P3 +
            g12 * P4 * cos(lon_rad) +
            h12 * P4 * sin(lon_rad);
  
  // East component (Y)
  float Y = g11 * P1 * sin(lon_rad) - 
            h11 * P1 * cos(lon_rad) +
            g12 * P2 * sin(lon_rad) -
            h12 * P2 * cos(lon_rad);
  
  // Calculate declination in degrees
  float declination = atan2(Y, X) * RAD_TO_DEG;
  
  // Apply regional corrections (simplified)
  // These corrections are very approximate and should be replaced with a proper model
  // or lookup table for production use
  
  // North America correction
  if (longitude >= -130.0 && longitude <= -60.0 && latitude >= 20.0 && latitude <= 60.0) {
    declination += 5.0 * sin((longitude + 95.0) * DEG_TO_RAD);
  }
  
  // Europe correction
  if (longitude >= -10.0 && longitude <= 40.0 && latitude >= 35.0 && latitude <= 70.0) {
    declination += 2.0 * sin((longitude - 15.0) * DEG_TO_RAD);
  }
  
  // Asia correction
  if (longitude >= 60.0 && longitude <= 150.0 && latitude >= 0.0 && latitude <= 60.0) {
    declination -= 3.0 * sin((longitude - 105.0) * DEG_TO_RAD);
  }
  
  // Japan specific correction (more accurate for the target region)
  if (longitude >= 125.0 && longitude <= 150.0 && latitude >= 30.0 && latitude <= 45.0) {
    declination = -7.5 + (latitude - 35.0) * 0.2 + (longitude - 135.0) * 0.1;
  }
  
  return declination;
}

void applyMagneticDeclination(float *heading, float declination) {
  // Apply magnetic declination to convert magnetic heading to true heading
  *heading += declination;
  
  // Normalize to 0-360 degrees
  while (*heading < 0) *heading += 360.0;
  while (*heading >= 360.0) *heading -= 360.0;
}

// Calculate sunrise and sunset times
void calculateSunriseSunset(float latitude, float longitude, int year, int month, int day,
                           int* sunriseHour, int* sunriseMinute, int* sunsetHour, int* sunsetMinute) {
  // Simple approximation for sunrise/sunset times
  // This is a placeholder - for accurate calculations, use a proper astronomical library
  
  // Convert date to day of year
  int dayOfYear = 0;
  for (int m = 1; m < month; m++) {
    if (m == 2) {
      dayOfYear += 28;
      // Check for leap year
      if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        dayOfYear += 1;
      }
    } else if (m == 4 || m == 6 || m == 9 || m == 11) {
      dayOfYear += 30;
    } else {
      dayOfYear += 31;
    }
  }
  dayOfYear += day;
  
  // Calculate approximate sunrise/sunset times based on latitude and day of year
  // This is a very simplified model
  float latRad = latitude * PI / 180.0;
  
  // Approximate solar declination
  float declination = 23.45 * sin(2.0 * PI * (284 + dayOfYear) / 365.0) * PI / 180.0;
  
  // Calculate day length in hours
  float dayLength = 24.0 - (24.0 / PI) * acos((sin(-0.83 * PI / 180.0) - sin(latRad) * sin(declination)) / 
                                            (cos(latRad) * cos(declination)));
  
  // Calculate noon time (solar noon)
  float solarNoon = 12.0 - longitude / 15.0;
  
  // Calculate sunrise and sunset times
  float sunrise = solarNoon - dayLength / 2.0;
  float sunset = solarNoon + dayLength / 2.0;
  
  // Convert to hours and minutes
  *sunriseHour = (int)sunrise;
  *sunriseMinute = (int)((sunrise - *sunriseHour) * 60);
  
  *sunsetHour = (int)sunset;
  *sunsetMinute = (int)((sunset - *sunsetHour) * 60);
  
  // Handle edge cases
  if (*sunriseHour < 0) {
    *sunriseHour += 24;
  } else if (*sunriseHour >= 24) {
    *sunriseHour -= 24;
  }
  
  if (*sunsetHour < 0) {
    *sunsetHour += 24;
  } else if (*sunsetHour >= 24) {
    *sunsetHour -= 24;
  }
}

// Calculate moonrise and moonset times
void calculateMoonriseMoonset(float latitude, float longitude, int year, int month, int day,
                             int* moonriseHour, int* moonriseMinute, int* moonsetHour, int* moonsetMinute) {
  // Simple approximation for moonrise/moonset times
  // This is a placeholder - for accurate calculations, use a proper astronomical library
  
  // For this simple implementation, we'll offset from sunrise/sunset
  // with a phase shift based on moon phase
  int sunriseHour, sunriseMinute, sunsetHour, sunsetMinute;
  calculateSunriseSunset(latitude, longitude, year, month, day, 
                        &sunriseHour, &sunriseMinute, &sunsetHour, &sunsetMinute);
  
  // Get moon phase
  float moonPhase = calculateMoonPhase(year, month, day);
  
  // Calculate offset in hours based on moon phase (0.0 to 1.0)
  // This creates a full cycle over the lunar month
  float phaseOffset = moonPhase * 24.0;
  
  // Convert sunrise/sunset to decimal hours
  float sunriseDecimal = sunriseHour + sunriseMinute / 60.0;
  float sunsetDecimal = sunsetHour + sunsetMinute / 60.0;
  
  // Calculate moonrise/moonset with offset
  float moonriseDecimal = fmod(sunriseDecimal + phaseOffset, 24.0);
  float moonsetDecimal = fmod(sunsetDecimal + phaseOffset, 24.0);
  
  // Convert back to hours and minutes
  *moonriseHour = (int)moonriseDecimal;
  *moonriseMinute = (int)((moonriseDecimal - *moonriseHour) * 60);
  
  *moonsetHour = (int)moonsetDecimal;
  *moonsetMinute = (int)((moonsetDecimal - *moonsetHour) * 60);
}

// Calculate moon phase (0.0 to 1.0)
float calculateMoonPhase(int year, int month, int day) {
  // Simple approximation for moon phase
  // This is a placeholder - for accurate calculations, use a proper astronomical library
  
  // Calculate approximate moon phase using a simple algorithm
  // Based on a lunar cycle of 29.53 days
  // Reference new moon: January 6, 2000
  
  // Convert date to days since reference
  long totalDays = 0;
  
  // Add days for years
  for (int y = 2000; y < year; y++) {
    if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) {
      totalDays += 366; // Leap year
    } else {
      totalDays += 365;
    }
  }
  
  // Add days for months in the current year
  for (int m = 1; m < month; m++) {
    if (m == 2) {
      if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        totalDays += 29; // February in leap year
      } else {
        totalDays += 28; // February in non-leap year
      }
    } else if (m == 4 || m == 6 || m == 9 || m == 11) {
      totalDays += 30;
    } else {
      totalDays += 31;
    }
  }
  
  // Add days in the current month
  totalDays += day;
  
  // Subtract reference date (January 6, 2000)
  totalDays -= 6;
  
  // Calculate phase based on 29.53 day lunar cycle
  // January 6, 2000 was a new moon
  float phase = fmod(totalDays, 29.53) / 29.53;
  
  return phase;
}
