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
  if (currentTime - _lastUpdateTime < 200) {  // 更新頻度を上げて応答性を向上
    return; // Skip update if less than 200ms has passed
  }
  
  // Update last update time
  _lastUpdateTime = currentTime;
  
  // Display based on the current mode
  switch (mode) {
    case RAW_IMU:
      // IMU data display
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_BLUE);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(2, 0);
      M5.Display.println("IMU RAW DATA MODE");
      
      // 実際のIMUデータを表示
      showRawIMU();
      break;
      
    case RAW_GPS:
      // GPS data display
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(2, 0);
      M5.Display.println("GPS RAW DATA MODE");
      
      // グローバル変数を直接使用せず、関数の引数として渡す
      extern float latitude, longitude, altitude;
      extern int satellites;
      extern float hdop;
      extern int hour, minute, second;
      extern bool gpsValid;
      
      // メインプログラムのgps変数を直接参照せず、関数を呼び出す
      showRawGPS(nullptr, latitude, longitude, altitude, satellites, hdop, hour, minute, second, gpsValid);
      break;
      
    case RAW_CELESTIAL:
      // Celestial data display
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_MAGENTA);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(2, 0);
      M5.Display.println("CELESTIAL DATA MODE");
      // 天体関連のグローバル変数を取得
      extern float sunAz, sunAlt, moonAz, moonAlt, moonPhase, polarisAz, polarisAlt;
      // 天体データを表示
      showRawCelestial(sunAz, sunAlt, moonAz, moonAlt, moonPhase, polarisAz, polarisAlt);
      break;
      
    case RAW_SYSTEM:
      // System information display
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_YELLOW);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(2, 0);
      M5.Display.println("SYSTEM INFO");
      // システム情報を表示
      showSystemInfo();
      break;
      
    case DISPLAY_DEBUG:
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(0xFD20);  // Orange color
      M5.Display.setTextSize(1);
      M5.Display.setCursor(2, 0);
      M5.Display.println("DEBUG MODE");
      
      // デバッグ情報を表示
      showDebugInfo("Debug Information");
      break;
      
    default:
      // Unknown mode
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setTextColor(TFT_RED);
      M5.Display.setTextSize(1);
      M5.Display.setCursor(2, 0);
      M5.Display.println("UNKNOWN MODE");
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

