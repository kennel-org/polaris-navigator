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
  
  // Initialize cached values
  _latitude = 0.0;
  _longitude = 0.0;
  _altitude = 0.0;
  _satellites = 0;
  _hdop = 99.99;
  _speed = 0.0;
  _course = 0.0;
}

// Initialize GPS with specified baud rate
bool AtomicBaseGPS::begin(unsigned long baud) {
  // Use Serial2 for GPS communication on AtomS3R
  Serial2.begin(baud, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);
  _serial = &Serial2;
  
  // Wait a moment for GPS to initialize
  delay(100);
  
  Serial.println("AtomicBase GPS initialized on pin " + String(GPS_TX_PIN));
  
  // Clear any existing data in the buffer
  while (_serial->available()) {
    _serial->read();
  }
  
  return true;
}

// Update GPS data (call this regularly)
void AtomicBaseGPS::update() {
  bool newData = false;
  unsigned long startTime = millis();
  
  // Read all available data from GPS with timeout
  while (_serial->available() > 0 && (millis() - startTime < 100)) {
    char c = _serial->read();
    newData = _gps.encode(c) || newData;
    
    // Collect NMEA sentence for debugging
    if (c == '$') {
      // Start of a new NMEA sentence
      _lastNMEA = String(c);
    } else if (_lastNMEA.length() > 0) {
      // Continue collecting the NMEA sentence
      _lastNMEA += c;
      
      // End of NMEA sentence
      if (c == '\n') {
        // Only print NMEA sentences occasionally to avoid flooding Serial
        static unsigned long lastNmeaPrint = 0;
        if (millis() - lastNmeaPrint > 5000) {  // Print every 5 seconds
          Serial.print("NMEA: ");
          Serial.print(_lastNMEA);
          lastNmeaPrint = millis();
        }
      }
    }
  }
  
  // Report if new data was processed
  static unsigned long lastStatusReport = 0;
  if (millis() - lastStatusReport > 10000) {  // Every 10 seconds
    Serial.print("GPS Status: ");
    if (newData) {
      Serial.println("Receiving data");
    } else {
      Serial.println("No new data");
    }
    
    Serial.print("Satellites: ");
    if (_gps.satellites.isValid()) {
      Serial.println(_satellites);
    } else {
      Serial.println("Invalid");
    }
    
    Serial.print("Location valid: ");
    Serial.println(_gps.location.isValid() ? "Yes" : "No");
    
    lastStatusReport = millis();
  }
  
  // Update cached values if valid
  if (_gps.location.isValid()) {
    _latitude = _gps.location.lat();
    _longitude = _gps.location.lng();
    _isValid = true;
    _lastValidFix = millis();
  }
  
  if (_gps.altitude.isValid()) {
    _altitude = _gps.altitude.meters();
  }
  
  if (_gps.satellites.isValid()) {
    _satellites = _gps.satellites.value();
  }
  
  if (_gps.hdop.isValid()) {
    _hdop = _gps.hdop.hdop();
  }
  
  if (_gps.speed.isValid()) {
    _speed = _gps.speed.kmph();
  }
  
  if (_gps.course.isValid()) {
    _course = _gps.course.deg();
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
bool AtomicBaseGPS::isValid() const {
  return _isValid;
}

// Get location data
float AtomicBaseGPS::getLatitude() const {
  return _latitude;
}

float AtomicBaseGPS::getLongitude() const {
  return _longitude;
}

float AtomicBaseGPS::getAltitude() const {
  return _altitude;
}

// Get quality indicators
int AtomicBaseGPS::getSatellites() const {
  return _satellites;
}

float AtomicBaseGPS::getHDOP() const {
  return _hdop;
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

// Get speed and course
float AtomicBaseGPS::getSpeed() const {
  return _speed;
}

float AtomicBaseGPS::getCourse() const {
  return _course;
}

// Get raw NMEA sentence (for debugging)
String AtomicBaseGPS::getLastNMEA() const {
  return _lastNMEA;
}

// Get raw TinyGPS++ object for advanced usage
TinyGPSPlus* AtomicBaseGPS::getRawGPS() {
  return &_gps;
}
