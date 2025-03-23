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

// Constructor
CompassDisplay::CompassDisplay() {
  _currentColor = COLOR_BLACK;
  _lastAnimationTime = 0;
}

// Initialize display
void CompassDisplay::begin() {
  // Initialize display
  M5.dis.begin();
  M5.dis.clear();
  
  // Initialize celestial overlay
  _celestialOverlay.begin();
  
  // Show welcome animation
  showWelcome();
}

// Update display based on current mode
void CompassDisplay::update(DisplayMode mode, bool gpsValid, bool imuCalibrated) {
  // Update display based on current mode
  switch (mode) {
    case POLAR_ALIGNMENT:
      // Basic status indicator for now
      if (gpsValid && imuCalibrated) {
        // Green LED for ready state
        setPixelColor(COLOR_GREEN);
      } else if (!gpsValid && imuCalibrated) {
        // Yellow LED for IMU ready but no GPS
        setPixelColor(COLOR_YELLOW);
      } else if (gpsValid && !imuCalibrated) {
        // Blue LED for GPS ready but IMU not calibrated
        setPixelColor(COLOR_BLUE);
      } else {
        // Red LED for neither ready
        setPixelColor(COLOR_RED);
      }
      break;
      
    case GPS_DATA:
      // Basic status indicator for GPS data
      if (gpsValid) {
        // Green LED for valid GPS
        setPixelColor(COLOR_GREEN);
      } else {
        // Red LED for invalid GPS
        setPixelColor(COLOR_RED);
      }
      break;
      
    case IMU_DATA:
      // Basic status indicator for IMU data
      if (imuCalibrated) {
        // Green LED for calibrated IMU
        setPixelColor(COLOR_GREEN);
      } else {
        // Yellow LED for uncalibrated IMU
        setPixelColor(COLOR_YELLOW);
      }
      break;
      
    case CELESTIAL_DATA:
      // Cyan for celestial data
      setPixelColor(COLOR_CYAN);
      break;
      
    case CALIBRATION:
      // Blue LED during calibration
      setPixelColor(COLOR_BLUE);
      break;
      
    case SETTINGS:
      // Purple LED for settings
      setPixelColor(COLOR_PURPLE);
      break;
  }
}

// Display polar alignment compass
void CompassDisplay::showPolarAlignment(float heading, float polarisAz, float polarisAlt, 
                                      float pitch, float roll) {
  // Calculate the angular difference between current heading and Polaris azimuth
  float azDiff = polarisAz - heading;
  
  // Normalize to -180 to 180 degrees
  while (azDiff > 180) azDiff -= 360;
  while (azDiff < -180) azDiff += 360;
  
  // Calculate the absolute difference
  float absDiff = fabs(azDiff);
  
  // Get color based on alignment accuracy
  uint32_t alignColor = getAlignmentColor(absDiff);
  
  // If we're close to alignment, blink faster for better feedback
  if (absDiff < 5.0) {
    // Very close alignment - fast pulse
    pulsePixel(alignColor, 500);
  } else if (absDiff < 15.0) {
    // Close alignment - slow pulse
    pulsePixel(alignColor, 1000);
  } else {
    // Far from alignment - rotate in the direction to turn
    rotatePixel(alignColor, 1500, azDiff > 0 ? 1 : -1);
  }
  
  // Debug output
  Serial.print("Alignment: Heading=");
  Serial.print(heading);
  Serial.print(", PolarisAz=");
  Serial.print(polarisAz);
  Serial.print(", Diff=");
  Serial.print(azDiff);
  Serial.print(", Alt=");
  Serial.println(polarisAlt);
}

// Display GPS data
void CompassDisplay::showGPSData(float latitude, float longitude, float altitude,
                               int satellites, float hdop, int hour, int minute) {
  // For now, just blink based on satellite count
  if (satellites >= 5) {
    // Good satellite count - solid green
    setPixelColor(COLOR_GREEN);
  } else if (satellites >= 3) {
    // Marginal satellite count - yellow
    setPixelColor(COLOR_YELLOW);
  } else {
    // Poor satellite count - red
    setPixelColor(COLOR_RED);
  }
  
  // Debug output
  Serial.print("GPS: Lat=");
  Serial.print(latitude, 6);
  Serial.print(", Lon=");
  Serial.print(longitude, 6);
  Serial.print(", Alt=");
  Serial.print(altitude);
  Serial.print(", Sats=");
  Serial.print(satellites);
  Serial.print(", HDOP=");
  Serial.print(hdop);
  Serial.print(", Time=");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
}

