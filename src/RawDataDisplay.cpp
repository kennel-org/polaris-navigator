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

// Set pixel color (for RGB LED)
void RawDataDisplay::setPixelColor(uint32_t color) {
  // Debug output
  Serial.print("LED color set to 0x");
  Serial.println(color, HEX);
  
  // M5UnifiedでのLED制御
  // RGB値に分解
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  // RGB565形式に変換
  uint16_t rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  
  // AtomS3のLEDを制御
  if (M5.getBoard() == m5::board_t::board_M5AtomS3) {
    // M5.Displayを使用して小さな円を描画（LED表現）
    M5.Display.fillCircle(5, 5, 5, rgb565);
  }
}

// Update display with raw data
void RawDataDisplay::update(RawDataMode mode) {
  // Save the current mode
  _currentMode = mode;
  
  // Get current time
  unsigned long currentTime = millis();
  
  // Limit update frequency (to prevent flickering)
  if (currentTime - _lastUpdateTime < 1000) {
    return; // Skip update if less than 1 second has passed
  }
  
  // Update last update time
  _lastUpdateTime = currentTime;
  
  // Display based on the current mode
  switch (mode) {
    case RAW_IMU:
      // IMU data display (actual data is set in a separate function)
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(10, 10);
      M5.Display.println("IMU Raw Data Mode");
      // Actual data display is handled by showRawIMU function
      break;
      
    case RAW_GPS:
      // GPS data display (actual data is set in a separate function)
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(10, 10);
      M5.Display.println("GPS Raw Data Mode");
      // Actual data display is handled by showRawGPS function
      break;
      
    case RAW_CELESTIAL:
      // Celestial data display (actual data is set in a separate function)
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Celestial Raw Data Mode");
      // Actual data display is handled by showRawCelestial function
      break;
      
    case RAW_SYSTEM:
      // System information display (actual data is set in a separate function)
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(10, 10);
      M5.Display.println("System Information");
      // Actual data display is handled by showSystemInfo function
      break;
      
    case DISPLAY_DEBUG:  // Added debug mode display
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(0xFD20);  // Orange color
      M5.Display.setTextSize(1);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Debug Mode");
      break;
      
    default:
      // Unknown mode
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Unknown Mode");
      break;
  }
  
  // Update LED color based on mode
  switch (mode) {
    case RAW_IMU:
      setPixelColor(0x0000FF); // Blue
      break;
    case RAW_GPS:
      setPixelColor(0x00FF00); // Green
      break;
    case RAW_CELESTIAL:
      setPixelColor(0xFF00FF); // Purple
      break;
    case RAW_SYSTEM:
      setPixelColor(0xFFFF00); // Yellow
      break;
    case DISPLAY_DEBUG:
      setPixelColor(0xFF8000); // Orange
      break;
    default:
      setPixelColor(0xFF0000); // Red
      break;
  }
}