// IMUデータを表示する関数
void RawDataDisplay::showRawIMU() {
  // グローバル変数からIMUデータを取得
  extern float heading, pitch, roll;
  
  // 直接センサーからデータを取得
  float acc[3], gyro[3], mag[3];
  bool accOk = false, gyroOk = false, magOk = false;
  
  // センサーデータを取得 - 毎回最新のデータを取得するために直接呼び出し
  accOk = M5.Imu.getAccel(&acc[0], &acc[1], &acc[2]);    // 加速度 (g)
  gyroOk = M5.Imu.getGyro(&gyro[0], &gyro[1], &gyro[2]);  // 角速度 (dps)
  magOk = M5.Imu.getMag(&mag[0], &mag[1], &mag[2]);      // 地磁気 (μT)
  
  // テキスト設定
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  // センサー状態表示
  int y = 15; // 2行上に移動（25から15に変更）
  M5.Display.setCursor(2, y);
  M5.Display.print("Acc: ");
  M5.Display.print(accOk ? "OK" : "NG");
  M5.Display.setCursor(64, y);
  M5.Display.print("Gyro: ");
  M5.Display.print(gyroOk ? "OK" : "NG");
  y += 8;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Mag: ");
  M5.Display.print(magOk ? "OK" : "NG");
  y += 10;
  
  // 加速度データ
  if (accOk) {
    M5.Display.setCursor(2, y);
    M5.Display.print("Acc X: ");
    M5.Display.print(acc[0], 2);
    M5.Display.print(" g");
    y += 8;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Acc Y: ");
    M5.Display.print(acc[1], 2);
    M5.Display.print(" g");
    y += 8;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Acc Z: ");
    M5.Display.print(acc[2], 2);
    M5.Display.print(" g");
    y += 10;
  }
  
  // ジャイロデータ
  if (gyroOk) {
    M5.Display.setCursor(2, y);
    M5.Display.print("Gyr X: ");
    M5.Display.print(gyro[0], 1);
    M5.Display.print(" dps");
    y += 8;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Gyr Y: ");
    M5.Display.print(gyro[1], 1);
    M5.Display.print(" dps");
    y += 8;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Gyr Z: ");
    M5.Display.print(gyro[2], 1);
    M5.Display.print(" dps");
    y += 10;
  }
  
  // 地磁気データ
  if (magOk) {
    M5.Display.setCursor(2, y);
    M5.Display.print("Mag X: ");
    M5.Display.print(mag[0], 1);
    M5.Display.print(" uT");
    y += 8;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Mag Y: ");
    M5.Display.print(mag[1], 1);
    M5.Display.print(" uT");
    y += 8;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Mag Z: ");
    M5.Display.print(mag[2], 1);
    M5.Display.print(" uT");
    y += 10;
  }
  
  // 姿勢データ
  M5.Display.setCursor(2, y);
  M5.Display.print("Heading: ");
  M5.Display.print(heading, 1);
  M5.Display.print(" deg");
  y += 8;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Pitch: ");
  M5.Display.print(pitch, 1);
  M5.Display.print(" deg");
  y += 8;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Roll: ");
  M5.Display.print(roll, 1);
  M5.Display.print(" deg");
  
  // キャリブレーション情報を表示
  // 画面下部に小さく表示
  extern bool imuCalibrated;
  
  // 画面下部に長押しでキャリブレーションモードに入る情報を表示
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(2, M5.Display.height() - 16);
  M5.Display.print("Calibration: ");
  M5.Display.print(imuCalibrated ? "OK" : "NG");
  
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(2, M5.Display.height() - 8);
  M5.Display.print("Long press to calibrate");
}

// GPS生データを表示する関数
void RawDataDisplay::showRawGPS(AtomicBaseGPS* gps, 
                              float latitude, float longitude, float altitude,
                              int satellites, float hdop, 
                              int hour, int minute, int second,
                              bool valid) {
  // テキスト設定
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  int y = 25;
  
  // GPS状態
  M5.Display.setCursor(2, y);
  M5.Display.print("GPS Status: ");
  if (valid) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.print("Valid");
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.print("Invalid");
  }
  M5.Display.setTextColor(TFT_WHITE);
  y += 10;
  
  // 衛星数
  M5.Display.setCursor(2, y);
  M5.Display.print("Satellites: ");
  M5.Display.print(satellites);
  y += 10;
  
  // HDOP (水平精度)
  M5.Display.setCursor(2, y);
  M5.Display.print("HDOP: ");
  M5.Display.print(hdop, 2);
  y += 10;
  
  // 時刻
  M5.Display.setCursor(2, y);
  M5.Display.print("Time: ");
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
  M5.Display.print(timeStr);
  y += 10;
  
  // 座標情報
  if (valid) {
    // 緯度
    M5.Display.setCursor(2, y);
    M5.Display.print("Lat: ");
    M5.Display.print(latitude, 6);
    M5.Display.print(" deg");
    y += 10;
    
    // 経度
    M5.Display.setCursor(2, y);
    M5.Display.print("Lon: ");
    M5.Display.print(longitude, 6);
    M5.Display.print(" deg");
    y += 10;
    
    // 高度
    M5.Display.setCursor(2, y);
    M5.Display.print("Alt: ");
    M5.Display.print(altitude, 1);
    M5.Display.print(" m");
    y += 10;
    
    // 10進数から度分秒形式に変換して表示
    M5.Display.setCursor(2, y);
    M5.Display.print("Lat (DMS): ");
    
    int latDeg = abs((int)latitude);
    int latMin = abs((int)((latitude - (int)latitude) * 60));
    float latSec = abs((float)(latitude - (int)latitude - latMin/60.0) * 3600);
    
    char latStr[20];
    sprintf(latStr, "%d\xB0%d'%.1f\"%c", latDeg, latMin, latSec, (latitude >= 0) ? 'N' : 'S');
    M5.Display.print(latStr);
    y += 10;
    
    M5.Display.setCursor(2, y);
    M5.Display.print("Lon (DMS): ");
    
    int lonDeg = abs((int)longitude);
    int lonMin = abs((int)((longitude - (int)longitude) * 60));
    float lonSec = abs((float)(longitude - (int)longitude - lonMin/60.0) * 3600);
    
    char lonStr[20];
    sprintf(lonStr, "%d\xB0%d'%.1f\"%c", lonDeg, lonMin, lonSec, (longitude >= 0) ? 'E' : 'W');
    M5.Display.print(lonStr);
    y += 10;
  } else {
    M5.Display.setCursor(2, y);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.print("Waiting for GPS signal...");
    M5.Display.setTextColor(TFT_WHITE);
    y += 10;
  }
  
  // GPSオブジェクトが提供されている場合、追加情報を表示
  if (gps != nullptr) {
    // Fix Typeはサポートされていない可能性があるため、コメントアウト
    // M5.Display.setCursor(2, y);
    // M5.Display.print("Fix Type: ");
    // M5.Display.print(gps->getFixType());
    // y += 10;
    
    // Fix Qualityはサポートされていないため削除
    // M5.Display.setCursor(2, y);
    // M5.Display.print("Fix Quality: ");
    // M5.Display.print(gps->getFixQuality());
    // y += 10;
  }
}