// Display IMU data
void CompassDisplay::showIMUData(float heading, float pitch, float roll) {
  // For now, just use color to indicate heading
  // Map heading (0-360) to hue (0-255)
  uint8_t hue = map(int(heading), 0, 360, 0, 255);
  
  // Create color from HSV
  uint32_t color = 0;
  uint8_t r, g, b;
  
  // Simple HSV to RGB conversion
  if (hue < 85) {
    r = 255 - hue * 3;
    g = hue * 3;
    b = 0;
  } else if (hue < 170) {
    hue -= 85;
    r = 0;
    g = 255 - hue * 3;
    b = hue * 3;
  } else {
    hue -= 170;
    r = hue * 3;
    g = 0;
    b = 255 - hue * 3;
  }
  
  color = (r << 16) | (g << 8) | b;
  setPixelColor(color);
  
  // Debug output
  Serial.print("IMU: Heading=");
  Serial.print(heading);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
}

// Display celestial data
void CompassDisplay::showCelestialData(float sunAz, float sunAlt, float moonAz, float moonAlt, float moonPhase) {
  // For now, just use color to indicate if sun/moon is visible
  if (sunAlt > 0) {
    // Sun is above horizon - yellow
    setPixelColor(COLOR_YELLOW);
  } else if (moonAlt > 0) {
    // Moon is above horizon - blue/white based on phase
    uint8_t brightness = 128 + moonPhase * 127;
    uint32_t color = (brightness << 16) | (brightness << 8) | 255;
    setPixelColor(color);
  } else {
    // Neither visible - dark blue for night
    setPixelColor(0x000080);
  }
  
  // Debug output
  Serial.print("Celestial: SunAz=");
  Serial.print(sunAz);
  Serial.print(", SunAlt=");
  Serial.print(sunAlt);
  Serial.print(", MoonAz=");
  Serial.print(moonAz);
  Serial.print(", MoonAlt=");
  Serial.print(moonAlt);
  Serial.print(", MoonPhase=");
  Serial.println(moonPhase);
}

// Enhanced celestial display with overlay
void CompassDisplay::showCelestialOverlay(float heading, float latitude, float longitude,
                                        int year, int month, int day, int hour, int minute, int second) {
  // Update celestial overlay data
  updateCelestialOverlay(latitude, longitude, year, month, day, hour, minute, second);
  
  // Get celestial positions
  float sunAz, sunAlt, moonAz, moonAlt, polarisAz, polarisAlt;
  _celestialOverlay.getSunPosition(&sunAz, &sunAlt);
  _celestialOverlay.getMoonPosition(&moonAz, &moonAlt);
  _celestialOverlay.getPolarisPosition(&polarisAz, &polarisAlt);
  
  // Get moon phase
  float moonPhase = _celestialOverlay.getMoonPhase();
  MoonPhase moonPhaseEnum = _celestialOverlay.getMoonPhaseEnum();
  int moonIllumination = _celestialOverlay.getMoonIllumination();
  
  // Calculate relative positions to current heading
  float relSunAz = sunAz - heading;
  float relMoonAz = moonAz - heading;
  float relPolarisAz = polarisAz - heading;
  
  // Normalize to -180 to 180 degrees
  while (relSunAz > 180) relSunAz -= 360;
  while (relSunAz < -180) relSunAz += 360;
  
  while (relMoonAz > 180) relMoonAz -= 360;
  while (relMoonAz < -180) relMoonAz += 360;
  
  while (relPolarisAz > 180) relPolarisAz -= 360;
  while (relPolarisAz < -180) relPolarisAz += 360;
  
  // Determine which celestial object to highlight based on visibility and importance
  if (polarisAlt > 0) {
    // Polaris is visible - primary focus for polar alignment
    uint32_t polarisColor = getAlignmentColor(fabs(relPolarisAz));
    pulsePixel(polarisColor, 1000);
  } else if (sunAlt > 0) {
    // Sun is visible - show sun position
    // Map relative azimuth (-180 to 180) to hue (0 to 255)
    uint8_t hue = map(int(relSunAz + 180), 0, 360, 0, 255);
    
    // Create color from HSV with yellow base
    uint32_t sunColor = 0xFFFF00; // Yellow
    pulsePixel(sunColor, 1000);
  } else if (moonAlt > 0) {
    // Moon is visible - show moon position and phase
    // Map relative azimuth (-180 to 180) to hue (0 to 255)
    uint8_t hue = map(int(relMoonAz + 180), 0, 360, 0, 255);
    
    // Create color based on moon phase
    uint32_t moonColor;
    switch (moonPhaseEnum) {
      case NEW_MOON:
        moonColor = 0x202020; // Very dark
        break;
      case WAXING_CRESCENT:
      case WANING_CRESCENT:
        moonColor = 0x404080; // Dark blue
        break;
      case FIRST_QUARTER:
      case LAST_QUARTER:
        moonColor = 0x8080A0; // Medium blue
        break;
      case WAXING_GIBBOUS:
      case WANING_GIBBOUS:
        moonColor = 0xA0A0C0; // Light blue
        break;
      case FULL_MOON:
        moonColor = 0xE0E0FF; // Bright blue-white
        break;
    }
    
    pulsePixel(moonColor, 1000);
  } else {
    // Nothing visible - show night sky
    setPixelColor(0x000040); // Dark blue
  }
  
  // Debug output
  Serial.println("Celestial Overlay:");
  _celestialOverlay.printCelestialData();
  Serial.print("Current Heading: ");
  Serial.println(heading);
}

