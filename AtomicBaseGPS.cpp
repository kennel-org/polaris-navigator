/*
 * AtomicBaseGPS.cpp
 * 
 * Implementation for the AtomicBase GPS module interface
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "AtomicBaseGPS.h"

// Constructor
AtomicBaseGPS::AtomicBaseGPS() {
  _isValid = false;
  _lastValidFix = 0;
  _serial = nullptr;
}

// Initialize GPS with specified baud rate
bool AtomicBaseGPS::begin(unsigned long baud) {
  // Use Serial2 for GPS communication on AtomS3R
  Serial2.begin(baud, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);
  _serial = &Serial2;
  
  Serial.println("AtomicBase GPS initialized on pin " + String(GPS_TX_PIN));
  return true;
}

// Update GPS data (call this regularly)
void AtomicBaseGPS::update() {
  // Read all available data from GPS
  while (_serial->available() > 0) {
    char c = _serial->read();
    _gps.encode(c);
    
    // Collect NMEA sentence for debugging
    if (c == '$') {
      // Start of a new NMEA sentence
      _lastNMEA = String(c);
    } else if (_lastNMEA.length() > 0) {
      // Continue collecting the NMEA sentence
      _lastNMEA += c;
      
      // End of NMEA sentence
      if (c == '\n') {
        Serial.print("NMEA: ");
        Serial.print(_lastNMEA);
      }
    }
  }
  
  // Check if we have a valid fix
  if (_gps.location.isValid() && _gps.satellites.isValid()) {
    _isValid = true;
    _lastValidFix = millis();
  } else if (millis() - _lastValidFix > 10000) {
    // If no valid fix for 10 seconds, mark as invalid
    _isValid = false;
  }
}

// Check if GPS data is valid
bool AtomicBaseGPS::isValid() {
  return _isValid && _gps.location.isValid();
}

// Get location data
float AtomicBaseGPS::getLatitude() {
  return _gps.location.lat();
}

float AtomicBaseGPS::getLongitude() {
  return _gps.location.lng();
}

float AtomicBaseGPS::getAltitude() {
  return _gps.altitude.meters();
}

// Get quality indicators
int AtomicBaseGPS::getSatellites() {
  return _gps.satellites.value();
}

float AtomicBaseGPS::getHDOP() {
  return _gps.hdop.hdop();
}

// Get time data
bool AtomicBaseGPS::getTime(int *hour, int *minute, int *second) {
  if (!_gps.time.isValid()) {
    return false;
  }
  
  *hour = _gps.time.hour();
  *minute = _gps.time.minute();
  *second = _gps.time.second();
  
  return true;
}

bool AtomicBaseGPS::getDate(int *year, int *month, int *day) {
  if (!_gps.date.isValid()) {
    return false;
  }
  
  *year = _gps.date.year();
  *month = _gps.date.month();
  *day = _gps.date.day();
  
  return true;
}

// Get raw NMEA sentence (for debugging)
String AtomicBaseGPS::getLastNMEA() {
  return _lastNMEA;
}
