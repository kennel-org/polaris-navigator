/*
 * RawDataDisplay.cpp
 * 
 * Implementation for the Raw data display
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "RawDataDisplay.h"
#include <math.h>

// Constructor
RawDataDisplay::RawDataDisplay() {
  _detailedView = false;
  _lastUpdateTime = 0;
  _currentPage = 0;
  _currentMode = RAW_IMU;
}

// Initialize display
void RawDataDisplay::begin() {
  // Nothing to initialize for now
}

// Update display with raw data
void RawDataDisplay::update(RawDataMode mode) {
  _currentMode = mode;
  
  // Update LED based on mode
  switch (mode) {
    case RAW_IMU:
      M5.dis.drawpix(0, 0x0000FF); // Blue for IMU
      break;
    case RAW_GPS:
      M5.dis.drawpix(0, 0x00FF00); // Green for GPS
      break;
    case RAW_CELESTIAL:
      M5.dis.drawpix(0, 0xFF00FF); // Purple for Celestial
      break;
    case RAW_SYSTEM:
      M5.dis.drawpix(0, 0xFFFF00); // Yellow for System
      break;
    case RAW_DEBUG:
      M5.dis.drawpix(0, 0xFF0000); // Red for Debug
      break;
  }
}

// Display raw IMU data
void RawDataDisplay::showRawIMU(BMI270* bmi270, BMM150class* bmm150, 
                              float heading, float pitch, float roll,
                              bool calibrated) {
  // Print raw IMU data to serial
  printRawIMUData(bmi270, bmm150);
  
  // Update LED based on calibration status
  if (calibrated) {
    M5.dis.drawpix(0, 0x0000FF); // Blue for calibrated
  } else {
    M5.dis.drawpix(0, 0xFF00FF); // Purple for uncalibrated
  }
  
  // Print formatted data
  Serial.println("=== IMU DATA ===");
  Serial.print("Heading: ");
  Serial.print(heading, 1);
  Serial.println(" deg");
  
  Serial.print("Pitch: ");
  Serial.print(pitch, 1);
  Serial.println(" deg");
  
  Serial.print("Roll: ");
  Serial.print(roll, 1);
  Serial.println(" deg");
  
  Serial.print("Calibration: ");
  Serial.println(calibrated ? "YES" : "NO");
  
  // Print raw accelerometer data if in detailed view
  if (_detailedView) {
    float accX, accY, accZ;
    bmi270->getAcceleration(&accX, &accY, &accZ);
    
    Serial.println("=== ACCELEROMETER ===");
    Serial.print("X: ");
    Serial.print(accX, 4);
    Serial.println(" g");
    
    Serial.print("Y: ");
    Serial.print(accY, 4);
    Serial.println(" g");
    
    Serial.print("Z: ");
    Serial.print(accZ, 4);
    Serial.println(" g");
    
    // Print raw gyroscope data
    float gyrX, gyrY, gyrZ;
    bmi270->getGyroscope(&gyrX, &gyrY, &gyrZ);
    
    Serial.println("=== GYROSCOPE ===");
    Serial.print("X: ");
    Serial.print(gyrX, 4);
    Serial.println(" deg/s");
    
    Serial.print("Y: ");
    Serial.print(gyrY, 4);
    Serial.println(" deg/s");
    
    Serial.print("Z: ");
    Serial.print(gyrZ, 4);
    Serial.println(" deg/s");
    
    // Print raw magnetometer data
    float magX, magY, magZ;
    bmm150->getMagnetometer(&magX, &magY, &magZ);
    
    Serial.println("=== MAGNETOMETER ===");
    Serial.print("X: ");
    Serial.print(magX, 4);
    Serial.println(" uT");
    
    Serial.print("Y: ");
    Serial.print(magY, 4);
    Serial.println(" uT");
    
    Serial.print("Z: ");
    Serial.print(magZ, 4);
    Serial.println(" uT");
  }
}

// Display raw GPS data
void RawDataDisplay::showRawGPS(AtomicBaseGPS* gps, 
                              float latitude, float longitude, float altitude,
                              int satellites, float hdop, 
                              int hour, int minute, int second,
                              bool valid) {
  // Print raw GPS data to serial
  printRawGPSData(gps);
  
  // Update LED based on GPS validity
  if (valid) {
    M5.dis.drawpix(0, 0x00FF00); // Green for valid
  } else {
    M5.dis.drawpix(0, 0xFF0000); // Red for invalid
  }
  
  // Print formatted data
  Serial.println("=== GPS DATA ===");
  
  char buffer[32];
  
  // Format and print latitude
  formatCoordinateValue(buffer, latitude, true);
  Serial.print("Latitude: ");
  Serial.println(buffer);
  
  // Format and print longitude
  formatCoordinateValue(buffer, longitude, false);
  Serial.print("Longitude: ");
  Serial.println(buffer);
  
  Serial.print("Altitude: ");
  Serial.print(altitude, 1);
  Serial.println(" m");
  
  Serial.print("Satellites: ");
  Serial.println(satellites);
  
  Serial.print("HDOP: ");
  Serial.println(hdop, 1);
  
  // Format and print time
  formatTimeValue(buffer, hour, minute, second);
  Serial.print("Time (UTC): ");
  Serial.println(buffer);
  
  // Print detailed GPS data if in detailed view
  if (_detailedView) {
    int year, month, day;
    gps->getDate(&year, &month, &day);
    
    // Format and print date
    formatDateValue(buffer, year, month, day);
    Serial.print("Date: ");
    Serial.println(buffer);
    
    float speed = gps->getSpeed();
    Serial.print("Speed: ");
    Serial.print(speed, 1);
    Serial.println(" km/h");
    
    float course = gps->getCourse();
    Serial.print("Course: ");
    Serial.print(course, 1);
    Serial.println(" deg");
    
    // Print raw NMEA sentences if available and in detailed view
    Serial.println("=== LAST NMEA SENTENCE ===");
    Serial.println(gps->getLastNMEA());
  }
}

// Display raw celestial data
void RawDataDisplay::showRawCelestial(float sunAz, float sunAlt, 
                                    float moonAz, float moonAlt, float moonPhase,
                                    float polarisAz, float polarisAlt) {
  // Print raw celestial data to serial
  printRawCelestialData(sunAz, sunAlt, moonAz, moonAlt, moonPhase, polarisAz, polarisAlt);
  
  // Update LED based on Polaris visibility
  if (polarisAlt > 0) {
    M5.dis.drawpix(0, 0x00FFFF); // Cyan for visible Polaris
  } else {
    M5.dis.drawpix(0, 0xFF00FF); // Purple for invisible Polaris
  }
  
  // Print formatted data
  Serial.println("=== CELESTIAL DATA ===");
  
  Serial.println("=== SUN ===");
  Serial.print("Azimuth: ");
  Serial.print(sunAz, 1);
  Serial.println(" deg");
  
  Serial.print("Altitude: ");
  Serial.print(sunAlt, 1);
  Serial.println(" deg");
  
  Serial.print("Visible: ");
  Serial.println(sunAlt > 0 ? "YES" : "NO");
  
  Serial.println("=== MOON ===");
  Serial.print("Azimuth: ");
  Serial.print(moonAz, 1);
  Serial.println(" deg");
  
  Serial.print("Altitude: ");
  Serial.print(moonAlt, 1);
  Serial.println(" deg");
  
  Serial.print("Phase: ");
  Serial.print(moonPhase * 100, 1);
  Serial.println("%");
  
  Serial.print("Visible: ");
  Serial.println(moonAlt > 0 ? "YES" : "NO");
  
  Serial.println("=== POLARIS ===");
  Serial.print("Azimuth: ");
  Serial.print(polarisAz, 1);
  Serial.println(" deg");
  
  Serial.print("Altitude: ");
  Serial.print(polarisAlt, 1);
  Serial.println(" deg");
  
  Serial.print("Visible: ");
  Serial.println(polarisAlt > 0 ? "YES" : "NO");
}

// Display system information
void RawDataDisplay::showSystemInfo(float batteryLevel, float temperature, 
                                  unsigned long uptime, int freeMemory) {
  // Print system information to serial
  printSystemInfo(batteryLevel, temperature, uptime, freeMemory);
  
  // Update LED based on battery level
  if (batteryLevel > 75) {
    M5.dis.drawpix(0, 0x00FF00); // Green for good battery
  } else if (batteryLevel > 25) {
    M5.dis.drawpix(0, 0xFFFF00); // Yellow for medium battery
  } else {
    M5.dis.drawpix(0, 0xFF0000); // Red for low battery
  }
  
  // Print formatted data
  Serial.println("=== SYSTEM INFO ===");
  
  Serial.print("Battery: ");
  Serial.print(batteryLevel, 1);
  Serial.println("%");
  
  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.println(" C");
  
  // Format uptime
  unsigned long seconds = uptime / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds %= 60;
  minutes %= 60;
  
  Serial.print("Uptime: ");
  Serial.print(hours);
  Serial.print("h ");
  Serial.print(minutes);
  Serial.print("m ");
  Serial.print(seconds);
  Serial.println("s");
  
  Serial.print("Free Memory: ");
  Serial.print(freeMemory);
  Serial.println(" bytes");
  
  // Print more detailed system info if in detailed view
  if (_detailedView) {
    Serial.println("=== DETAILED SYSTEM INFO ===");
    
    // Print firmware version
    Serial.print("Firmware: ");
    Serial.println("Polaris Navigator v1.0");
    
    // Print hardware info
    Serial.println("Hardware: AtomS3R with AtomicBase GPS");
    
    // Print compilation date and time
    Serial.print("Compiled: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);
  }
}

// Display debug information
void RawDataDisplay::showDebugInfo(const char* debugMessage) {
  // Print debug information
  Serial.println("=== DEBUG INFO ===");
  Serial.println(debugMessage);
  
  // Flash LED to indicate debug mode
  M5.dis.drawpix(0, 0xFF0000); // Red
  delay(100);
  M5.dis.drawpix(0, 0x000000); // Off
  delay(100);
  M5.dis.drawpix(0, 0xFF0000); // Red
}

// Toggle detailed view
void RawDataDisplay::toggleDetailedView() {
  _detailedView = !_detailedView;
  
  Serial.print("Detailed view: ");
  Serial.println(_detailedView ? "ON" : "OFF");
}

// Set LED color based on data quality
void RawDataDisplay::setDataQualityIndicator(float quality) {
  // quality should be between 0.0 and 1.0
  if (quality < 0.0) quality = 0.0;
  if (quality > 1.0) quality = 1.0;
  
  // Map quality to color (red -> yellow -> green)
  uint8_t red, green;
  
  if (quality < 0.5) {
    // Red to Yellow (increase green)
    red = 255;
    green = 255 * (quality * 2);
  } else {
    // Yellow to Green (decrease red)
    green = 255;
    red = 255 * (2 - quality * 2);
  }
  
  uint32_t color = (red << 16) | (green << 8);
  M5.dis.drawpix(0, color);
}

// Helper methods

void RawDataDisplay::formatFloatValue(char* buffer, float value, int precision) {
  // Format float value with specified precision
  dtostrf(value, 0, precision, buffer);
}

void RawDataDisplay::formatTimeValue(char* buffer, int hours, int minutes, int seconds) {
  // Format time as HH:MM:SS
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
}

void RawDataDisplay::formatDateValue(char* buffer, int year, int month, int day) {
  // Format date as YYYY-MM-DD
  sprintf(buffer, "%04d-%02d-%02d", year, month, day);
}

void RawDataDisplay::formatCoordinateValue(char* buffer, float value, bool isLatitude) {
  // Format coordinate as degrees, minutes, seconds with direction
  float absValue = fabs(value);
  int degrees = (int)absValue;
  float minutesFloat = (absValue - degrees) * 60.0;
  int minutes = (int)minutesFloat;
  float seconds = (minutesFloat - minutes) * 60.0;
  
  char direction;
  if (isLatitude) {
    direction = (value >= 0) ? 'N' : 'S';
  } else {
    direction = (value >= 0) ? 'E' : 'W';
  }
  
  sprintf(buffer, "%d° %d' %.1f\" %c", degrees, minutes, seconds, direction);
}

// Serial output helpers

void RawDataDisplay::printRawIMUData(BMI270* bmi270, BMM150class* bmm150) {
  // Print raw IMU data to serial in CSV format for logging
  float accX, accY, accZ;
  float gyrX, gyrY, gyrZ;
  float magX, magY, magZ;
  
  bmi270->getAcceleration(&accX, &accY, &accZ);
  bmi270->getGyroscope(&gyrX, &gyrY, &gyrZ);
  bmm150->getMagnetometer(&magX, &magY, &magZ);
  
  Serial.print("RAW_IMU,");
  Serial.print(accX, 4); Serial.print(",");
  Serial.print(accY, 4); Serial.print(",");
  Serial.print(accZ, 4); Serial.print(",");
  Serial.print(gyrX, 4); Serial.print(",");
  Serial.print(gyrY, 4); Serial.print(",");
  Serial.print(gyrZ, 4); Serial.print(",");
  Serial.print(magX, 4); Serial.print(",");
  Serial.print(magY, 4); Serial.print(",");
  Serial.println(magZ, 4);
}

void RawDataDisplay::printRawGPSData(AtomicBaseGPS* gps) {
  // Print raw GPS data to serial in CSV format for logging
  Serial.print("RAW_GPS,");
  Serial.print(gps->getLatitude(), 6); Serial.print(",");
  Serial.print(gps->getLongitude(), 6); Serial.print(",");
  Serial.print(gps->getAltitude(), 2); Serial.print(",");
  Serial.print(gps->getSatellites()); Serial.print(",");
  Serial.print(gps->getHDOP(), 2); Serial.print(",");
  Serial.print(gps->getSpeed(), 2); Serial.print(",");
  Serial.println(gps->getCourse(), 2);
}

void RawDataDisplay::printRawCelestialData(float sunAz, float sunAlt, 
                                         float moonAz, float moonAlt, float moonPhase,
                                         float polarisAz, float polarisAlt) {
  // Print raw celestial data to serial in CSV format for logging
  Serial.print("RAW_CELESTIAL,");
  Serial.print(sunAz, 2); Serial.print(",");
  Serial.print(sunAlt, 2); Serial.print(",");
  Serial.print(moonAz, 2); Serial.print(",");
  Serial.print(moonAlt, 2); Serial.print(",");
  Serial.print(moonPhase, 4); Serial.print(",");
  Serial.print(polarisAz, 2); Serial.print(",");
  Serial.println(polarisAlt, 2);
}

void RawDataDisplay::printSystemInfo(float batteryLevel, float temperature, 
                                   unsigned long uptime, int freeMemory) {
  // Print system information to serial in CSV format for logging
  Serial.print("RAW_SYSTEM,");
  Serial.print(batteryLevel, 2); Serial.print(",");
  Serial.print(temperature, 2); Serial.print(",");
  Serial.print(uptime); Serial.print(",");
  Serial.println(freeMemory);
}
