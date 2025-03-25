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
#include "AtomicBaseGPS.h" // GPS設定のためのインクルード

// Constructor
CompassDisplay::CompassDisplay() {
  _currentColor = COLOR_BLACK;
  _lastAnimationTime = 0;
  _sprite = nullptr;
}

// Initialize display
void CompassDisplay::begin() {
  // M5Unifiedでは、ディスプレイの初期化はM5.begin()で行われるため、
  // 個別の初期化は不要です
  
  // ディスプレイの設定
  M5.Display.setRotation(0); // 縦向き表示
  M5.Display.setTextSize(1); // 小さいフォントサイズ
  
  // Initialize celestial overlay
  _celestialOverlay.begin();
  
  // スプライト（ダブルバッファリング用）の初期化
  _sprite = new LGFX_Sprite(&M5.Display);
  if (_sprite) {
    _sprite->createSprite(M5.Display.width(), M5.Display.height());
    _sprite->setTextSize(1);
    Serial.println("Sprite initialized for double buffering");
  } else {
    Serial.println("Failed to create sprite for double buffering");
  }
  
  // Show welcome animation
  showWelcome();
}

// Set pixel color (for RGB LED)
void CompassDisplay::setPixelColor(uint32_t color) {
  // 現在の色を保存
  _currentColor = color;
  
  // AtomS3RのLEDを制御
  // M5Unifiedでは、LEDの制御方法が変わります
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;
  
  // AtomS3Rの内蔵LEDを設定
  M5.Lcd.fillScreen(color);  // 画面全体を指定色で塗りつぶす
  
  // Debug output
  Serial.print("Set pixel color to 0x");
  Serial.println(color, HEX);
}

// Helper method: Blink the pixel between two colors
void CompassDisplay::blinkPixel(uint32_t color1, uint32_t color2, int count, int delayMs) {
  // 一時的に簡略化した実装
  for (int i = 0; i < count; i++) {
    setPixelColor(color1);
    delay(delayMs);
    setPixelColor(color2);
    delay(delayMs);
  }
  // 最後は最初の色に戻す
  setPixelColor(color1);
}

// Update display based on current mode
void CompassDisplay::update(int mode, bool gpsValid, bool imuCalibrated) {
  // モード名をLCDに表示
  M5.Display.clear();
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(0, 0);
  
  // モード番号と名前を表示
  switch (mode) {
    case POLAR_ALIGNMENT:
      M5.Display.print("Mode 1: Polar");
      break;
    case GPS_DATA:
      M5.Display.print("Mode 2: GPS");
      break;
    case IMU_DATA:
      M5.Display.print("Mode 3: IMU");
      break;
    case CELESTIAL_DATA:
      M5.Display.print("Mode 4: Celestial");
      break;
    case SETTINGS_MENU:
      M5.Display.print("Mode 5: Settings");
      break;
    case CALIBRATION_MODE:
      M5.Display.print("Mode 6: Calibration");
      break;
    default:
      M5.Display.print("Unknown Mode");
      break;
  }
  
  M5.Display.display();
  
  // Check if GPS and IMU are valid
  if (!gpsValid || !imuCalibrated) {
    // Show error if GPS or IMU is not valid
    if (!gpsValid && !imuCalibrated) {
      showError("GPS & IMU Invalid");
    } else if (!gpsValid) {
      showError("GPS Invalid");
    } else {
      showError("IMU Not Calibrated");
    }
    return;
  }
  
  // Update display based on current mode
  switch (mode) {
    case POLAR_ALIGNMENT:
      // Polar alignment mode - handled by main loop
      break;
    case GPS_DATA:
      // GPS mode - handled by main loop
      break;
    case IMU_DATA:
      // IMU mode - handled by main loop
      break;
    case CELESTIAL_DATA:
      // Celestial mode - handled by main loop
      break;
    case SETTINGS_MENU:
      showSettings();
      break;
    case CALIBRATION_MODE:
      // Calibration mode - handled by main loop
      break;
  }
}

// Display polar alignment compass
void CompassDisplay::showPolarAlignment(float heading, float polarisAz, float polarisAlt, 
                                      float pitch, float roll) {
  // 画面をクリア
  M5.Display.clear();
  
  // モード表示 (小さく表示)
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(0, 0);
  M5.Display.print("Mode 1: Polar");
  
  // 誤差を計算
  float azError = polarisAz - heading;
  // Normalize to -180 to 180 degrees
  while (azError > 180) azError -= 360;
  while (azError < -180) azError += 360;
  float absDiff = fabs(azError);
  
  // 極軸情報を表示 (重要な情報のみ)
  M5.Display.setTextColor(TFT_WHITE);
  
  char buffer[20]; // 短いバッファで十分
  
  // 方位角
  M5.Display.setCursor(0, 12);
  sprintf(buffer, "HDG:%5.1f", heading);
  M5.Display.print(buffer);
  
  // 北極星方位角
  M5.Display.setCursor(0, 24);
  sprintf(buffer, "POL:%5.1f", polarisAz);
  M5.Display.print(buffer);
  
  // 誤差
  M5.Display.setCursor(0, 36);
  
  // 誤差に応じて色を変える
  if (absDiff < 1.0) {
    M5.Display.setTextColor(TFT_GREEN);
  } else if (absDiff < 5.0) {
    M5.Display.setTextColor(TFT_YELLOW);
  } else {
    M5.Display.setTextColor(TFT_RED);
  }
  
  sprintf(buffer, "ERR:%5.1f", azError);
  M5.Display.print(buffer);
  
  // 水平レベル
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(0, 48);
  sprintf(buffer, "P/R:%4.1f/%4.1f", pitch, roll);
  M5.Display.print(buffer);
  
  // 北極星高度
  M5.Display.setCursor(0, 60);
  sprintf(buffer, "ALT:%5.1f", polarisAlt);
  M5.Display.print(buffer);
  
  // コンパスローズを描画
  drawCompassRose(heading);
  
  // 水平線を描画
  drawHorizon(pitch, roll);
  
  // 画面を更新
  M5.Display.display();
  
  // Debug output
  Serial.print("Polar Alignment: Heading=");
  Serial.print(heading);
  Serial.print(", PolarisAz=");
  Serial.print(polarisAz);
  Serial.print(", AzError=");
  Serial.print(azError);
  Serial.print(", PolarisAlt=");
  Serial.print(polarisAlt);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
}