// システム情報を表示する関数
void RawDataDisplay::showSystemInfo() {
  // テキスト設定
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  int y = 25;
  
  // バージョン情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Version: 1.0.0");
  y += 10;
  
  // ビルド日時
  M5.Display.setCursor(2, y);
  M5.Display.print("Build: ");
  M5.Display.print(__DATE__);
  M5.Display.print(" ");
  M5.Display.print(__TIME__);
  y += 10;
  
  // ESP32情報
  M5.Display.setCursor(2, y);
  M5.Display.print("ESP32-S3 CPU: ");
  M5.Display.print(ESP.getCpuFreqMHz());
  M5.Display.print("MHz");
  y += 10;
  
  // メモリ情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Free RAM: ");
  M5.Display.print(ESP.getFreeHeap() / 1024);
  M5.Display.print("KB / ");
  M5.Display.print(ESP.getHeapSize() / 1024);
  M5.Display.print("KB");
  y += 10;
  
  // フラッシュ情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Flash: ");
  M5.Display.print(ESP.getFlashChipSize() / (1024 * 1024));
  M5.Display.print("MB");
  y += 10;
  
  // 温度情報（M5Unifiedから取得）
  float temp = 0;
  if (M5.Imu.getTemp(&temp)) {
    M5.Display.setCursor(2, y);
    M5.Display.print("Temp: ");
    M5.Display.print(temp, 1);
    M5.Display.print("C");
    y += 10;
  }
  
  // バッテリー情報
  int batteryLevel = M5.Power.getBatteryLevel();
  if (batteryLevel >= 0) {
    M5.Display.setCursor(2, y);
    M5.Display.print("Battery: ");
    M5.Display.print(batteryLevel);
    M5.Display.print("%");
    y += 10;
    
    // 充電状態
    M5.Display.setCursor(2, y);
    M5.Display.print("Charging: ");
    M5.Display.print(M5.Power.isCharging() ? "Yes" : "No");
    y += 10;
  }
  
  // 実行時間
  M5.Display.setCursor(2, y);
  M5.Display.print("Uptime: ");
  unsigned long uptime = millis() / 1000; // 秒単位
  int hours = uptime / 3600;
  int mins = (uptime % 3600) / 60;
  int secs = uptime % 60;
  
  char uptimeStr[16];
  sprintf(uptimeStr, "%02d:%02d:%02d", hours, mins, secs);
  M5.Display.print(uptimeStr);
  y += 10;
  
  // デバイス情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Device: M5AtomS3");
  y += 10;
  
  // 接続情報
  M5.Display.setCursor(2, y);
  M5.Display.print("GPS: ");
  extern bool gpsValid;
  M5.Display.print(gpsValid ? "Connected" : "Not connected");
}

