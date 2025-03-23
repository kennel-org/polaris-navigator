/*
 * AtomicBaseGPS.cpp
 * 
 * Implementation for the AtomicBase GPS module
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "AtomicBaseGPS.h"

// Pin definitions for AtomicBase GPS
#define GPS_TX_PIN 5   // GPS TX pin connected to this pin on AtomS3R
#define GPS_RX_PIN -1  // Not used (one-way communication from GPS to AtomS3R)

AtomicBaseGPS::AtomicBaseGPS() : 
  _isValid(false),
  _lastValidTime(0),
  _latitude(0.0),
  _longitude(0.0),
  _altitude(0.0),
  _satellites(0),
  _hdop(99.99),
  _speed(0.0),
  _course(0.0),
  _hour(0),
  _minute(0),
  _second(0),
  _year(0),
  _month(0),
  _day(0) {
}

bool AtomicBaseGPS::begin(long baudRate) {
  // Initialize GPS on Serial2 with specified baud rate
  Serial2.begin(baudRate, SERIAL_8N1, GPS_TX_PIN, GPS_RX_PIN);
  
  // Wait a moment for GPS to initialize
  delay(100);
  
  return true;
}

bool AtomicBaseGPS::update() {
  // Read data from GPS
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    if (_gps.encode(c)) {
      // New data was parsed
      
      // Update location data if valid
      if (_gps.location.isValid()) {
        _latitude = _gps.location.lat();
        _longitude = _gps.location.lng();
        _isValid = true;
        _lastValidTime = millis();
      }
      
      // Update altitude if valid
      if (_gps.altitude.isValid()) {
        _altitude = _gps.altitude.meters();
      }
      
      // Update satellite data if valid
      if (_gps.satellites.isValid()) {
        _satellites = _gps.satellites.value();
      }
      
      // Update HDOP if valid
      if (_gps.hdop.isValid()) {
        _hdop = _gps.hdop.hdop();
      }
      
      // Update speed if valid
      if (_gps.speed.isValid()) {
        _speed = _gps.speed.kmph();
      }
      
      // Update course if valid
      if (_gps.course.isValid()) {
        _course = _gps.course.deg();
      }
      
      // Update time if valid
      if (_gps.time.isValid()) {
        _hour = _gps.time.hour();
        _minute = _gps.time.minute();
        _second = _gps.time.second();
      }
      
      // Update date if valid
      if (_gps.date.isValid()) {
        _year = _gps.date.year();
        _month = _gps.date.month();
        _day = _gps.date.day();
      }
      
      return true;
    }
  }
  
  // Check if we've lost GPS signal for more than 5 seconds
  if (_isValid && (millis() - _lastValidTime > 5000)) {
    _isValid = false;
  }
  
  return false;
}

bool AtomicBaseGPS::isValid() const {
  return _isValid;
}

float AtomicBaseGPS::getLatitude() const {
  return _latitude;
}

float AtomicBaseGPS::getLongitude() const {
  return _longitude;
}

float AtomicBaseGPS::getAltitude() const {
  return _altitude;
}

uint8_t AtomicBaseGPS::getHour() const {
  return _hour;
}

uint8_t AtomicBaseGPS::getMinute() const {
  return _minute;
}

uint8_t AtomicBaseGPS::getSecond() const {
  return _second;
}

uint16_t AtomicBaseGPS::getYear() const {
  return _year;
}

uint8_t AtomicBaseGPS::getMonth() const {
  return _month;
}

uint8_t AtomicBaseGPS::getDay() const {
  return _day;
}

uint8_t AtomicBaseGPS::getSatellites() const {
  return _satellites;
}

float AtomicBaseGPS::getHDOP() const {
  return _hdop;
}

float AtomicBaseGPS::getSpeed() const {
  return _speed;
}

float AtomicBaseGPS::getCourse() const {
  return _course;
}

TinyGPSPlus* AtomicBaseGPS::getRawGPS() {
  return &_gps;
}