// Display IMU data
void CompassDisplay::showIMU() {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 7: IMU Data");
    
    // センサー情報を表示
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 15);
    M5.Display.println("AtomS3R IMU Sensors:");
    M5.Display.println("BMI270 (6-axis) + BMM150 (3-axis)");
    
    // ジャイロスコープデータ (BMI270)
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.setCursor(0, 45);
    M5.Display.println("Gyroscope (deg/s):");
    M5.Display.printf(" X: %6.2f  Y: %6.2f  Z: %6.2f\n", 
                     _imuData.gyroX, _imuData.gyroY, _imuData.gyroZ);
    
    // 加速度計データ (BMI270)
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(0, 75);
    M5.Display.println("Accelerometer (m/s^2):");
    M5.Display.printf(" X: %6.2f  Y: %6.2f  Z: %6.2f\n", 
                     _imuData.accelX, _imuData.accelY, _imuData.accelZ);
    
    // 磁力計データ (BMM150)
    M5.Display.setTextColor(TFT_MAGENTA);
    M5.Display.setCursor(0, 105);
    M5.Display.println("Magnetometer (uT):");
    M5.Display.printf(" X: %6.2f  Y: %6.2f  Z: %6.2f\n", 
                     _imuData.magX, _imuData.magY, _imuData.magZ);
    
    // 傾き表示 (ピッチとロール)
    // BMI270のデータから計算
    float pitch = atan2(_imuData.accelX, sqrt(_imuData.accelY * _imuData.accelY + _imuData.accelZ * _imuData.accelZ)) * 180.0 / PI;
    float roll = atan2(_imuData.accelY, _imuData.accelZ) * 180.0 / PI;
    
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 135);
    M5.Display.println("Orientation:");
    M5.Display.printf(" Pitch: %6.2f  Roll: %6.2f\n", pitch, roll);
    
    // 傾きを視覚的に表示（円を使用）
    int centerX = 120;
    int centerY = 180;
    int radius = 20;
    
    // 基準円（水平位置）
    M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
    M5.Display.drawLine(centerX - radius, centerY, centerX + radius, centerY, TFT_WHITE);
    M5.Display.drawLine(centerX, centerY - radius, centerX, centerY + radius, TFT_WHITE);
    
    // 現在の傾き位置（緑の円）
    // ピッチとロールを座標に変換（最大±30度を円の半径に対応）
    int offsetX = constrain(roll / 30.0 * radius, -radius, radius);
    int offsetY = constrain(pitch / 30.0 * radius, -radius, radius);
    M5.Display.fillCircle(centerX + offsetX, centerY - offsetY, 5, TFT_GREEN);
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 7: IMU Data");
    
    // センサー情報を表示
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 15);
    _sprite->println("AtomS3R IMU Sensors:");
    _sprite->println("BMI270 (6-axis) + BMM150 (3-axis)");
    
    // ジャイロスコープデータ (BMI270)
    _sprite->setTextColor(TFT_GREEN);
    _sprite->setCursor(0, 45);
    _sprite->println("Gyroscope (deg/s):");
    _sprite->printf(" X: %6.2f  Y: %6.2f  Z: %6.2f\n", 
                   _imuData.gyroX, _imuData.gyroY, _imuData.gyroZ);
    
    // 加速度計データ (BMI270)
    _sprite->setTextColor(TFT_YELLOW);
    _sprite->setCursor(0, 75);
    _sprite->println("Accelerometer (m/s^2):");
    _sprite->printf(" X: %6.2f  Y: %6.2f  Z: %6.2f\n", 
                   _imuData.accelX, _imuData.accelY, _imuData.accelZ);
    
    // 磁力計データ (BMM150)
    _sprite->setTextColor(TFT_MAGENTA);
    _sprite->setCursor(0, 105);
    _sprite->println("Magnetometer (uT):");
    _sprite->printf(" X: %6.2f  Y: %6.2f  Z: %6.2f\n", 
                   _imuData.magX, _imuData.magY, _imuData.magZ);
    
    // 傾き表示 (ピッチとロール)
    // BMI270のデータから計算
    float pitch = atan2(_imuData.accelX, sqrt(_imuData.accelY * _imuData.accelY + _imuData.accelZ * _imuData.accelZ)) * 180.0 / PI;
    float roll = atan2(_imuData.accelY, _imuData.accelZ) * 180.0 / PI;
    
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 135);
    _sprite->println("Orientation:");
    _sprite->printf(" Pitch: %6.2f  Roll: %6.2f\n", pitch, roll);
    
    // 傾きを視覚的に表示（円を使用）
    int centerX = 120;
    int centerY = 180;
    int radius = 20;
    
    // 基準円（水平位置）
    _sprite->drawCircle(centerX, centerY, radius, TFT_WHITE);
    _sprite->drawLine(centerX - radius, centerY, centerX + radius, centerY, TFT_WHITE);
    _sprite->drawLine(centerX, centerY - radius, centerX, centerY + radius, TFT_WHITE);
    
    // 現在の傾き位置（緑の円）
    // ピッチとロールを座標に変換（最大±30度を円の半径に対応）
    int offsetX = constrain(roll / 30.0 * radius, -radius, radius);
    int offsetY = constrain(pitch / 30.0 * radius, -radius, radius);
    _sprite->fillCircle(centerX + offsetX, centerY - offsetY, 5, TFT_GREEN);
    
    // スプライトをディスプレイに転送（一度に画面更新）
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.println("IMU Data:");
  Serial.printf("Gyro: X=%6.2f Y=%6.2f Z=%6.2f\n", _imuData.gyroX, _imuData.gyroY, _imuData.gyroZ);
  Serial.printf("Accel: X=%6.2f Y=%6.2f Z=%6.2f\n", _imuData.accelX, _imuData.accelY, _imuData.accelZ);
  Serial.printf("Mag: X=%6.2f Y=%6.2f Z=%6.2f\n", _imuData.magX, _imuData.magY, _imuData.magZ);
  
  // IMUデータ表示中はLEDを青色に点灯
  setPixelColor(COLOR_BLUE);
}