// 天体データを表示する関数
void RawDataDisplay::showRawCelestial(float sunAz, float sunAlt, 
                                    float moonAz, float moonAlt, float moonPhase,
                                    float polarisAz, float polarisAlt) {
  // テキスト設定
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  
  int y = 25;
  
  // 太陽情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Sun Az: ");
  M5.Display.print(sunAz, 1);
  M5.Display.print(" deg");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Sun Alt: ");
  M5.Display.print(sunAlt, 1);
  M5.Display.print(" deg");
  y += 10;
  
  // 月情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Moon Az: ");
  M5.Display.print(moonAz, 1);
  M5.Display.print(" deg");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Moon Alt: ");
  M5.Display.print(moonAlt, 1);
  M5.Display.print(" deg");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Moon Phase: ");
  M5.Display.print(moonPhase * 100.0, 1);
  M5.Display.print("%");
  y += 10;
  
  // 月齢の表示（0-29.5日）
  float moonAge = moonPhase * 29.53;
  M5.Display.setCursor(2, y);
  M5.Display.print("Moon Age: ");
  M5.Display.print(moonAge, 1);
  M5.Display.print(" days");
  y += 10;
  
  // 月の名称
  M5.Display.setCursor(2, y);
  M5.Display.print("Moon: ");
  if (moonPhase < 0.01) {
    M5.Display.print("New Moon");
  } else if (moonPhase < 0.25) {
    M5.Display.print("Waxing Crescent");
  } else if (moonPhase < 0.26) {
    M5.Display.print("First Quarter");
  } else if (moonPhase < 0.49) {
    M5.Display.print("Waxing Gibbous");
  } else if (moonPhase < 0.51) {
    M5.Display.print("Full Moon");
  } else if (moonPhase < 0.75) {
    M5.Display.print("Waning Gibbous");
  } else if (moonPhase < 0.76) {
    M5.Display.print("Last Quarter");
  } else {
    M5.Display.print("Waning Crescent");
  }
  y += 10;
  
  // 北極星/天の北極情報
  M5.Display.setCursor(2, y);
  M5.Display.print("Polaris Az: ");
  M5.Display.print(polarisAz, 1);
  M5.Display.print(" deg");
  y += 10;
  
  M5.Display.setCursor(2, y);
  M5.Display.print("Polaris Alt: ");
  M5.Display.print(polarisAlt, 1);
  M5.Display.print(" deg");
  y += 10;
}