// Celestial overlay helper
void CompassDisplay::updateCelestialOverlay(float latitude, float longitude,
                                          int year, int month, int day, int hour, int minute, int second) {
  // Update celestial overlay with current location and time
  _celestialOverlay.updateCelestialData(latitude, longitude, year, month, day, hour, minute, second);
}

// Display settings
void CompassDisplay::showSettings() {
  // Purple for settings
  setPixelColor(COLOR_PURPLE);
}

// Display calibration
void CompassDisplay::showCalibration(int stage, float progress) {
  // Pulse blue during calibration
  uint8_t brightness = 50 + progress * 205;
  uint32_t color = 0x0000FF * brightness / 255;
  setPixelColor(color);
  
  // Debug output
  Serial.print("Calibration: Stage=");
  Serial.print(stage);
  Serial.print(", Progress=");
  Serial.println(progress);
}

// Show welcome screen
void CompassDisplay::showWelcome() {
  // Rainbow animation for welcome
  for (int i = 0; i < 256; i += 8) {
    uint8_t hue = i;
    uint8_t r, g, b;
    
    // Simple HSV to RGB conversion
    if (hue < 85) {
      r = 255 - hue * 3;
      g = hue * 3;
      b = 0;
    } else if (hue < 170) {
      hue -= 85;
      r = 0;
      g = 255 - hue * 3;
      b = hue * 3;
    } else {
      hue -= 170;
      r = hue * 3;
      g = 0;
      b = 255 - hue * 3;
    }
    
    uint32_t color = (r << 16) | (g << 8) | b;
    setPixelColor(color);
    delay(20);
  }
  
  // Blink white a few times
  blinkPixel(COLOR_WHITE, COLOR_BLACK, 3, 200);
  
  // Set to default color
  setPixelColor(COLOR_BLUE);
}

// Show error message
void CompassDisplay::showError(const char* message) {
  // Blink red for error
  blinkPixel(COLOR_RED, COLOR_BLACK, 5, 100);
  
  // Debug output
  Serial.print("ERROR: ");
  Serial.println(message);
}

// Helper methods
uint32_t CompassDisplay::getAlignmentColor(float angleDiff) {
  // Return color based on alignment accuracy
  if (angleDiff < 1.0) {
    return COLOR_GREEN;  // Very accurate
  } else if (angleDiff < 5.0) {
    return 0x80FF00;     // Good
  } else if (angleDiff < 10.0) {
    return COLOR_YELLOW; // Acceptable
  } else if (angleDiff < 20.0) {
    return 0xFF8000;     // Poor
  } else {
    return COLOR_RED;    // Bad
  }
}

void CompassDisplay::setPixelColor(uint32_t color) {
  M5.dis.drawpix(0, color);
  _currentColor = color;
}

void CompassDisplay::blinkPixel(uint32_t color1, uint32_t color2, int count, int delayMs) {
  for (int i = 0; i < count; i++) {
    setPixelColor(color1);
    delay(delayMs);
    setPixelColor(color2);
    delay(delayMs);
  }
}

void CompassDisplay::pulsePixel(uint32_t color, int duration) {
  unsigned long currentTime = millis();
  
  // Only update animation at certain intervals
  if (currentTime - _lastAnimationTime > 50) {
    _lastAnimationTime = currentTime;
    
    // Calculate brightness based on sine wave
    float phase = (currentTime % duration) / (float)duration;
    float brightness = (sin(phase * 2 * PI) + 1) / 2.0;
    
    // Extract RGB components
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    // Apply brightness
    r = r * brightness;
    g = g * brightness;
    b = b * brightness;
    
    // Set color
    uint32_t adjustedColor = (r << 16) | (g << 8) | b;
    setPixelColor(adjustedColor);
  }
}

void CompassDisplay::rotatePixel(uint32_t color, int duration, int direction) {
  unsigned long currentTime = millis();
  
  // Only update animation at certain intervals
  if (currentTime - _lastAnimationTime > 50) {
    _lastAnimationTime = currentTime;
    
    // Calculate position in animation
    float phase = (currentTime % duration) / (float)duration;
    
    // If direction is negative, reverse the phase
    if (direction < 0) {
      phase = 1.0 - phase;
    }
    
    // Simple on/off based on phase
    if (phase < 0.5) {
      setPixelColor(color);
    } else {
      setPixelColor(COLOR_BLACK);
    }
  }
}
