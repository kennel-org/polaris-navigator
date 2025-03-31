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
  
  // Note: Welcome screen is now handled by StartupScreen class
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
  M5.Display.setTextColor(TFT_MAGENTA);
  M5.Display.setCursor(2, 0);
  M5.Display.println("RAW IMU DATA:");
  M5.Display.setTextColor(TFT_WHITE);
  
  int y = 11;
  
  // Accelerometer data
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.println("Accelerometer (G):");
  M5.Display.setTextColor(TFT_WHITE);
  y += 9; 
  
  // Display X and Y on the same line
  M5.Display.setCursor(2, y);
  M5.Display.printf("X: %.3f  Y: %.3f", _imuData.accelX, _imuData.accelY);
  y += 9; 
  
  // Display Z on its own line
  M5.Display.setCursor(2, y);
  M5.Display.printf("Z: %.3f", _imuData.accelZ);
  y += 12; 
  
  // Gyroscope data
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.println("Gyroscope (deg/s):");
  M5.Display.setTextColor(TFT_WHITE);
  y += 9; 
  
  // Display X and Y on the same line
  M5.Display.setCursor(2, y);
  M5.Display.printf("X: %.3f  Y: %.3f", _imuData.gyroX, _imuData.gyroY);
  y += 9; 
  
  // Display Z on its own line
  M5.Display.setCursor(2, y);
  M5.Display.printf("Z: %.3f", _imuData.gyroZ);
  y += 12; 
  
  // Magnetometer data
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.println("Magnetometer (uT):");
  M5.Display.setTextColor(TFT_WHITE);
  y += 9; 
  
  // Display X and Y on the same line
  M5.Display.setCursor(2, y);
  M5.Display.printf("X: %.3f  Y: %.3f", _imuData.magX, _imuData.magY);
  y += 9; 
  
  // Display Z on its own line
  M5.Display.setCursor(2, y);
  M5.Display.printf("Z: %.3f", _imuData.magZ);
  
  // Set LED to blue
  setPixelColor(COLOR_BLUE);
}

