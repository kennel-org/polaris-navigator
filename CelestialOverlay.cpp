/*
 * CelestialOverlay.cpp
 * 
 * Implementation for the Celestial overlay display
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "CelestialOverlay.h"
#include <math.h>

// Constructor
CelestialOverlay::CelestialOverlay() {
  // Initialize values
  _sunAzimuth = 0.0;
  _sunAltitude = 0.0;
  _moonAzimuth = 0.0;
  _moonAltitude = 0.0;
  _moonPhase = 0.0;
  _polarisAzimuth = 0.0;
  _polarisAltitude = 0.0;
  
  _latitude = 0.0;
  _longitude = 0.0;
  _year = 2025;
  _month = 1;
  _day = 1;
  _hour = 0;
  _minute = 0;
  _second = 0;
  
  _sunriseHour = 6;
  _sunriseMinute = 0;
  _sunsetHour = 18;
  _sunsetMinute = 0;
  
  _moonriseHour = 0;
  _moonriseMinute = 0;
  _moonsetHour = 0;
  _moonsetMinute = 0;
}

// Initialize overlay
void CelestialOverlay::begin() {
  // Nothing to initialize yet
}

// Update celestial data
void CelestialOverlay::updateCelestialData(float lat, float lon, int year, int month, int day, 
                                         int hour, int minute, int second) {
  // Store location and time
  _latitude = lat;
  _longitude = lon;
  _year = year;
  _month = month;
  _day = day;
  _hour = hour;
  _minute = minute;
  _second = second;
  
  // Calculate positions
  calculateSunPosition();
  calculateMoonPosition();
  calculatePolarisPosition();
  calculateSunriseSunset();
  calculateMoonriseMoonset();
  calculateMoonPhase();
}

// Get sun position
void CelestialOverlay::getSunPosition(float* azimuth, float* altitude) {
  *azimuth = _sunAzimuth;
  *altitude = _sunAltitude;
}

// Get moon position
void CelestialOverlay::getMoonPosition(float* azimuth, float* altitude) {
  *azimuth = _moonAzimuth;
  *altitude = _moonAltitude;
}

// Get moon phase (0.0 to 1.0, where 0.0 is new moon and 1.0 is full moon)
float CelestialOverlay::getMoonPhase() {
  return _moonPhase;
}

// Get moon phase as enum
MoonPhase CelestialOverlay::getMoonPhaseEnum() {
  return phaseValueToEnum(_moonPhase);
}

// Get moon illumination percentage (0-100%)
int CelestialOverlay::getMoonIllumination() {
  // Convert phase to illumination percentage
  // 0.0 = new moon (0%), 0.5 = full moon (100%), 1.0 = new moon (0%)
  float illumination;
  if (_moonPhase <= 0.5) {
    illumination = _moonPhase * 2.0; // 0.0 to 1.0
  } else {
    illumination = (1.0 - _moonPhase) * 2.0; // 1.0 to 0.0
  }
  return round(illumination * 100);
}

// Check if object is visible (above horizon)
bool CelestialOverlay::isSunVisible() {
  return _sunAltitude > 0;
}

bool CelestialOverlay::isMoonVisible() {
  return _moonAltitude > 0;
}

// Get celestial event times (sunrise, sunset, moonrise, moonset)
void CelestialOverlay::getSunriseSunsetTime(int* sunriseHour, int* sunriseMinute, 
                                          int* sunsetHour, int* sunsetMinute) {
  *sunriseHour = _sunriseHour;
  *sunriseMinute = _sunriseMinute;
  *sunsetHour = _sunsetHour;
  *sunsetMinute = _sunsetMinute;
}

void CelestialOverlay::getMoonriseMoonsetTime(int* moonriseHour, int* moonriseMinute, 
                                            int* moonsetHour, int* moonsetMinute) {
  *moonriseHour = _moonriseHour;
  *moonriseMinute = _moonriseMinute;
  *moonsetHour = _moonsetHour;
  *moonsetMinute = _moonsetMinute;
}

// Get time until next celestial event in minutes
int CelestialOverlay::getMinutesToNextSunrise() {
  int currentMinutes = _hour * 60 + _minute;
  int sunriseMinutes = _sunriseHour * 60 + _sunriseMinute;
  
  if (currentMinutes >= sunriseMinutes) {
    // Sunrise has already occurred today, calculate for tomorrow
    return (24 * 60 - currentMinutes) + sunriseMinutes;
  } else {
    // Sunrise is later today
    return sunriseMinutes - currentMinutes;
  }
}

int CelestialOverlay::getMinutesToNextSunset() {
  int currentMinutes = _hour * 60 + _minute;
  int sunsetMinutes = _sunsetHour * 60 + _sunsetMinute;
  
  if (currentMinutes >= sunsetMinutes) {
    // Sunset has already occurred today, calculate for tomorrow
    return (24 * 60 - currentMinutes) + sunsetMinutes;
  } else {
    // Sunset is later today
    return sunsetMinutes - currentMinutes;
  }
}

int CelestialOverlay::getMinutesToNextMoonrise() {
  int currentMinutes = _hour * 60 + _minute;
  int moonriseMinutes = _moonriseHour * 60 + _moonriseMinute;
  
  if (currentMinutes >= moonriseMinutes) {
    // Moonrise has already occurred today, calculate for tomorrow
    return (24 * 60 - currentMinutes) + moonriseMinutes;
  } else {
    // Moonrise is later today
    return moonriseMinutes - currentMinutes;
  }
}

int CelestialOverlay::getMinutesToNextMoonset() {
  int currentMinutes = _hour * 60 + _minute;
  int moonsetMinutes = _moonsetHour * 60 + _moonsetMinute;
  
  if (currentMinutes >= moonsetMinutes) {
    // Moonset has already occurred today, calculate for tomorrow
    return (24 * 60 - currentMinutes) + moonsetMinutes;
  } else {
    // Moonset is later today
    return moonsetMinutes - currentMinutes;
  }
}

// Get Polaris position
void CelestialOverlay::getPolarisPosition(float* azimuth, float* altitude) {
  *azimuth = _polarisAzimuth;
  *altitude = _polarisAltitude;
}

// Debug output
void CelestialOverlay::printCelestialData() {
  Serial.println("Celestial Data:");
  Serial.print("Sun: Az=");
  Serial.print(_sunAzimuth);
  Serial.print(", Alt=");
  Serial.println(_sunAltitude);
  
  Serial.print("Moon: Az=");
  Serial.print(_moonAzimuth);
  Serial.print(", Alt=");
  Serial.print(_moonAltitude);
  Serial.print(", Phase=");
  Serial.print(_moonPhase);
  Serial.print(" (");
  
  // Print moon phase name
  switch (getMoonPhaseEnum()) {
    case NEW_MOON:
      Serial.print("New Moon");
      break;
    case WAXING_CRESCENT:
      Serial.print("Waxing Crescent");
      break;
    case FIRST_QUARTER:
      Serial.print("First Quarter");
      break;
    case WAXING_GIBBOUS:
      Serial.print("Waxing Gibbous");
      break;
    case FULL_MOON:
      Serial.print("Full Moon");
      break;
    case WANING_GIBBOUS:
      Serial.print("Waning Gibbous");
      break;
    case LAST_QUARTER:
      Serial.print("Last Quarter");
      break;
    case WANING_CRESCENT:
      Serial.print("Waning Crescent");
      break;
  }
  Serial.print("), Illumination=");
  Serial.print(getMoonIllumination());
  Serial.println("%");
  
  Serial.print("Polaris: Az=");
  Serial.print(_polarisAzimuth);
  Serial.print(", Alt=");
  Serial.println(_polarisAltitude);
  
  Serial.print("Sunrise: ");
  Serial.print(_sunriseHour);
  Serial.print(":");
  if (_sunriseMinute < 10) Serial.print("0");
  Serial.print(_sunriseMinute);
  Serial.print(", Sunset: ");
  Serial.print(_sunsetHour);
  Serial.print(":");
  if (_sunsetMinute < 10) Serial.print("0");
  Serial.println(_sunsetMinute);
  
  Serial.print("Moonrise: ");
  Serial.print(_moonriseHour);
  Serial.print(":");
  if (_moonriseMinute < 10) Serial.print("0");
  Serial.print(_moonriseMinute);
  Serial.print(", Moonset: ");
  Serial.print(_moonsetHour);
  Serial.print(":");
  if (_moonsetMinute < 10) Serial.print("0");
  Serial.println(_moonsetMinute);
}

// Private methods

// Calculate sun position
void CelestialOverlay::calculateSunPosition() {
  // Use celestial_math library to calculate sun position
  calculateSunPosition(_latitude, _longitude, _year, _month, _day, _hour, _minute, _second, &_sunAzimuth, &_sunAltitude);
}

// Calculate moon position
void CelestialOverlay::calculateMoonPosition() {
  // Use celestial_math library to calculate moon position
  calculateMoonPosition(_latitude, _longitude, _year, _month, _day, _hour, _minute, _second, &_moonAzimuth, &_moonAltitude);
}

// Calculate Polaris position
void CelestialOverlay::calculatePolarisPosition() {
  // Use celestial_math library to calculate Polaris position
  calculatePolarisPosition(_latitude, _longitude, _year, _month, _day, _hour, _minute, _second, &_polarisAzimuth, &_polarisAltitude);
}

// Calculate sunrise and sunset times
void CelestialOverlay::calculateSunriseSunset() {
  // Use celestial_math library to calculate sunrise and sunset
  calculateSunriseSunset(_latitude, _longitude, _year, _month, _day, &_sunriseHour, &_sunriseMinute, &_sunsetHour, &_sunsetMinute);
}

// Calculate moonrise and moonset times
void CelestialOverlay::calculateMoonriseMoonset() {
  // Use celestial_math library to calculate moonrise and moonset
  calculateMoonriseMoonset(_latitude, _longitude, _year, _month, _day, &_moonriseHour, &_moonriseMinute, &_moonsetHour, &_moonsetMinute);
}

// Calculate moon phase
void CelestialOverlay::calculateMoonPhase() {
  // Use celestial_math library to calculate moon phase
  _moonPhase = calculateMoonPhase(_year, _month, _day);
}

// Convert phase value to enum
MoonPhase CelestialOverlay::phaseValueToEnum(float phase) {
  // Convert phase value (0.0 to 1.0) to moon phase enum
  if (phase < 0.0625 || phase >= 0.9375) {
    return NEW_MOON;
  } else if (phase < 0.1875) {
    return WAXING_CRESCENT;
  } else if (phase < 0.3125) {
    return FIRST_QUARTER;
  } else if (phase < 0.4375) {
    return WAXING_GIBBOUS;
  } else if (phase < 0.5625) {
    return FULL_MOON;
  } else if (phase < 0.6875) {
    return WANING_GIBBOUS;
  } else if (phase < 0.8125) {
    return LAST_QUARTER;
  } else {
    return WANING_CRESCENT;
  }
}