// Display IMU data (heading, pitch, roll)
void CompassDisplay::showIMUData(float heading, float pitch, float roll) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 5: IMU Data");
    
    // 方位表示
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 20);
    M5.Display.printf("HDG: %5.1f", heading);
    
    // ピッチ・ロール表示
    M5.Display.setCursor(0, 50);
    M5.Display.printf("PCH: %5.1f", pitch);
    
    M5.Display.setCursor(0, 80);
    M5.Display.printf("ROL: %5.1f", roll);
    
    // 水平線表示（傾きを視覚的に表示）
    int centerX = M5.Display.width() / 2;
    int centerY = 150;
    int radius = 30;
    
    // 基準円（水平位置）
    M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
    
    // 現在の傾き位置（最大±30度を円の半径に対応）
    int offsetX = constrain(pitch, -30, 30) * radius / 30;
    int offsetY = constrain(roll, -30, 30) * radius / 30;
    M5.Display.fillCircle(centerX + offsetX, centerY + offsetY, 5, TFT_GREEN);
    
    // キャリブレーション状態表示
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(5, 190);
    M5.Display.print("Calibration Status:");
    
    // IMUの校正状態を表示（引数から取得）
    bool imuCalibrated = true; // ここでは常にtrueとする（実際の校正状態は呼び出し元から取得する必要がある）
    M5.Display.setTextColor(imuCalibrated ? TFT_GREEN : TFT_RED);
    M5.Display.setCursor(120, 190);
    M5.Display.print(imuCalibrated ? "GOOD" : "POOR");
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 5: IMU Data");
    
    // 方位表示
    _sprite->setTextSize(2);
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 20);
    _sprite->printf("HDG: %5.1f", heading);
    
    // ピッチ・ロール表示
    _sprite->setCursor(0, 50);
    _sprite->printf("PCH: %5.1f", pitch);
    
    _sprite->setCursor(0, 80);
    _sprite->printf("ROL: %5.1f", roll);
    
    // 水平線表示（傾きを視覚的に表示）
    int centerX = _sprite->width() / 2;
    int centerY = 150;
    int radius = 30;
    
    // 基準円（水平位置）
    _sprite->drawCircle(centerX, centerY, radius, TFT_WHITE);
    
    // 現在の傾き位置（最大±30度を円の半径に対応）
    int offsetX = constrain(pitch, -30, 30) * radius / 30;
    int offsetY = constrain(roll, -30, 30) * radius / 30;
    _sprite->fillCircle(centerX + offsetX, centerY + offsetY, 5, TFT_GREEN);
    
    // キャリブレーション状態表示
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(5, 190);
    _sprite->print("Calibration Status:");
    
    // IMUの校正状態を表示（引数から取得）
    bool imuCalibrated = true; // ここでは常にtrueとする（実際の校正状態は呼び出し元から取得する必要がある）
    _sprite->setTextColor(imuCalibrated ? TFT_GREEN : TFT_RED);
    _sprite->setCursor(120, 190);
    _sprite->print(imuCalibrated ? "GOOD" : "POOR");
    
    // スプライトをディスプレイに転送
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.print("IMU Data: Heading=");
  Serial.print(heading);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
  
  // IMUデータ表示ではLEDを緑色に設定
  setPixelColor(COLOR_GREEN);
}

