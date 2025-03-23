/*
 * CelestialOverlay.h
 * 
 * Celestial overlay display for the Polaris Navigator
 * Handles rendering sun and moon positions on the compass display
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef CELESTIAL_OVERLAY_H
#define CELESTIAL_OVERLAY_H

#include <M5Unified.h>
#include "celestial_math.h"

// Moon phase definitions
enum MoonPhase {
  NEW_MOON = 0,
  WAXING_CRESCENT = 1,
  FIRST_QUARTER = 2,
  WAXING_GIBBOUS = 3,
  FULL_MOON = 4,
  WANING_GIBBOUS = 5,
  LAST_QUARTER = 6,
  WANING_CRESCENT = 7
};

class CelestialOverlay {
public:
  // Constructor
  CelestialOverlay();
  
  // Initialize overlay
  void begin();
  
  // Update celestial data
  void updateCelestialData(float lat, float lon, int year, int month, int day, 
                          int hour, int minute, int second);
  
  // Get sun position
  void getSunPosition(float* azimuth, float* altitude);
  
  // Get moon position
  void getMoonPosition(float* azimuth, float* altitude);
  
  // Get moon phase (0.0 to 1.0, where 0.0 is new moon and 1.0 is full moon)
  float getMoonPhase();
  
  // Get moon phase as enum
  MoonPhase getMoonPhaseEnum();
  
  // Get moon illumination percentage (0-100%)
  int getMoonIllumination();
  
  // Check if object is visible (above horizon)
  bool isSunVisible();
  bool isMoonVisible();
  
  // Get celestial event times (sunrise, sunset, moonrise, moonset)
  void getSunriseSunsetTime(int* sunriseHour, int* sunriseMinute, 
                           int* sunsetHour, int* sunsetMinute);
  
  void getMoonriseMoonsetTime(int* moonriseHour, int* moonriseMinute, 
                             int* moonsetHour, int* moonsetMinute);
  
  // Get time until next celestial event in minutes
  int getMinutesToNextSunrise();
  int getMinutesToNextSunset();
  int getMinutesToNextMoonrise();
  int getMinutesToNextMoonset();
  
  // Get Polaris position
  void getPolarisPosition(float* azimuth, float* altitude);
  
  // Debug output
  void printCelestialData();
  
private:
  // Celestial data
  float _sunAzimuth;
  float _sunAltitude;
  float _moonAzimuth;
  float _moonAltitude;
  float _moonPhase;
  float _polarisAzimuth;
  float _polarisAltitude;
  
  // Location and time data
  float _latitude;
  float _longitude;
  int _year, _month, _day;
  int _hour, _minute, _second;
  
  // Sunrise/sunset times
  int _sunriseHour, _sunriseMinute;
  int _sunsetHour, _sunsetMinute;
  
  // Moonrise/moonset times
  int _moonriseHour, _moonriseMinute;
  int _moonsetHour, _moonsetMinute;
  
  // Helper methods
  void calculateSunPosition();
  void calculateMoonPosition();
  void calculatePolarisPosition();
  void calculateSunriseSunset();
  void calculateMoonriseMoonset();
  
  // Convert phase value to enum
  MoonPhase phaseValueToEnum(float phase);
};

#endif // CELESTIAL_OVERLAY_H