// Display compass
void CompassDisplay::showCompass(float heading, float pitch, float roll, bool gpsValid, bool imuCalibrated) {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(2, 0);
  M5.Display.println("COMPASS DATA");
  M5.Display.setTextColor(TFT_WHITE);
  
  // Draw compass rose first (in the upper portion of the screen)
  int centerX = M5.Display.width() / 2;
  int centerY = 45; 
  int radius = 25; 
  
  // Draw compass circle
  M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // Draw cardinal directions
  // 方位角の計算を修正 - 北が上になるように調整
  // 方位角は時計回りで、北が0度、東が90度、南が180度、西が270度
  float angle = heading * PI / 180.0; 
  
  // North - 北を指す針（赤色）
  int nx = centerX + radius * sin(angle);
  int ny = centerY - radius * cos(angle);
  M5.Display.drawLine(centerX, centerY, nx, ny, TFT_RED);
  
  // Add cardinal direction labels
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // North label
  M5.Display.setCursor(centerX - 3, centerY - radius - 8);
  M5.Display.print("N");
  
  // East label
  M5.Display.setCursor(centerX + radius + 3, centerY - 3);
  M5.Display.print("E");
  
  // South label
  M5.Display.setCursor(centerX - 3, centerY + radius + 2);
  M5.Display.print("S");
  
  // West label
  M5.Display.setCursor(centerX - radius - 8, centerY - 3);
  M5.Display.print("W");
  
  // Display heading with larger text and centered
  M5.Display.setTextSize(2);
  char headingStr[8];
  sprintf(headingStr, "%03.1f", heading);
  int textWidth = strlen(headingStr) * 12; // Approximate width of text
  M5.Display.setCursor(centerX - textWidth/2, centerY - 8);
  M5.Display.print(headingStr);
  M5.Display.setTextSize(1);
  
  // Display numerical data below the compass
  int y = centerY + radius + 12;
  
  // Display heading, pitch and roll on separate lines
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Heading: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(heading, 1);
  M5.Display.println(" ");
  y += 9;
  
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Pitch: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(pitch, 1);
  M5.Display.println(" ");
  y += 9;
  
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Roll: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(roll, 1);
  M5.Display.println(" ");
  y += 12;
  
  // Display status indicators with colored icons
  M5.Display.setCursor(2, y);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print("GPS: ");
  if (gpsValid) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("OK");
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("NO");
  }
  y += 9;
  
  M5.Display.setCursor(2, y);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print("IMU: ");
  if (imuCalibrated) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("OK");
  } else {
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.println("CAL");
  }
  
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
  M5.Display.setTextColor(TFT_MAGENTA);
  M5.Display.setCursor(2, 0);
  M5.Display.println("POLAR ALIGNMENT");
  M5.Display.setTextColor(TFT_WHITE);
  
  // Draw compass rose in the upper portion of the screen
  int centerX = M5.Display.width() / 2;
  int centerY = 45; 
  int radius = 25; 
  
  // Draw compass circle
  M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // Draw cardinal directions
  // 方位角の計算を修正 - 北が上になるように調整
  // 方位角は時計回りで、北が0度、東が90度、南が180度、西が270度
  float angle = heading * PI / 180.0; 
  
  // North - 北を指す針（赤色）
  int nx = centerX + radius * sin(angle);
  int ny = centerY - radius * cos(angle);
  M5.Display.drawLine(centerX, centerY, nx, ny, TFT_RED);
  
  // Calculate Polaris position relative to heading
  // 北極星の方位角計算も同様に修正
  float polarisAngle = polarisAz * PI / 180.0;
  int px = centerX + (radius * 0.8) * sin(polarisAngle);
  int py = centerY - (radius * 0.8) * cos(polarisAngle);
  
  // Draw Polaris indicator (star symbol)
  M5.Display.fillCircle(px, py, 2, TFT_CYAN);
  M5.Display.drawLine(px-3, py, px+3, py, TFT_CYAN);
  M5.Display.drawLine(px, py-3, px, py+3, TFT_CYAN);
  
  // Add cardinal direction labels
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // North label
  M5.Display.setCursor(centerX - 3, centerY - radius - 8);
  M5.Display.print("N");
  
  // East label
  M5.Display.setCursor(centerX + radius + 3, centerY - 3);
  M5.Display.print("E");
  
  // South label
  M5.Display.setCursor(centerX - 3, centerY + radius + 2);
  M5.Display.print("S");
  
  // West label
  M5.Display.setCursor(centerX - radius - 8, centerY - 3);
  M5.Display.print("W");
  
  // Display numerical data below the compass
  int y = centerY + radius + 12;
  
  // Display heading
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Heading: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(heading, 1);
  M5.Display.println(" ");
  y += 9;
  
  // Display Polaris position
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Polaris Az: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(polarisAz, 1);
  M5.Display.println(" ");
  y += 9;
  
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Polaris Alt: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(polarisAlt, 1);
  M5.Display.println(" ");
  y += 9;
  
  // Display pitch and roll
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Pitch: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(pitch, 1);
  M5.Display.println(" ");
  y += 9;
  
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Roll: ");
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.print(roll, 1);
  M5.Display.println(" ");
  
  // 仰角のグラフィック表示を追加
  // 垂直表示に変更（左側に配置）
  
  // 仰角インジケーター
  int barX = 15; // 少し中央に寄せる
  int barWidth = 8;
  int barHeight = radius * 2; // コンパスの円の直径と同じ高さに設定
  int barY = centerY - radius; // コンパスの円の上端と同じ高さに設定
  
  // 仰角インジケーターのタイトル
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(barX, barY - 10);
  M5.Display.println("Alt");
  
  // 背景バー（グレー）
  M5.Display.fillRect(barX, barY, barWidth, barHeight, TFT_DARKGREY);
  
  // 中央マーカー（白）
  M5.Display.fillRect(barX - 2, barY + barHeight/2 - 1, barWidth + 4, 2, TFT_WHITE);
  
  // 目標高度マーカー（シアン）
  int targetPosY = barY + barHeight - (int)((polarisAlt / 90.0) * barHeight);
  targetPosY = constrain(targetPosY, barY + 2, barY + barHeight - 2);
  M5.Display.fillRect(barX - 2, targetPosY - 1, barWidth + 4, 2, TFT_CYAN);
  
  // 現在の傾きインジケーター（黄色）
  int currentPosY = barY + barHeight - (int)((pitch / 90.0) * barHeight);
  currentPosY = constrain(currentPosY, barY + 2, barY + barHeight - 2);
  M5.Display.fillTriangle(
    barX - 4, currentPosY,
    barX - 8, currentPosY - 4,
    barX - 8, currentPosY + 4,
    TFT_YELLOW
  );
  
  // 仰角の数値表示
  // T:（Target）- 現在のピッチ角からのずれを表示（Nと同じ高さ、右詰め）
  float deviation = polarisAlt - pitch;
  M5.Display.setTextColor(TFT_WHITE);
  // Nの位置を計算（コンパスの上端）
  int northY = centerY - radius - 8;
  M5.Display.setCursor(M5.Display.width() - 45, northY);
  M5.Display.print("T:");
  if (deviation > 0) M5.Display.print("+");
  M5.Display.print(deviation, 1);
  M5.Display.print(" "); // 単位を空白に変更
  
  // C:（Current）- 現在のピッチ角を表示（Sと同じ高さ、右詰め）
  M5.Display.setTextColor(TFT_YELLOW);
  // Sの位置を計算（コンパスの下端）
  int southY = centerY + radius + 2;
  M5.Display.setCursor(M5.Display.width() - 45, southY);
  M5.Display.print("C:");
  M5.Display.print(pitch, 1);
  M5.Display.print(" "); // 単位を空白に変更

  // Set LED to blue
  setPixelColor(COLOR_BLUE);
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
  M5.Display.setCursor(10, 0);
  M5.Display.println("Celestial Overlay");
  
  // Display heading
  M5.Display.setCursor(10, 30); // 20から30に変更（10ピクセル下げる）
  M5.Display.print("Heading: ");
  M5.Display.print(heading, 1);
  M5.Display.println(" ");
  
  // Draw compass rose
  int centerX = M5.Display.width() / 2;
  int centerY = 110;
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
  M5.Display.setCursor(10, 160);
  M5.Display.print("Sun: Az=");
  M5.Display.print(sunAz, 1);
  M5.Display.print(" Alt=");
  M5.Display.print(sunAlt, 1);
  
  M5.Display.setCursor(10, 175);
  M5.Display.print("Moon: Az=");
  M5.Display.print(moonAz, 1);
  M5.Display.print(" Alt=");
  M5.Display.print(moonAlt, 1);
  
  // Display moon phase
  M5.Display.setCursor(10, 190);
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
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(2, 0);
  M5.Display.println("RAW GPS DATA:");
  
  // Display GPS status in compact format
  M5.Display.fillRect(80, 0, 48, 10, TFT_BLACK);
  M5.Display.setCursor(80, 0);
  M5.Display.print("GPS:");
  
  // Determine GPS status
  int gpsStatus = 0;
  if (satellites == 0) {
    gpsStatus = 0; // No connection
  } else if (hdop > 5.0) {
    gpsStatus = 1; // Poor signal
  } else if (satellites < 4) {
    gpsStatus = 2; // Acquiring
  } else {
    gpsStatus = 3; // Good signal
  }
  
  // Display GPS status with color
  switch(gpsStatus) {
    case 0:
      M5.Display.setTextColor(TFT_RED);
      M5.Display.println("NO");
      break;
    case 1:
      M5.Display.setTextColor(TFT_YELLOW);
      M5.Display.println("NS");
      break;
    case 2:
      M5.Display.setTextColor(TFT_BLUE);
      M5.Display.println("AQ");
      break;
    case 3:
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.println("OK");
      break;
  }
  
  M5.Display.setTextColor(TFT_WHITE);
  int y = 15;
  
  // Satellites
  M5.Display.setCursor(2, y);
  M5.Display.print("Sats: ");
  M5.Display.print(satellites);
  y += 8;
  
  // Location
  M5.Display.setCursor(2, y);
  M5.Display.print("Lat: ");
  M5.Display.print(latitude, 6);
  y += 8;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Lng: ");
  M5.Display.print(longitude, 6);
  y += 8;
  
  // Altitude
  M5.Display.setCursor(2, y);
  M5.Display.print("Alt: ");
  M5.Display.print(altitude, 1);
  M5.Display.print("m");
  y += 8;
  
  // HDOP (Horizontal Dilution of Precision)
  M5.Display.setCursor(2, y);
  M5.Display.print("HDOP: ");
  M5.Display.print(hdop, 1);
  
  // Set LED color based on GPS status
  switch(gpsStatus) {
    case 0:
      setPixelColor(COLOR_RED);
      break;
    case 1:
      setPixelColor(COLOR_YELLOW);
      break;
    case 2:
      setPixelColor(COLOR_BLUE);
      break;
    case 3:
      setPixelColor(COLOR_GREEN);
      break;
  }
}

// Display GPS invalid message
void CompassDisplay::showGPSInvalid() {
  // Clear display
  M5.Display.fillScreen(TFT_BLACK);
  
  // Set text settings
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // Display title
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(2, 0);
  M5.Display.println("GPS STATUS");
  
  // Display GPS status in compact format
  M5.Display.fillRect(80, 0, 48, 10, TFT_BLACK);
  M5.Display.setCursor(80, 0);
  M5.Display.print("GPS:");
  M5.Display.setTextColor(TFT_RED);
  M5.Display.println("NO");
  
  M5.Display.setTextColor(TFT_WHITE);
  
  // Display error message with more compact layout
  int y = 20;
  M5.Display.setCursor(2, y);
  M5.Display.println("GPS Signal Invalid");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.println("Waiting for GPS fix...");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.println("Check antenna connection");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.println("Ensure clear sky view");
  
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
  M5.Display.setCursor(10, 0);
  M5.Display.println("Error");
  
  // Display error message
  M5.Display.setCursor(10, 20);
  M5.Display.println(message);
  
  // Blink LED red
  blinkPixel(COLOR_RED, COLOR_BLACK, 3, 200);
}
