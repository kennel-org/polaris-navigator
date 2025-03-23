/*
 * Polaris Navigator
 * 
 * A polar alignment assistant device for astrophotography
 * using AtomS3R (with IMU) and AtomicBase GPS.
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

// Include necessary libraries
#include <M5AtomS3.h>        // For AtomS3R
#include <Wire.h>            // I2C communication
#include <TinyGPS++.h>       // GPS parsing

// IMU related
#include "BMM150class.h"     // Magnetometer
#include "BMI270.h"          // Accelerometer and Gyroscope

// Display related
#include <FastLED.h>         // For LED control

// Celestial calculations
#include "celestial_math.h"  // Custom celestial calculations

// Constants
#define GPS_BAUD 9600        // GPS baud rate
#define SERIAL_BAUD 115200   // Serial monitor baud rate
#define UPDATE_INTERVAL 500  // Main loop update interval in ms

// Global variables
TinyGPSPlus gps;             // GPS object
BMI270 bmi270;               // IMU object
BMM150class bmm150;          // Magnetometer object

// GPS data
float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;
int satellites = 0;
bool gpsValid = false;

// IMU data
float heading = 0.0;         // Compass heading in degrees
float pitch = 0.0;           // Pitch angle in degrees
float roll = 0.0;            // Roll angle in degrees
bool imuCalibrated = false;

// Celestial data
float polarisAlt = 0.0;      // Altitude of Polaris/celestial pole
float polarisAz = 0.0;       // Azimuth of Polaris/celestial pole
float sunAz = 0.0;           // Sun azimuth
float sunAlt = 0.0;          // Sun altitude
float moonAz = 0.0;          // Moon azimuth
float moonAlt = 0.0;         // Moon altitude
float moonPhase = 0.0;       // Moon phase (0-1)

// UI state
enum DisplayMode {
  POLAR_ALIGNMENT,
  GPS_DATA,
  IMU_DATA,
  SETTINGS
};

DisplayMode currentMode = POLAR_ALIGNMENT;

// Function prototypes
void setupHardware();
void setupIMU();
void setupGPS();
void readGPS();
void readIMU();
void calculateCelestialPositions();
void updateDisplay();
void handleButtons();

void setup() {
  // Initialize hardware
  setupHardware();
  
  // Initialize sensors
  setupIMU();
  setupGPS();
  
  // Show welcome screen
  M5.dis.drawpix(0, 0x00ff00);  // Green LED to indicate ready
}

void loop() {
  static unsigned long lastUpdate = 0;
  
  // Update button state
  M5.update();
  
  // Handle button presses
  handleButtons();
  
  // Update at specified interval
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    
    // Read sensor data
    readGPS();
    readIMU();
    
    // Calculate celestial positions
    if (gpsValid) {
      calculateCelestialPositions();
    }
    
    // Update display based on current mode
    updateDisplay();
  }
}

void setupHardware() {
  // Initialize AtomS3R
  M5.begin(true, true, false);  // Enable display, enable power, disable serial
  Serial.begin(SERIAL_BAUD);    // Start serial for debugging
  
  Serial.println("Polaris Navigator initializing...");
}

void setupIMU() {
  // Initialize IMU sensors
  Wire.begin(38, 39);  // SDA, SCL pins for AtomS3R
  
  // Initialize BMI270
  if (bmi270.begin() != BMI270_OK) {
    Serial.println("Failed to initialize BMI270!");
  }
  
  // Initialize BMM150
  if (bmm150.initialize() != BMM150_OK) {
    Serial.println("Failed to initialize BMM150!");
  }
  
  Serial.println("IMU initialized");
}

void setupGPS() {
  // Initialize GPS on Serial2
  Serial2.begin(GPS_BAUD);
  Serial.println("GPS initialized");
}

void readGPS() {
  // Read data from GPS
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      // Update GPS data if valid
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        gpsValid = true;
      }
      
      if (gps.altitude.isValid()) {
        altitude = gps.altitude.meters();
      }
      
      if (gps.satellites.isValid()) {
        satellites = gps.satellites.value();
      }
    }
  }
}

void readIMU() {
  // Read IMU data
  bmi270.readAcceleration();
  bmi270.readGyro();
  bmm150.readMagnetometer();
  
  // Calculate orientation
  // TODO: Implement sensor fusion algorithm
  heading = 0.0;  // Placeholder
  pitch = 0.0;    // Placeholder
  roll = 0.0;     // Placeholder
}

void calculateCelestialPositions() {
  // TODO: Implement celestial calculations
  // This will be implemented in celestial_math.h/.cpp
}

void updateDisplay() {
  // Update display based on current mode
  switch (currentMode) {
    case POLAR_ALIGNMENT:
      // TODO: Display polar alignment information
      break;
      
    case GPS_DATA:
      // TODO: Display raw GPS data
      break;
      
    case IMU_DATA:
      // TODO: Display raw IMU data
      break;
      
    case SETTINGS:
      // TODO: Display settings menu
      break;
  }
}

void handleButtons() {
  // Handle button presses to change modes
  if (M5.Btn.wasPressed()) {
    // Cycle through display modes
    currentMode = (DisplayMode)((currentMode + 1) % 4);
    
    // Indicate mode change with LED color
    switch (currentMode) {
      case POLAR_ALIGNMENT:
        M5.dis.drawpix(0, 0x00ff00);  // Green
        break;
      case GPS_DATA:
        M5.dis.drawpix(0, 0x0000ff);  // Blue
        break;
      case IMU_DATA:
        M5.dis.drawpix(0, 0xff0000);  // Red
        break;
      case SETTINGS:
        M5.dis.drawpix(0, 0xffff00);  // Yellow
        break;
    }
  }
}
