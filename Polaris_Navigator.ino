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

// IMU related
#include "BMM150class.h"     // Magnetometer
#include "BMI270.h"          // Accelerometer and Gyroscope
#include "IMUFusion.h"       // Sensor fusion

// GPS related
#include "AtomicBaseGPS.h"   // AtomicBase GPS module

// Display related
#include <FastLED.h>         // For LED control
#include "CompassDisplay.h"  // Custom display interface

// Celestial calculations
#include "celestial_math.h"  // Custom celestial calculations

// Constants
#define GPS_BAUD 9600        // GPS baud rate
#define SERIAL_BAUD 115200   // Serial monitor baud rate
#define UPDATE_INTERVAL 50   // Main loop update interval in ms

// Global variables
AtomicBaseGPS gps;           // GPS object
BMI270 bmi270;               // IMU object
BMM150class bmm150;          // Magnetometer object
IMUFusion imuFusion(&bmi270, &bmm150); // Sensor fusion
CompassDisplay display;      // Display object

// GPS data
float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;
int satellites = 0;
bool gpsValid = false;
float hdop = 99.99;          // Horizontal dilution of precision

// Time data
int year = 2025;
int month = 3;
int day = 23;
int hour = 0;
int minute = 0;
int second = 0;
bool timeValid = false;

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
float magDeclination = 0.0;  // Magnetic declination