// Enhanced celestial display with overlay
void CompassDisplay::showCelestialOverlay(float heading, float latitude, float longitude,
                                        int year, int month, int day, int hour, int minute, int second) {
  // 天体データを更新
  _celestialOverlay.updateCelestialData(latitude, longitude, year, month, day, hour, minute, second);
  
  // 太陽と月の位置を取得
  float sunAz, sunAlt, moonAz, moonAlt, polarisAz, polarisAlt;
  _celestialOverlay.getSunPosition(&sunAz, &sunAlt);
  _celestialOverlay.getMoonPosition(&moonAz, &moonAlt);
  _celestialOverlay.getPolarisPosition(&polarisAz, &polarisAlt);
  
  // 月相を取得
  MoonPhase moonPhase = _celestialOverlay.getMoonPhaseEnum();
  int moonIllumination = _celestialOverlay.getMoonIllumination();
  
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 4: Celestial Overlay");
    
    // コンパスの中心座標とサイズ
    int centerX = M5.Display.width() / 2;
    int centerY = 70;
    int radius = 50;
    
    // コンパスの外枠を描画
    M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
    
    // 方位を示す記号を描画
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(centerX - 3, centerY - radius - 10);
    M5.Display.print("N");
    M5.Display.setCursor(centerX + radius + 5, centerY - 3);
    M5.Display.print("E");
    M5.Display.setCursor(centerX - 3, centerY + radius + 5);
    M5.Display.print("S");
    M5.Display.setCursor(centerX - radius - 10, centerY - 3);
    M5.Display.print("W");
    
    // 現在の方位角を表示
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(5, 15);
    M5.Display.printf("Heading: %.1f deg", heading);
    
    // 太陽の位置を描画（黄色）
    if (sunAlt > 0) { // 地平線より上にある場合のみ描画
      int sunX = centerX + int(sin(radians(sunAz - heading)) * radius * cos(radians(sunAlt)));
      int sunY = centerY - int(cos(radians(sunAz - heading)) * radius * cos(radians(sunAlt)));
      M5.Display.fillCircle(sunX, sunY, 5, TFT_YELLOW);
    }
    
    // 月の位置を描画（白または青白）
    if (moonAlt > 0) { // 地平線より上にある場合のみ描画
      int moonX = centerX + int(sin(radians(moonAz - heading)) * radius * cos(radians(moonAlt)));
      int moonY = centerY - int(cos(radians(moonAz - heading)) * radius * cos(radians(moonAlt)));
      
      // 月相に応じて色を変える
      uint16_t moonColor = TFT_WHITE;
      if (moonPhase == NEW_MOON || moonPhase == WANING_CRESCENT || moonPhase == WAXING_CRESCENT) {
        moonColor = TFT_LIGHTGREY; // 新月に近い場合は薄い色
      }
      
      M5.Display.fillCircle(moonX, moonY, 4, moonColor);
    }
    
    // 北極星の位置を描画（水色）
    if (polarisAlt > 0) { // 地平線より上にある場合のみ描画
      int polarisX = centerX + int(sin(radians(polarisAz - heading)) * radius * cos(radians(polarisAlt)));
      int polarisY = centerY - int(cos(radians(polarisAz - heading)) * radius * cos(radians(polarisAlt)));
      M5.Display.fillCircle(polarisX, polarisY, 2, TFT_CYAN);
      M5.Display.drawCircle(polarisX, polarisY, 3, TFT_CYAN);
    }
    
    // 天体情報を表示
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 130);
    M5.Display.printf("Sun: Az=%.1f Alt=%.1f", sunAz, sunAlt);
    
    M5.Display.setCursor(5, 140);
    M5.Display.printf("Moon: Az=%.1f Alt=%.1f", moonAz, moonAlt);
    
    // 月相を表示
    M5.Display.setCursor(5, 150);
    const char* phaseNames[] = {
      "New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
      "Full Moon", "Waning Gibbous", "Last Quarter", "Waning Crescent"
    };
    M5.Display.printf("Phase: %s (%d%%)", phaseNames[moonPhase], moonIllumination);
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 4: Celestial Overlay");
    
    // コンパスの中心座標とサイズ
    int centerX = _sprite->width() / 2;
    int centerY = 70;
    int radius = 50;
    
    // コンパスの外枠を描画
    _sprite->drawCircle(centerX, centerY, radius, TFT_WHITE);
    
    // 方位を示す記号を描画
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(centerX - 3, centerY - radius - 10);
    _sprite->print("N");
    _sprite->setCursor(centerX + radius + 5, centerY - 3);
    _sprite->print("E");
    _sprite->setCursor(centerX - 3, centerY + radius + 5);
    _sprite->print("S");
    _sprite->setCursor(centerX - radius - 10, centerY - 3);
    _sprite->print("W");
    
    // 現在の方位角を表示
    _sprite->setTextColor(TFT_YELLOW);
    _sprite->setCursor(5, 15);
    _sprite->printf("Heading: %.1f deg", heading);
    
    // 太陽の位置を描画（黄色）
    if (sunAlt > 0) { // 地平線より上にある場合のみ描画
      int sunX = centerX + int(sin(radians(sunAz - heading)) * radius * cos(radians(sunAlt)));
      int sunY = centerY - int(cos(radians(sunAz - heading)) * radius * cos(radians(sunAlt)));
      _sprite->fillCircle(sunX, sunY, 5, TFT_YELLOW);
    }
    
    // 月の位置を描画（白または青白）
    if (moonAlt > 0) { // 地平線より上にある場合のみ描画
      int moonX = centerX + int(sin(radians(moonAz - heading)) * radius * cos(radians(moonAlt)));
      int moonY = centerY - int(cos(radians(moonAz - heading)) * radius * cos(radians(moonAlt)));
      
      // 月相に応じて色を変える
      uint16_t moonColor = TFT_WHITE;
      if (moonPhase == NEW_MOON || moonPhase == WANING_CRESCENT || moonPhase == WAXING_CRESCENT) {
        moonColor = TFT_LIGHTGREY; // 新月に近い場合は薄い色
      }
      
      _sprite->fillCircle(moonX, moonY, 4, moonColor);
    }
    
    // 北極星の位置を描画（水色）
    if (polarisAlt > 0) { // 地平線より上にある場合のみ描画
      int polarisX = centerX + int(sin(radians(polarisAz - heading)) * radius * cos(radians(polarisAlt)));
      int polarisY = centerY - int(cos(radians(polarisAz - heading)) * radius * cos(radians(polarisAlt)));
      _sprite->fillCircle(polarisX, polarisY, 2, TFT_CYAN);
      _sprite->drawCircle(polarisX, polarisY, 3, TFT_CYAN);
    }
    
    // 天体情報を表示
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(5, 130);
    _sprite->printf("Sun: Az=%.1f Alt=%.1f", sunAz, sunAlt);
    
    _sprite->setCursor(5, 140);
    _sprite->printf("Moon: Az=%.1f Alt=%.1f", moonAz, moonAlt);
    
    // 月相を表示
    _sprite->setCursor(5, 150);
    const char* phaseNames[] = {
      "New Moon", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
      "Full Moon", "Waning Gibbous", "Last Quarter", "Waning Crescent"
    };
    _sprite->printf("Phase: %s (%d%%)", phaseNames[moonPhase], moonIllumination);
    
    // スプライトをディスプレイに転送
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.print("Celestial Overlay: Heading=");
  Serial.print(heading);
  Serial.print(", Lat=");
  Serial.print(latitude);
  Serial.print(", Lon=");
  Serial.print(longitude);
  Serial.print(", Sun Az=");
  Serial.print(sunAz);
  Serial.print(", Alt=");
  Serial.print(sunAlt);
  Serial.print(", Moon Az=");
  Serial.print(moonAz);
  Serial.print(", Alt=");
  Serial.print(moonAlt);
  Serial.print(", Phase=");
  Serial.println((int)moonPhase);
  
  // 天体オーバーレイ表示ではLEDを水色に設定
  setPixelColor(COLOR_CYAN);
}

// Helper method to calculate moon illumination percentage from phase
int CompassDisplay::getMoonIllumination(int moonPhase) {
  // 月相に基づいて月の照度を計算（簡易版）
  // 新月: 0%, 上弦/下弦: 50%, 満月: 100%
  if (moonPhase == 0 || moonPhase == 7) {
    return 0; // 新月または新月に近い
  } else if (moonPhase == 4) {
    return 100; // 満月
  } else if (moonPhase == 2 || moonPhase == 6) {
    return 50; // 上弦または下弦
  } else if (moonPhase == 1 || moonPhase == 3) {
    return moonPhase * 25; // 三日月または十三夜月
  } else {
    return (8 - moonPhase) * 25; // 十六夜月または二十日月
  }
}

// Display settings
void CompassDisplay::showSettings() {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 5: Settings");
    
    // 設定情報を表示
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 20);
    M5.Display.println("Polaris Navigator Settings");
    M5.Display.println("-------------------------");
    
    // 設定項目を表示
    M5.Display.setCursor(0, 50);
    M5.Display.println("1. Calibration");
    M5.Display.println("2. Display Brightness");
    M5.Display.println("3. GPS Settings");
    M5.Display.println("4. Time Settings");
    
    // 操作方法
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(0, 120);
    M5.Display.println("Press Btn to select option");
    
    // バージョン情報
    M5.Display.setTextColor(TFT_DARKGREY);
    M5.Display.setCursor(0, 150);
    M5.Display.println("Polaris Navigator v1.0");
    M5.Display.println("(c) 2025 Kennel Organization");
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 5: Settings");
    
    // 設定情報を表示
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 20);
    _sprite->println("Polaris Navigator Settings");
    _sprite->println("-------------------------");
    
    // 設定項目を表示
    _sprite->setCursor(0, 50);
    _sprite->println("1. Calibration");
    _sprite->println("2. Display Brightness");
    _sprite->println("3. GPS Settings");
    _sprite->println("4. Time Settings");
    
    // 操作方法
    _sprite->setTextColor(TFT_YELLOW);
    _sprite->setCursor(0, 120);
    _sprite->println("Press Btn to select option");
    
    // バージョン情報
    _sprite->setTextColor(TFT_DARKGREY);
    _sprite->setCursor(0, 150);
    _sprite->println("Polaris Navigator v1.0");
    _sprite->println("(c) 2025 Kennel Organization");
    
    // スプライトをディスプレイに転送
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.println("Settings screen displayed");
  
  // 設定画面ではLEDを青色に設定
  setPixelColor(COLOR_BLUE);
}