// デバッグ情報を表示する関数
void RawDataDisplay::showDebugInfo(const char* debugMessage) {
  // テキスト設定
  M5.Display.setTextColor(0xFD20); // オレンジ色
  M5.Display.setTextSize(1);
  
  // グローバル変数からIMUデータを取得
  extern BMI270 bmi270;
  extern BMM150class bmm150;
  extern float heading, pitch, roll;
  
  // 直接センサーからデータを取得
  // 最新のデータを取得するために直接センサーから読み取り
  bmi270.readAcceleration();
  bmi270.readGyro();
  bmm150.readMagnetometer();
  
  // XY, YZ, XZ平面の方位角を計算
  float xy_angle = atan2(bmm150.mag_y, bmm150.mag_x) * 180.0 / PI;
  if (xy_angle < 0) xy_angle += 360.0;
  
  float yz_angle = atan2(bmm150.mag_z, bmm150.mag_y) * 180.0 / PI;
  if (yz_angle < 0) yz_angle += 360.0;
  
  float xz_angle = atan2(bmm150.mag_z, bmm150.mag_x) * 180.0 / PI;
  if (xz_angle < 0) xz_angle += 360.0;
  
  // タイトル表示
  M5.Display.setCursor(2, 10);
  M5.Display.println("SENSOR DEBUG VIEW");
  
  // コンパスローズを描画（中央上部）
  int centerX = M5.Display.width() / 2;
  int centerY = 45;
  int radius = 25;
  
  // コンパス円を描画
  M5.Display.drawCircle(centerX, centerY, radius, TFT_WHITE);
  
  // 方位の表示
  M5.Display.setCursor(centerX - 3, centerY - radius - 8);
  M5.Display.print("N");
  M5.Display.setCursor(centerX + radius + 3, centerY - 3);
  M5.Display.print("E");
  M5.Display.setCursor(centerX - 3, centerY + radius + 2);
  M5.Display.print("S");
  M5.Display.setCursor(centerX - radius - 8, centerY - 3);
  M5.Display.print("W");
  
  // 3軸をRGBの三本線で表示
  // X軸（赤）- XY平面の角度を使用
  float x_angle = -xy_angle * PI / 180.0;
  int x_end_x = centerX + radius * cos(x_angle);
  int x_end_y = centerY + radius * sin(x_angle);
  M5.Display.drawLine(centerX, centerY, x_end_x, x_end_y, TFT_RED);
  
  // Y軸（緑）- YZ平面の角度を使用
  float y_angle = -yz_angle * PI / 180.0;
  int y_end_x = centerX + radius * cos(y_angle);
  int y_end_y = centerY + radius * sin(y_angle);
  M5.Display.drawLine(centerX, centerY, y_end_x, y_end_y, TFT_GREEN);
  
  // Z軸（青）- XZ平面の角度を使用
  float z_angle = -xz_angle * PI / 180.0;
  int z_end_x = centerX + radius * cos(z_angle);
  int z_end_y = centerY + radius * sin(z_angle);
  M5.Display.drawLine(centerX, centerY, z_end_x, z_end_y, TFT_BLUE);
  
  // 方位角の数値情報を表示
  int y = centerY + radius + 10;
  
  // XY平面の方位角（通常のコンパス方位角）
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(2, y);
  M5.Display.print("XY: ");
  M5.Display.print(xy_angle, 1);
  M5.Display.print("°");
  y += 10;
  
  // YZ平面の方位角
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.setCursor(2, y);
  M5.Display.print("YZ: ");
  M5.Display.print(yz_angle, 1);
  M5.Display.print("°");
  y += 10;
  
  // XZ平面の方位角
  M5.Display.setTextColor(TFT_BLUE);
  M5.Display.setCursor(2, y);
  M5.Display.print("XZ: ");
  M5.Display.print(xz_angle, 1);
  M5.Display.print("°");
  y += 15;
  
  // センサー生値の表示
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(2, y);
  M5.Display.println("Raw Sensor Values:");
  y += 10;
  
  // 磁力計の生値
  M5.Display.setTextColor(TFT_CYAN);
  M5.Display.setCursor(2, y);
  M5.Display.print("Mag X: ");
  M5.Display.print(bmm150.mag_x, 2);
  M5.Display.setCursor(80, y);
  M5.Display.print("Y: ");
  M5.Display.print(bmm150.mag_y, 2);
  y += 10;
  M5.Display.setCursor(2, y);
  M5.Display.print("Mag Z: ");
  M5.Display.print(bmm150.mag_z, 2);
  y += 15;
  
  // 加速度計の生値
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.setCursor(2, y);
  M5.Display.print("Acc X: ");
  M5.Display.print(bmi270.acc_x, 2);
  M5.Display.setCursor(80, y);
  M5.Display.print("Y: ");
  M5.Display.print(bmi270.acc_y, 2);
  y += 10;
  M5.Display.setCursor(2, y);
  M5.Display.print("Acc Z: ");
  M5.Display.print(bmi270.acc_z, 2);
  
  // LED色をシアンに設定
  setPixelColor(0x00FFFF); // シアン
}