// UI state
enum DisplayMode {
  POLAR_ALIGNMENT,
  GPS_DATA,
  IMU_DATA,
  SETTINGS,
  CALIBRATION,
  CELESTIAL_DATA
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
void calibrateIMU();

// Timing variables
unsigned long lastUpdateTime = 0;
unsigned long lastDisplayTime = 0;

void setup() {
  // Initialize hardware
  setupHardware();
  
  // Initialize sensors
  setupIMU();
  setupGPS();
  
  // Initialize display
  display.begin();
  
  // Initialize timing
  lastUpdateTime = millis();
}

void loop() {
  // Calculate delta time for sensor fusion
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0f;  // Convert to seconds
  lastUpdateTime = currentTime;
  
  // Update button state
  M5.update();
  
  // Handle button presses
  handleButtons();
  
  // Read sensor data and update fusion
  readGPS();
  readIMU();
  
  // Update sensor fusion
  imuFusion.update(deltaTime);
  
  // Calculate celestial positions
  if (gpsValid) {
    calculateCelestialPositions();
  }
  
  // Update display at specified interval
  if (currentTime - lastDisplayTime >= UPDATE_INTERVAL) {
    lastDisplayTime = currentTime;
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
  
  // Initialize sensor fusion
  imuFusion.begin();
  
  // Check if already calibrated
  imuCalibrated = imuFusion.isCalibrated();
  
  Serial.println("IMU initialized");
}

void setupGPS() {
  // Initialize GPS module
  if (gps.begin(GPS_BAUD)) {
    Serial.println("GPS initialized");
  } else {
    Serial.println("Failed to initialize GPS!");
  }
}

void readGPS() {
  // Update GPS data
  gps.update();
  
  // Check if GPS data is valid
  gpsValid = gps.isValid();
  
  if (gpsValid) {
    // Update GPS variables
    latitude = gps.getLatitude();
    longitude = gps.getLongitude();
    altitude = gps.getAltitude();
    satellites = gps.getSatellites();
    hdop = gps.getHDOP();
    
    // Update time from GPS if available
    if (gps.getTime(&hour, &minute, &second) && 
        gps.getDate(&year, &month, &day)) {
      timeValid = true;
      
      // Debug time output
      Serial.print("Date/Time: ");
      Serial.print(year);
      Serial.print("-");
      Serial.print(month);
      Serial.print("-");
      Serial.print(day);
      Serial.print(" ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.println(second);
    }
    
    // Debug output
    Serial.print("GPS: ");
    Serial.print(latitude, 6);
    Serial.print(", ");
    Serial.print(longitude, 6);
    Serial.print(" Alt: ");
    Serial.print(altitude);
    Serial.print("m Sats: ");
    Serial.print(satellites);
    Serial.print(" HDOP: ");
    Serial.println(hdop);
  }
}

void readIMU() {
  // Get orientation from sensor fusion
  heading = imuFusion.getYaw();
  pitch = imuFusion.getPitch();
  roll = imuFusion.getRoll();
  
  // Debug output
  Serial.print("Orientation: Heading=");
  Serial.print(heading);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
}

void calculateCelestialPositions() {
  // Calculate celestial positions based on GPS location and current time
  if (!gpsValid) {
    return;  // Need valid GPS data for calculations
  }
  
  // Calculate magnetic declination for current location
  magDeclination = calculateMagneticDeclination(latitude, longitude);
  
  // Apply magnetic declination to heading
  float trueHeading = heading;
  applyMagneticDeclination(&trueHeading, magDeclination);
  
  // Calculate celestial pole position
  calculatePolePosition(latitude, longitude, &polarisAz, &polarisAlt);
  
  // Calculate sun position if time is valid
  if (timeValid) {
    calculateSunPosition(latitude, longitude, year, month, day, hour, minute, second, &sunAz, &sunAlt);
    
    // Calculate moon position
    calculateMoonPosition(latitude, longitude, year, month, day, hour, minute, second, &moonAz, &moonAlt, &moonPhase);
  }
  
  // Debug output
  Serial.print("Celestial Calculations - Magnetic Declination: ");
  Serial.print(magDeclination);
  Serial.print("° | Polaris/Pole: Az=");
  Serial.print(polarisAz);
  Serial.print("° Alt=");
  Serial.print(polarisAlt);
  Serial.print("° | True Heading: ");
  Serial.println(trueHeading);
  
  if (timeValid) {
    Serial.print("Sun: Az=");
    Serial.print(sunAz);
    Serial.print("° Alt=");
    Serial.print(sunAlt);
    Serial.print("° | Moon: Az=");
    Serial.print(moonAz);
    Serial.print("° Alt=");
    Serial.print(moonAlt);
    Serial.print("° Phase=");
    Serial.println(moonPhase);
  }
}

void updateDisplay() {
  // Update display based on current mode
  display.update(currentMode, gpsValid, imuCalibrated);
  
  // Display specific information based on current mode
  switch (currentMode) {
    case POLAR_ALIGNMENT:
      // Display polar alignment information
      if (gpsValid && imuCalibrated) {
        // Show polar alignment compass
        display.showPolarAlignment(heading, polarisAz, polarisAlt, pitch, roll);
      }
      break;
      
    case GPS_DATA:
      // Display raw GPS data
      if (gpsValid) {
        display.showGPSData(latitude, longitude, altitude, satellites, hdop, hour, minute);
      }
      break;
      
    case IMU_DATA:
      // Display raw IMU data
      display.showIMUData(heading, pitch, roll);
      break;
      
    case CELESTIAL_DATA:
      // Display celestial data
      display.showCelestialData(sunAz, sunAlt, moonAz, moonAlt, moonPhase);
      break;
      
    case CALIBRATION:
      // Display calibration status
      display.showCalibration(0, 0.5); // Placeholder values
      break;
      
    case SETTINGS:
      // Display settings menu
      display.showSettings();
      break;
  }
}

void handleButtons() {
  // Handle button presses
  if (M5.Btn.wasPressed()) {
    // Cycle through display modes
    switch (currentMode) {
      case POLAR_ALIGNMENT:
        currentMode = GPS_DATA;
        break;
      case GPS_DATA:
        currentMode = IMU_DATA;
        break;
      case IMU_DATA:
        currentMode = CELESTIAL_DATA;
        break;
      case CELESTIAL_DATA:
        currentMode = SETTINGS;
        break;
      case SETTINGS:
        currentMode = POLAR_ALIGNMENT;
        break;
      case CALIBRATION:
        // Exit calibration mode
        currentMode = POLAR_ALIGNMENT;
        break;
    }
    
    Serial.print("Mode changed to: ");
    Serial.println(currentMode);
  }
  
  // Handle long press for calibration
  if (M5.Btn.pressedFor(2000) && currentMode != CALIBRATION) {
    currentMode = CALIBRATION;
    Serial.println("Entering calibration mode");
    calibrateIMU();
  }
}

void calibrateIMU() {
  // Perform IMU calibration
  Serial.println("Starting IMU calibration...");
  
  // Update display to show calibration mode
  display.showCalibration(0, 0.0);
  
  // Calibration steps
  for (int stage = 0; stage < 3; stage++) {
    Serial.print("Calibration stage ");
    Serial.print(stage + 1);
    Serial.println(" of 3...");
    
    // Show current stage
    for (float progress = 0.0; progress < 1.0; progress += 0.1) {
      display.showCalibration(stage, progress);
      delay(200);
    }
  }
  
  // Calibrate magnetometer
  imuFusion.calibrateMagnetometer();
  
  // Update calibration status
  imuCalibrated = imuFusion.isCalibrated();
  
  // Show completion
  display.showCalibration(3, 1.0);
  delay(1000);
  
  Serial.println("IMU calibration complete");
}