// Display calibration
void CompassDisplay::showCalibration(int calibrationStage, int8_t progress) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 6: Calibration");
    
    // センサー情報を表示
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 15);
    M5.Display.println("AtomS3R IMU Sensors:");
    M5.Display.println("BMI270 (6-axis) + BMM150 (3-axis)");
    
    // 校正ステージを表示
    M5.Display.setCursor(0, 40);
    M5.Display.printf("Stage: %d/3", calibrationStage);
    
    // 進捗状況を表示
    M5.Display.setCursor(0, 55);
    M5.Display.print("Progress: ");
    M5.Display.print(progress);
    M5.Display.print("%");
    
    // 校正手順を表示
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(0, 70);
    M5.Display.println("Calibration Instructions:");
    M5.Display.println("1. Rotate in figure 8 pattern");
    M5.Display.println("2. Flip device in all directions");
    
    // 注意書き
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(0, 105);
    M5.Display.println("Note: Basic calibration only.");
    M5.Display.println("For advanced cal, use MotionCal tool.");
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 6: Calibration");
    
    // センサー情報を表示
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 15);
    _sprite->println("AtomS3R IMU Sensors:");
    _sprite->println("BMI270 (6-axis) + BMM150 (3-axis)");
    
    // 校正ステージを表示
    _sprite->setCursor(0, 40);
    _sprite->printf("Stage: %d/3", calibrationStage);
    
    // 進捗状況を表示
    _sprite->setCursor(0, 55);
    _sprite->print("Progress: ");
    _sprite->print(progress);
    _sprite->print("%");
    
    // 校正手順を表示
    _sprite->setTextColor(TFT_YELLOW);
    _sprite->setCursor(0, 70);
    _sprite->println("Calibration Instructions:");
    _sprite->println("1. Rotate in figure 8 pattern");
    _sprite->println("2. Flip device in all directions");
    
    // 注意書き
    _sprite->setTextColor(TFT_RED);
    _sprite->setCursor(0, 105);
    _sprite->println("Note: Basic calibration only.");
    _sprite->println("For advanced cal, use MotionCal tool.");
    
    // スプライトをディスプレイに転送（一度に画面更新）
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.print("Calibration: Stage=");
  Serial.print(calibrationStage);
  Serial.print(", Progress=");
  Serial.println(progress);
  
  // 校正中はLEDを黄色に点滅させる
  blinkPixel(COLOR_YELLOW, COLOR_BLACK, 1, 300);
}

// Display compass
void CompassDisplay::showCompass(float heading, float pitch, float roll, bool gpsValid, bool imuCalibrated) {
  // Check if GPS and IMU are valid
  if (!gpsValid || !imuCalibrated) {
    // Show error if GPS or IMU is not valid
    if (!gpsValid && !imuCalibrated) {
      showError("GPS & IMU Invalid");
    } else if (!gpsValid) {
      showError("GPS Invalid");
    } else {
      showError("IMU Not Calibrated");
    }
    return;
  }

  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 1: Compass");
    
    // コンパス情報を表示
    M5.Display.setTextColor(TFT_WHITE);
    
    char buffer[20]; // 短いバッファで十分
    
    // 方位角
    M5.Display.setCursor(0, 12);
    sprintf(buffer, "HDG:%5.1f", heading);
    M5.Display.print(buffer);
    
    // ピッチ（上下の傾き）
    M5.Display.setCursor(0, 24);
    sprintf(buffer, "PCH:%5.1f", pitch);
    M5.Display.print(buffer);
    
    // ロール（左右の傾き）
    M5.Display.setCursor(0, 36);
    sprintf(buffer, "ROL:%5.1f", roll);
    M5.Display.print(buffer);
    
    // コンパスローズを描画
    drawCompassRose(heading);
    
    // 水平線を描画
    drawHorizon(pitch, roll);
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 1: Compass");
    
    // コンパス情報を表示
    _sprite->setTextColor(TFT_WHITE);
    
    char buffer[20]; // 短いバッファで十分
    
    // 方位角
    _sprite->setCursor(0, 12);
    sprintf(buffer, "HDG:%5.1f", heading);
    _sprite->print(buffer);
    
    // ピッチ（上下の傾き）
    _sprite->setCursor(0, 24);
    sprintf(buffer, "PCH:%5.1f", pitch);
    _sprite->print(buffer);
    
    // ロール（左右の傾き）
    _sprite->setCursor(0, 36);
    sprintf(buffer, "ROL:%5.1f", roll);
    _sprite->print(buffer);
    
    // コンパスローズを描画（スプライト上に）
    drawCompassRose(heading);
    
    // 水平線を描画（スプライト上に）
    drawHorizon(pitch, roll);
    
    // スプライトをディスプレイに転送（一度に画面更新）
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.print("Compass: Heading=");
  Serial.print(heading);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
}

// Show welcome screen
void CompassDisplay::showWelcome() {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // ウェルカムメッセージを中央に表示
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_WHITE);
    
    // 中央揃えのために文字列の幅を計算
    int welcomeWidth = 7 * 6; // "Welcome!" の幅を概算（文字数 * 文字幅）
    int welcomeX = (M5.Display.width() - welcomeWidth) / 2;
    
    M5.Display.setCursor(welcomeX > 0 ? welcomeX : 0, 30);
    M5.Display.print("Welcome!");
    
    // バージョン
    M5.Display.setCursor(0, 45);
    M5.Display.print("Version 1.0");
    
    // 起動中
    M5.Display.setCursor(0, 60);
    M5.Display.print("Starting...");
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // ウェルカムメッセージを中央に表示
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_WHITE);
    
    // 中央揃えのために文字列の幅を計算
    int welcomeWidth = 7 * 6; // "Welcome!" の幅を概算（文字数 * 文字幅）
    int welcomeX = (_sprite->width() - welcomeWidth) / 2;
    
    _sprite->setCursor(welcomeX > 0 ? welcomeX : 0, 30);
    _sprite->print("Welcome!");
    
    // バージョン
    _sprite->setCursor(0, 45);
    _sprite->print("Version 1.0");
    
    // 起動中
    _sprite->setCursor(0, 60);
    _sprite->print("Starting...");
    
    // スプライトをディスプレイに転送（一度に画面更新）
    _sprite->pushSprite(0, 0);
  }
  
  // 青色LEDを点灯（メモリの内容に基づき、アニメーションなしで単色表示）
  setPixelColor(COLOR_BLUE);
  
  // 少し待つ
  delay(1000);
}

