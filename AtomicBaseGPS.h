/*
 * AtomicBaseGPS.h
 * 
 * Interface for the AtomicBase GPS module
 * Handles GPS data parsing and provides location and time information
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef ATOMIC_BASE_GPS_H
#define ATOMIC_BASE_GPS_H

#include <Arduino.h>
#include <TinyGPS++.h>

// Default pins for AtomicBase GPS
#define GPS_TX_PIN 5    // GPS TX pin (connects to RX of AtomS3R)
#define GPS_RX_PIN -1   // GPS RX pin (not used for one-way communication)

class AtomicBaseGPS {
public:
  // Constructor
  AtomicBaseGPS();
  
  // Initialize GPS with specified baud rate
  bool begin(unsigned long baud = 9600);
  
  // Update GPS data (call this regularly)
  void update();
  
  // Check if GPS data is valid
  bool isValid();
  
  // Get location data
  float getLatitude();
  float getLongitude();
  float getAltitude();
  
  // Get quality indicators
  int getSatellites();
  float getHDOP();  // Horizontal Dilution of Precision
  
  // Get time data
  bool getTime(int *hour, int *minute, int *second);
  bool getDate(int *year, int *month, int *day);
  
  // Get raw NMEA sentence (for debugging)
  String getLastNMEA();
  
private:
  TinyGPSPlus _gps;        // GPS parser
  HardwareSerial *_serial; // Serial port for GPS
  bool _isValid;           // Flag for valid GPS data
  unsigned long _lastValidFix; // Timestamp of last valid fix
  String _lastNMEA;        // Last NMEA sentence received
};

#endif // ATOMIC_BASE_GPS_H
