/*
 * AtomicBaseGPS.h
 * 
 * Interface for the AtomicBase GPS module
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef ATOMIC_BASE_GPS_H
#define ATOMIC_BASE_GPS_H

#include <Arduino.h>
#include <TinyGPS++.h>

class AtomicBaseGPS {
public:
  // Constructor
  AtomicBaseGPS();
  
  // Initialize GPS module
  bool begin(long baudRate = 9600);
  
  // Update GPS data (call this frequently)
  bool update();
  
  // Check if GPS data is valid
  bool isValid() const;
  
  // Get location data
  float getLatitude() const;
  float getLongitude() const;
  float getAltitude() const;
  
  // Get time data
  uint8_t getHour() const;
  uint8_t getMinute() const;
  uint8_t getSecond() const;
  uint16_t getYear() const;
  uint8_t getMonth() const;
  uint8_t getDay() const;
  
  // Get satellite data
  uint8_t getSatellites() const;
  
  // Get HDOP (Horizontal Dilution of Precision)
  float getHDOP() const;
  
  // Get speed in km/h
  float getSpeed() const;
  
  // Get course in degrees
  float getCourse() const;
  
  // Get raw TinyGPS++ object for advanced usage
  TinyGPSPlus* getRawGPS();
  
private:
  TinyGPSPlus _gps;
  bool _isValid;
  unsigned long _lastValidTime;
  
  // GPS data
  float _latitude;
  float _longitude;
  float _altitude;
  uint8_t _satellites;
  float _hdop;
  float _speed;
  float _course;
  
  // Time data
  uint8_t _hour;
  uint8_t _minute;
  uint8_t _second;
  uint16_t _year;
  uint8_t _month;
  uint8_t _day;
};

#endif // ATOMIC_BASE_GPS_H