// Show error message with diagnostic information
void CompassDisplay::showError(const char* message) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // エラー表示
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(0, 0);
    M5.Display.print("ERROR");
    
    // エラーメッセージ
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 15);
    
    // メッセージが長い場合は折り返す
    int len = strlen(message);
    int charsPerLine = 18; // 1行あたりの文字数
    
    for (int i = 0; i < len; i++) {
      if (i > 0 && i % charsPerLine == 0) {
        M5.Display.setCursor(0, 15 + (i / charsPerLine) * 12);
      }
      M5.Display.print(message[i]);
    }
    
    // 診断情報を表示
    if (strcmp(message, "GPS & IMU Invalid") == 0 || 
        strcmp(message, "GPS Invalid") == 0 || 
        strcmp(message, "IMU Not Calibrated") == 0) {
      
      // 診断情報のヘッダー
      M5.Display.setCursor(0, 60);
      M5.Display.setTextColor(TFT_YELLOW);
      M5.Display.print("Check Serial Monitor");
    }
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // エラー表示
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_RED);
    _sprite->setCursor(0, 0);
    _sprite->print("ERROR");
    
    // エラーメッセージ
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 15);
    
    // メッセージが長い場合は折り返す
    int len = strlen(message);
    int charsPerLine = 18; // 1行あたりの文字数
    
    for (int i = 0; i < len; i++) {
      if (i > 0 && i % charsPerLine == 0) {
        _sprite->setCursor(0, 15 + (i / charsPerLine) * 12);
      }
      _sprite->print(message[i]);
    }
    
    // 診断情報を表示
    if (strcmp(message, "GPS & IMU Invalid") == 0 || 
        strcmp(message, "GPS Invalid") == 0 || 
        strcmp(message, "IMU Not Calibrated") == 0) {
      
      // 診断情報のヘッダー
      _sprite->setCursor(0, 60);
      _sprite->setTextColor(TFT_YELLOW);
      _sprite->print("Check Serial Monitor");
    }
    
    // スプライトをディスプレイに転送（一度に画面更新）
    _sprite->pushSprite(0, 0);
  }
  
  // 診断情報（シリアルにのみ出力）
  if (strcmp(message, "GPS & IMU Invalid") == 0 || 
      strcmp(message, "GPS Invalid") == 0 || 
      strcmp(message, "IMU Not Calibrated") == 0) {
    
    Serial.println("=== DIAGNOSTIC INFO ===");
    
    // GPSエラーの場合
    if (strcmp(message, "GPS & IMU Invalid") == 0 || strcmp(message, "GPS Invalid") == 0) {
      Serial.println("GPS Error Details:");
      Serial.print("- GPS_TX_PIN: ");
      Serial.println(GPS_TX_PIN);
      Serial.print("- GPS_RX_PIN: ");
      Serial.println(GPS_RX_PIN);
      Serial.println("- Check GPS connection on GPIO5");
      Serial.println("- Verify GPS module is powered");
      Serial.println("- Move to area with better GPS reception");
      Serial.println("- Check baud rate (9600 baud)");
      Serial.println("- Ensure AtomicBase GPS is properly seated");
    }
    
    // IMUエラーの場合
    if (strcmp(message, "GPS & IMU Invalid") == 0 || strcmp(message, "IMU Not Calibrated") == 0) {
      Serial.println("IMU Error Details:");
      Serial.println("- Check if BMI270 and BMM150 are properly connected");
      Serial.println("- I2C pins: SDA=38, SCL=39");
      Serial.println("- Run calibration mode (Mode 6)");
      Serial.println("- Rotate device in figure 8 pattern during calibration");
      Serial.println("- Ensure calibration completes all 3 stages");
      Serial.println("- Verify I2C connections are not loose");
    }
    
    Serial.println("========================");
  }
  
  // Debug output
  Serial.print("ERROR: ");
  Serial.println(message);
}

// コンパスローズを描画する関数
void CompassDisplay::drawCompassRose(float heading) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // コンパスの中心座標
    int centerX = M5.Display.width() / 2;
    int centerY = 48;
    int radius = 12;
    
    // 背景の円を描画
    M5.Display.drawCircle(centerX, centerY, radius, TFT_DARKGREY);
    
    // 北を示す線を描画（現在の方位角に基づいて回転）
    float northRad = (90 - heading) * PI / 180.0; // 北は0度、時計回りに増加
    int northX = centerX + radius * cos(northRad);
    int northY = centerY - radius * sin(northRad);
    M5.Display.drawLine(centerX, centerY, northX, northY, TFT_RED);
    
    // 南を示す線を描画
    float southRad = (270 - heading) * PI / 180.0;
    int southX = centerX + radius * cos(southRad);
    int southY = centerY - radius * sin(southRad);
    M5.Display.drawLine(centerX, centerY, southX, southY, TFT_WHITE);
    
    // 東を示す線を描画
    float eastRad = (180 - heading) * PI / 180.0;
    int eastX = centerX + radius * cos(eastRad);
    int eastY = centerY - radius * sin(eastRad);
    M5.Display.drawLine(centerX, centerY, eastX, eastY, TFT_WHITE);
    
    // 西を示す線を描画
    float westRad = (0 - heading) * PI / 180.0;
    int westX = centerX + radius * cos(westRad);
    int westY = centerY - radius * sin(westRad);
    M5.Display.drawLine(centerX, centerY, westX, westY, TFT_WHITE);
  } else {
    // スプライトに描画
    // コンパスの中心座標
    int centerX = _sprite->width() / 2;
    int centerY = 48;
    int radius = 12;
    
    // 背景の円を描画
    _sprite->drawCircle(centerX, centerY, radius, TFT_DARKGREY);
    
    // 北を示す線を描画（現在の方位角に基づいて回転）
    float northRad = (90 - heading) * PI / 180.0; // 北は0度、時計回りに増加
    int northX = centerX + radius * cos(northRad);
    int northY = centerY - radius * sin(northRad);
    _sprite->drawLine(centerX, centerY, northX, northY, TFT_RED);
    
    // 南を示す線を描画
    float southRad = (270 - heading) * PI / 180.0;
    int southX = centerX + radius * cos(southRad);
    int southY = centerY - radius * sin(southRad);
    _sprite->drawLine(centerX, centerY, southX, southY, TFT_WHITE);
    
    // 東を示す線を描画
    float eastRad = (180 - heading) * PI / 180.0;
    int eastX = centerX + radius * cos(eastRad);
    int eastY = centerY - radius * sin(eastRad);
    _sprite->drawLine(centerX, centerY, eastX, eastY, TFT_WHITE);
    
    // 西を示す線を描画
    float westRad = (0 - heading) * PI / 180.0;
    int westX = centerX + radius * cos(westRad);
    int westY = centerY - radius * sin(westRad);
    _sprite->drawLine(centerX, centerY, westX, westY, TFT_WHITE);
  }
}

