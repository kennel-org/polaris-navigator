/*
 * CompassDisplay.cpp
 * 
 * Implementation for the Polaris Navigator display interface
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "CompassDisplay.h"
#include <math.h>
#include "AtomicBaseGPS.h" // Include for GPS settings

// Constructor
CompassDisplay::CompassDisplay() {
  _currentColor = COLOR_BLACK;
  _lastAnimationTime = 0;
}

// Initialize display
void CompassDisplay::begin() {
  // No need for individual initialization in M5Unified, 
  // as display initialization is done in M5.begin()
  
  // Set display settings
  M5.Display.setRotation(0); // Portrait orientation
  M5.Display.setTextSize(1); // Small font size
  
  // Initialize celestial overlay
  _celestialOverlay.begin();
  
  // Initialize canvas for double buffering
  _canvas.createSprite(M5.Display.width(), M5.Display.height());
  
  // Show welcome animation
  showWelcome();
}

// Set pixel color (for RGB LED)
void CompassDisplay::setPixelColor(uint32_t color) {
  // Save current color
  _currentColor = color;
  
  // Debug output
  Serial.print("LED color set to 0x");
  Serial.println(color, HEX);
  
  // Control LED using M5Unified
  // Extract RGB components
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  // Convert to RGB565 format
  uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  
  // Control AtomS3 LED
  if (M5.getBoard() == m5::board_t::board_M5AtomS3) {
    // Draw a small circle to represent LED
    M5.Display.fillCircle(5, 5, 5, rgb565);
  }
}

// Helper method: Blink the pixel between two colors
void CompassDisplay::blinkPixel(uint32_t color1, uint32_t color2, int count, int delayMs) {
  // Simplified implementation for now
  for (int i = 0; i < count; i++) {
    setPixelColor(color1);
    delay(delayMs);
    setPixelColor(color2);
    delay(delayMs);
  }
  // Return to the first color
  setPixelColor(color1);
}

// Show welcome screen
void CompassDisplay::showWelcome() {
  // Clear display
  M5.Display.fillScreen(TFT_NAVY);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(2);
  
  // Display title
  M5.Display.setCursor(10, 30);
  M5.Display.println("Polaris");
  M5.Display.setCursor(10, 50);
  M5.Display.println("Navigator");
  
  // Display version information
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 80);
  M5.Display.println("Version 1.0");
  
  // Display copyright information
  M5.Display.setCursor(10, 100);
  M5.Display.println("(c) 2025 Kennel.org");
  
  // Startup animation
  for (int i = 0; i < 100; i += 5) {
    M5.Display.drawRect(10, 120, 100, 10, TFT_WHITE);
    M5.Display.fillRect(10, 120, i, 10, TFT_GREEN);
    delay(50);
  }
  
  // Set LED to green
  setPixelColor(COLOR_GREEN);
  
  // Wait for a short time
  delay(1000);
}

// Display IMU data
void CompassDisplay::showIMU() {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("IMU Data");
  
  // Display gyroscope data
  M5.Display.setCursor(10, 30);
  M5.Display.println("Gyroscope:");
  M5.Display.setCursor(20, 40);
  M5.Display.printf("X: %6.2f deg/s", _imuData.gyroX);
  M5.Display.setCursor(20, 50);
  M5.Display.printf("Y: %6.2f deg/s", _imuData.gyroY);
  M5.Display.setCursor(20, 60);
  M5.Display.printf("Z: %6.2f deg/s", _imuData.gyroZ);
  
  // Display accelerometer data
  M5.Display.setCursor(10, 80);
  M5.Display.println("Accelerometer:");
  M5.Display.setCursor(20, 90);
  M5.Display.printf("X: %6.2f G", _imuData.accelX);
  M5.Display.setCursor(20, 100);
  M5.Display.printf("Y: %6.2f G", _imuData.accelY);
  M5.Display.setCursor(20, 110);
  M5.Display.printf("Z: %6.2f G", _imuData.accelZ);
  
  // Display magnetometer data
  M5.Display.setCursor(10, 130);
  M5.Display.println("Magnetometer:");
  M5.Display.setCursor(20, 140);
  M5.Display.printf("X: %6.2f uT", _imuData.magX);
  M5.Display.setCursor(20, 150);
  M5.Display.printf("Y: %6.2f uT", _imuData.magY);
  M5.Display.setCursor(20, 160);
  M5.Display.printf("Z: %6.2f uT", _imuData.magZ);
  
  // Set LED to blue
  setPixelColor(COLOR_BLUE);
}

// Display settings menu
void CompassDisplay::showSettings() {
  // Clear display
  M5.Display.fillScreen(TFT_NAVY);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("Settings Menu");
  
  // Display settings items
  M5.Display.setCursor(10, 30);
  M5.Display.println("1. Display Brightness");
  
  M5.Display.setCursor(10, 45);
  M5.Display.println("2. Calibration");
  
  M5.Display.setCursor(10, 60);
  M5.Display.println("3. GPS Settings");
  
  M5.Display.setCursor(10, 75);
  M5.Display.println("4. Units");
  
  M5.Display.setCursor(10, 90);
  M5.Display.println("5. Power Options");
  
  // Display operation instructions
  M5.Display.setCursor(10, 120);
  M5.Display.println("Press button to select");
  
  // Set LED to cyan
  setPixelColor(COLOR_CYAN);
}

// Display compass
void CompassDisplay::showCompass(float heading, float pitch, float roll, bool gpsValid, bool imuCalibrated) {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 5);
  M5.Display.println("Compass");
  
  // Display heading
  M5.Display.setCursor(10, 20);
  M5.Display.print("Heading: ");
  M5.Display.print(heading, 1);
  M5.Display.println(" deg");
  
  // Display pitch and roll
  M5.Display.setCursor(10, 32);
  M5.Display.print("Pitch: ");
  M5.Display.print(pitch, 1);
  M5.Display.println(" deg");
  
  M5.Display.setCursor(10, 44);
  M5.Display.print("Roll: ");
  M5.Display.print(roll, 1);
  M5.Display.println(" deg");
  
  // Display status indicators
  M5.Display.setCursor(10, 56);
  M5.Display.print("GPS: ");
  if (gpsValid) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("Valid");
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("Invalid");
  }
  
  M5.Display.setCursor(10, 68);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print("IMU: ");
  if (imuCalibrated) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("Calibrated");
  } else {
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.println("Needs Calibration");
  }
  
  // Draw compass rose
  int centerX = M5.Display.width() / 2;
  int centerY = 110; 
  int radius = 30; 
  
  // Draw compass circle
  M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // Draw cardinal directions
  float angle = (360 - heading) * PI / 180.0; 
  
  // North
  int nx = centerX + radius * sin(angle);
  int ny = centerY - radius * cos(angle);
  M5.Display.drawLine(centerX, centerY, nx, ny, TFT_RED);
  
  // Add cardinal direction labels
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // North label
  M5.Display.setCursor(centerX - 3, centerY - radius - 10);
  M5.Display.print("N");
  
  // East label
  M5.Display.setCursor(centerX + radius + 5, centerY - 3);
  M5.Display.print("E");
  
  // South label
  M5.Display.setCursor(centerX - 3, centerY + radius + 2);
  M5.Display.print("S");
  
  // West label
  M5.Display.setCursor(centerX - radius - 10, centerY - 3);
  M5.Display.print("W");
  
  // Set LED color based on GPS and IMU status
  if (!gpsValid) {
    setPixelColor(COLOR_RED); 
  } else if (!imuCalibrated) {
    setPixelColor(COLOR_YELLOW); 
  } else {
    setPixelColor(COLOR_GREEN); 
  }
}

// Display polar alignment compass
void CompassDisplay::showPolarAlignment(float heading, float polarisAz, float polarisAlt, float pitch, float roll) {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("Polar Alignment");
  
  // Display heading
  M5.Display.setCursor(10, 30);
  M5.Display.print("Heading: ");
  M5.Display.print(heading, 1);
  M5.Display.println(" deg");
  
  // Display Polaris position
  M5.Display.setCursor(10, 45);
  M5.Display.print("Polaris Az: ");
  M5.Display.print(polarisAz, 1);
  M5.Display.println(" deg");
  
  M5.Display.setCursor(10, 60);
  M5.Display.print("Polaris Alt: ");
  M5.Display.print(polarisAlt, 1);
  M5.Display.println(" deg");
  
  // Draw compass rose
  int centerX = M5.Display.width() / 2;
  int centerY = 120;
  int radius = 40;
  
  // Draw compass circle
  M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // Draw cardinal directions
  float angle = (360 - heading) * PI / 180.0; 
  
  // North
  int nx = centerX + radius * sin(angle);
  int ny = centerY - radius * cos(angle);
  M5.Display.drawLine(centerX, centerY, nx, ny, TFT_RED);
  
  // Draw Polaris position
  float polarisAngle = (polarisAz - heading) * PI / 180.0;
  int px = centerX + radius * sin(polarisAngle);
  int py = centerY - radius * cos(polarisAngle);
  
  // Adjust for altitude (simple projection)
  float altFactor = 1.0 - (polarisAlt / 90.0) * 0.5;
  px = centerX + (px - centerX) * altFactor;
  py = centerY + (py - centerY) * altFactor;
  
  // Draw Polaris marker
  M5.Display.fillCircle(px, py, 3, TFT_CYAN);
  
  // Set LED to purple
  setPixelColor(COLOR_PURPLE);
}

// Display celestial overlay
void CompassDisplay::showCelestialOverlay(float heading, float pitch, float roll, 
                                        float sunAz, float sunAlt, 
                                        float moonAz, float moonAlt, float moonPhase) {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("Celestial Overlay");
  
  // Display heading
  M5.Display.setCursor(10, 30);
  M5.Display.print("Heading: ");
  M5.Display.print(heading, 1);
  M5.Display.println(" deg");
  
  // Draw compass rose
  int centerX = M5.Display.width() / 2;
  int centerY = 120;
  int radius = 40;
  
  // Draw compass circle
  M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // Draw cardinal directions
  float angle = (360 - heading) * PI / 180.0; 
  
  // North
  int nx = centerX + radius * sin(angle);
  int ny = centerY - radius * cos(angle);
  M5.Display.drawLine(centerX, centerY, nx, ny, TFT_RED);
  
  // Draw Sun position
  float sunAngle = (sunAz - heading) * PI / 180.0;
  int sx = centerX + radius * sin(sunAngle);
  int sy = centerY - radius * cos(sunAngle);
  
  // Adjust for altitude (simple projection)
  float sunAltFactor = 1.0 - (sunAlt / 90.0) * 0.5;
  sx = centerX + (sx - centerX) * sunAltFactor;
  sy = centerY + (sy - centerY) * sunAltFactor;
  
  // Draw Sun marker
  uint16_t sunColor = (sunAlt < 0) ? TFT_DARKGREY : TFT_YELLOW;
  M5.Display.fillCircle(sx, sy, 5, sunColor);
  
  // Draw Moon position
  float moonAngle = (moonAz - heading) * PI / 180.0;
  int mx = centerX + radius * sin(moonAngle);
  int my = centerY - radius * cos(moonAngle);
  
  // Adjust for altitude (simple projection)
  float moonAltFactor = 1.0 - (moonAlt / 90.0) * 0.5;
  mx = centerX + (mx - centerX) * moonAltFactor;
  my = centerY + (my - centerY) * moonAltFactor;
  
  // Draw Moon marker
  uint16_t moonColor = (moonAlt < 0) ? TFT_DARKGREY : TFT_WHITE;
  M5.Display.fillCircle(mx, my, 4, moonColor);
  
  // Display celestial information
  M5.Display.setCursor(10, 170);
  M5.Display.print("Sun: Az=");
  M5.Display.print(sunAz, 1);
  M5.Display.print(" Alt=");
  M5.Display.print(sunAlt, 1);
  
  M5.Display.setCursor(10, 185);
  M5.Display.print("Moon: Az=");
  M5.Display.print(moonAz, 1);
  M5.Display.print(" Alt=");
  M5.Display.print(moonAlt, 1);
  
  // Display moon phase
  M5.Display.setCursor(10, 200);
  M5.Display.print("Moon Phase: ");
  M5.Display.print(moonPhase * 100.0, 0);
  M5.Display.println("%");
  
  // Set LED to purple
  setPixelColor(COLOR_PURPLE);
}

// Display GPS information
void CompassDisplay::showGPS(float latitude, float longitude, float altitude, int satellites, float hdop) {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("GPS Data");
  
  // Display GPS information
  M5.Display.setCursor(10, 30);
  M5.Display.print("Latitude: ");
  M5.Display.println(latitude, 6);
  
  M5.Display.setCursor(10, 45);
  M5.Display.print("Longitude: ");
  M5.Display.println(longitude, 6);
  
  M5.Display.setCursor(10, 60);
  M5.Display.print("Altitude: ");
  M5.Display.print(altitude);
  M5.Display.println("m");
  
  M5.Display.setCursor(10, 75);
  M5.Display.print("Satellites: ");
  M5.Display.println(satellites);
  
  M5.Display.setCursor(10, 90);
  M5.Display.print("HDOP: ");
  M5.Display.println(hdop);
  
  // Set LED to green
  setPixelColor(COLOR_GREEN);
}

// Display GPS invalid message
void CompassDisplay::showGPSInvalid() {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("GPS Status");
  
  // Display error message
  M5.Display.setCursor(10, 40);
  M5.Display.println("GPS Signal Invalid");
  
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(10, 60);
  M5.Display.println("Waiting for GPS fix...");
  
  M5.Display.setCursor(10, 80);
  M5.Display.println("Please ensure clear sky view");
  
  // Set LED to red
  setPixelColor(COLOR_RED);
}

// Display error message
void CompassDisplay::showError(const char* message) {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setCursor(10, 10);
  M5.Display.println("Error");
  
  // Display error message
  M5.Display.setCursor(10, 40);
  M5.Display.println(message);
  
  // Blink LED red
  blinkPixel(COLOR_RED, COLOR_BLACK, 3, 200);
}