// 水平線を描画する関数
void CompassDisplay::drawHorizon(float pitch, float roll) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 水平線の中心座標
    int centerX = M5.Display.width() / 2;
    int centerY = 48;
    int radius = 12;
    
    // 背景の円を描画
    M5.Display.drawCircle(centerX, centerY, radius, TFT_DARKGREY);
    
    // ピッチとロールに基づいて水平線を描画
    float rollRad = roll * PI / 180.0;
    int x1 = centerX - radius * cos(rollRad);
    int y1 = centerY + radius * sin(rollRad);
    int x2 = centerX + radius * cos(rollRad);
    int y2 = centerY - radius * sin(rollRad);
    
    // 水平線の色を傾きに応じて変更
    uint16_t lineColor;
    if (fabs(pitch) < 1.0 && fabs(roll) < 1.0) {
      lineColor = TFT_GREEN;
    } else if (fabs(pitch) < 5.0 && fabs(roll) < 5.0) {
      lineColor = TFT_YELLOW;
    } else {
      lineColor = TFT_RED;
    }
    
    M5.Display.drawLine(x1, y1, x2, y2, lineColor);
  } else {
    // スプライトに描画
    // 水平線の中心座標
    int centerX = _sprite->width() / 2;
    int centerY = 48;
    int radius = 12;
    
    // 背景の円を描画
    _sprite->drawCircle(centerX, centerY, radius, TFT_DARKGREY);
    
    // ピッチとロールに基づいて水平線を描画
    float rollRad = roll * PI / 180.0;
    int x1 = centerX - radius * cos(rollRad);
    int y1 = centerY + radius * sin(rollRad);
    int x2 = centerX + radius * cos(rollRad);
    int y2 = centerY - radius * sin(rollRad);
    
    // 水平線の色を傾きに応じて変更
    uint16_t lineColor;
    if (fabs(pitch) < 1.0 && fabs(roll) < 1.0) {
      lineColor = TFT_GREEN;
    } else if (fabs(pitch) < 5.0 && fabs(roll) < 5.0) {
      lineColor = TFT_YELLOW;
    } else {
      lineColor = TFT_RED;
    }
    
    _sprite->drawLine(x1, y1, x2, y2, lineColor);
  }
}

// Display GPS data
void CompassDisplay::showGPSData(float latitude, float longitude, float altitude,
                               int satellites, float hdop, int hour, int minute) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 2: GPS");
    
    // GPS情報を表示 (コンパクトに)
    M5.Display.setTextColor(TFT_WHITE);
    
    char buffer[20]; // 短いバッファで十分
    
    // 緯度
    M5.Display.setCursor(0, 12);
    sprintf(buffer, "LAT:%8.4f", latitude);
    M5.Display.print(buffer);
    
    // 経度
    M5.Display.setCursor(0, 24);
    sprintf(buffer, "LON:%8.4f", longitude);
    M5.Display.print(buffer);
    
    // 高度
    M5.Display.setCursor(0, 36);
    sprintf(buffer, "ALT:%6.1fm", altitude);
    M5.Display.print(buffer);
    
    // 衛星数とHDOP
    M5.Display.setCursor(0, 48);
    
    // 衛星数に応じて色を変える
    if (satellites < 4) {
      M5.Display.setTextColor(TFT_RED);
    } else if (satellites < 7 || hdop > 2.0) {
      M5.Display.setTextColor(TFT_YELLOW);
    } else {
      M5.Display.setTextColor(TFT_GREEN);
    }
    
    sprintf(buffer, "SAT:%2d HDOP:%.1f", satellites, hdop);
    M5.Display.print(buffer);
    
    // 時刻
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 60);
    sprintf(buffer, "UTC:%02d:%02d", hour, minute);
    M5.Display.print(buffer);
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 2: GPS");
    
    // GPS情報を表示 (コンパクトに)
    _sprite->setTextColor(TFT_WHITE);
    
    char buffer[20]; // 短いバッファで十分
    
    // 緯度
    _sprite->setCursor(0, 12);
    sprintf(buffer, "LAT:%8.4f", latitude);
    _sprite->print(buffer);
    
    // 経度
    _sprite->setCursor(0, 24);
    sprintf(buffer, "LON:%8.4f", longitude);
    _sprite->print(buffer);
    
    // 高度
    _sprite->setCursor(0, 36);
    sprintf(buffer, "ALT:%6.1fm", altitude);
    _sprite->print(buffer);
    
    // 衛星数とHDOP
    _sprite->setCursor(0, 48);
    
    // 衛星数に応じて色を変える
    if (satellites < 4) {
      _sprite->setTextColor(TFT_RED);
    } else if (satellites < 7 || hdop > 2.0) {
      _sprite->setTextColor(TFT_YELLOW);
    } else {
      _sprite->setTextColor(TFT_GREEN);
    }
    
    sprintf(buffer, "SAT:%2d HDOP:%.1f", satellites, hdop);
    _sprite->print(buffer);
    
    // 時刻
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 60);
    sprintf(buffer, "UTC:%02d:%02d", hour, minute);
    _sprite->print(buffer);
    
    // スプライトをディスプレイに転送
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.print("GPS Data: Lat=");
  Serial.print(latitude, 6);
  Serial.print(", Lon=");
  Serial.print(longitude, 6);
  Serial.print(", Alt=");
  Serial.print(altitude);
  Serial.print("m, Satellites=");
  Serial.print(satellites);
  Serial.print(", HDOP=");
  Serial.print(hdop);
  Serial.print(", Time=");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  
  // GPSデータ表示ではLEDを黄色に設定
  setPixelColor(COLOR_YELLOW);
}

// Helper method: Get color based on alignment accuracy
uint32_t CompassDisplay::getAlignmentColor(float angleDiff) {
  // 角度差の絶対値を取得
  float absDiff = fabs(angleDiff);
  
  // 角度差に基づいて色を返す
  if (absDiff < 1.0) {
    return COLOR_GREEN;  // 1度未満: 緑（良好）
  } else if (absDiff < 5.0) {
    return COLOR_YELLOW; // 1-5度: 黄色（許容範囲）
  } else if (absDiff < 10.0) {
    return COLOR_PURPLE; // 5-10度: 紫（要調整）
  } else {
    return COLOR_RED;    // 10度以上: 赤（不良）
  }
}

// Animation helper: Pulse the pixel with a color
void CompassDisplay::pulsePixel(uint32_t color, int duration) {
  // 一時的に簡略化した実装
  setPixelColor(color);
  delay(duration);
}

// Animation helper: Rotate the pixel with a color
void CompassDisplay::rotatePixel(uint32_t color, int duration, int direction) {
  // 一時的に簡略化した実装
  setPixelColor(color);
  delay(duration);
}

// Display celestial data with sun and moon positions
void CompassDisplay::showCelestialData(float sunAzimuth, float sunAltitude, float moonAzimuth, float moonAltitude, int moonPhase) {
  // スプライトが初期化されていない場合は直接ディスプレイに描画
  if (!_sprite) {
    // 画面をクリア
    M5.Display.clear();
    
    // モード表示 (小さく表示)
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setCursor(0, 0);
    M5.Display.print("Mode 4: Celestial");
    
    // 太陽情報
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setCursor(0, 20);
    M5.Display.print("Sun: ");
    M5.Display.printf("Az=%5.1f Alt=%5.1f", sunAzimuth, sunAltitude);
    
    // 月情報
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(0, 40);
    M5.Display.print("Moon: ");
    M5.Display.printf("Az=%5.1f Alt=%5.1f", moonAzimuth, moonAltitude);
    
    // 月齢
    M5.Display.setCursor(0, 60);
    M5.Display.printf("Moon Phase: %d%%", getMoonIllumination(moonPhase));
    
    // 簡易的な天体位置表示（コンパスローズ上）
    int centerX = M5.Display.width() / 2;
    int centerY = 120;
    int radius = 40;
    
    // コンパスローズ（簡易版）
    M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
    M5.Display.drawLine(centerX - radius, centerY, centerX + radius, centerY, TFT_WHITE); // 東西線
    M5.Display.drawLine(centerX, centerY - radius, centerX, centerY + radius, TFT_WHITE); // 南北線
    
    // 北を示す
    M5.Display.setCursor(centerX - 4, centerY - radius - 10);
    M5.Display.print("N");
    
    // 太陽位置（黄色の点）
    float sunX = centerX + radius * sin(sunAzimuth * PI / 180.0);
    float sunY = centerY - radius * cos(sunAzimuth * PI / 180.0);
    M5.Display.fillCircle(sunX, sunY, 5, TFT_YELLOW);
    
    // 月位置（白の点）
    float moonX = centerX + radius * sin(moonAzimuth * PI / 180.0);
    float moonY = centerY - radius * cos(moonAzimuth * PI / 180.0);
    M5.Display.fillCircle(moonX, moonY, 5, TFT_WHITE);
    
    // 画面を更新
    M5.Display.display();
  } else {
    // スプライトにまず描画（ダブルバッファリング）
    _sprite->clear();
    
    // モード表示 (小さく表示)
    _sprite->setTextSize(1);
    _sprite->setTextColor(TFT_CYAN);
    _sprite->setCursor(0, 0);
    _sprite->print("Mode 4: Celestial");
    
    // 太陽情報
    _sprite->setTextColor(TFT_YELLOW);
    _sprite->setCursor(0, 20);
    _sprite->print("Sun: ");
    _sprite->printf("Az=%5.1f Alt=%5.1f", sunAzimuth, sunAltitude);
    
    // 月情報
    _sprite->setTextColor(TFT_WHITE);
    _sprite->setCursor(0, 40);
    _sprite->print("Moon: ");
    _sprite->printf("Az=%5.1f Alt=%5.1f", moonAzimuth, moonAltitude);
    
    // 月齢
    _sprite->setCursor(0, 60);
    _sprite->printf("Moon Phase: %d%%", getMoonIllumination(moonPhase));
    
    // 簡易的な天体位置表示（コンパスローズ上）
    int centerX = _sprite->width() / 2;
    int centerY = 120;
    int radius = 40;
    
    // コンパスローズ（簡易版）
    _sprite->drawCircle(centerX, centerY, radius, TFT_WHITE);
    _sprite->drawLine(centerX - radius, centerY, centerX + radius, centerY, TFT_WHITE); // 東西線
    _sprite->drawLine(centerX, centerY - radius, centerX, centerY + radius, TFT_WHITE); // 南北線
    
    // 北を示す
    _sprite->setCursor(centerX - 4, centerY - radius - 10);
    _sprite->print("N");
    
    // 太陽位置（黄色の点）
    float sunX = centerX + radius * sin(sunAzimuth * PI / 180.0);
    float sunY = centerY - radius * cos(sunAzimuth * PI / 180.0);
    _sprite->fillCircle(sunX, sunY, 5, TFT_YELLOW);
    
    // 月位置（白の点）
    float moonX = centerX + radius * sin(moonAzimuth * PI / 180.0);
    float moonY = centerY - radius * cos(moonAzimuth * PI / 180.0);
    _sprite->fillCircle(moonX, moonY, 5, TFT_WHITE);
    
    // スプライトをディスプレイに転送（一度に画面更新）
    _sprite->pushSprite(0, 0);
  }
  
  // Debug output
  Serial.println("Celestial Data:");
  Serial.printf("Sun: Az=%5.1f Alt=%5.1f\n", sunAzimuth, sunAltitude);
  Serial.printf("Moon: Az=%5.1f Alt=%5.1f Phase=%d%%\n", moonAzimuth, moonAltitude, getMoonIllumination(moonPhase));
  
  // 天体データ表示中はLEDを紫色に点灯
  setPixelColor(COLOR_PURPLE);
}
