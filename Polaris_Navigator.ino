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
#include <M5Unified.h>      // For AtomS3R
// #include <Wire.h>        // I2C communication - M5Unifiedに含まれるため不要

// IMU related
#include "src/BMI270.h"          // Accelerometer and Gyroscope
#include "src/BMM150class.h"     // Magnetometer
#include "src/IMUFusion.h"       // Sensor fusion

// GPS related
#include "src/AtomicBaseGPS.h"   // AtomicBase GPS module
#include <TinyGPSPlus.h>         // GPS parser
#include "src/GPSDataManager.h"  // GPS data storage manager

// Display related
#include "src/CompassDisplay.h"  // Compass display
#include "src/RawDataDisplay.h"  // Raw data display
#include "src/CelestialOverlay.h" // Celestial overlay
#include "src/DisplayModes.h"    // Display mode definitions
#include "src/logo.h"            // App logo

// SPIFFS must be included before PNGLoader.h
#include <SPIFFS.h>              // SPIFFSファイルシステム
#include "src/PNGLoader.h"       // PNG画像ローダー

// Celestial calculations
#include "src/celestial_math.h"  // Custom celestial calculations

// Calibration and Settings
#include "src/CalibrationManager.h" // Sensor calibration
#include "src/SettingsManager.h"    // User settings
#include "src/SettingsMenu.h"       // Settings menu interface

// Constants
#define GPS_BAUD 9600        // GPS baud rate
#define SERIAL_BAUD 115200   // Serial monitor baud rate
#define UPDATE_INTERVAL 200  // LCD更新間隔（ミリ秒）- 応答性向上のため短く設定

// GPS pins for AtomicBase GPS
// 注: これらの定義はAtomicBaseGPS.hですでに定義されているため、ここでは参照用です
// #define GPS_TX_PIN 5         // GPS TX pin
// #define GPS_RX_PIN -1        // GPS RX pin (not used for AtomicBase)

// Global variables
AtomicBaseGPS gps;           // GPS object
BMI270 bmi270;               // IMU object
BMM150class bmm150;          // Magnetometer object
IMUFusion imuFusion(&bmi270, &bmm150); // Sensor fusion
CompassDisplay display;      // Display object
RawDataDisplay rawDisplay;   // Raw data display object
CalibrationManager calibrationManager(&bmi270, &bmm150); // Calibration manager
SettingsManager settingsManager;  // Settings manager
SettingsMenu settingsMenu(&settingsManager); // Settings menu
GPSDataManager gpsDataManager;  // GPS data manager

// GPS data
float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;
int satellites = 0;
float hdop = 99.99;          // Horizontal dilution of precision
bool gpsValid = false;
bool locationValid = false;
bool altitudeValid = false;
bool timeValid = false;
bool dateValid = false;
bool speedValid = false;
bool courseValid = false;
bool wasGpsValid = false;    // 前回のGPS有効状態を記録
unsigned long lastGpsUpdate = 0; // 最終GPS更新時刻
TinyGPSDate gpsDate;
TinyGPSTime gpsTime;
float speed = 0.0;
float course = 0.0;

// Time data
int year = 2025;
int month = 3;
int day = 23;
int hour = 0;
int minute = 0;
int second = 0;

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
DisplayMode currentMode = POLAR_ALIGNMENT;
RawDataMode currentRawMode = RAW_IMU;

// Function prototypes
void setupHardware();
void setupIMU();
void setupGPS();
void readGPS();
void readIMU();
void calculateCelestialPositions();
void updateDisplay();
void handleButtonPress();
void cycleDisplayMode();
void handleLongPress();
void cycleRawDataMode();
void calibrateIMU();

// Get temperature from internal sensor
float getTemperature() {
  // M5Unifiedライブラリを使用して温度を取得
  // BMI270の内部温度センサーを使用
  float temp = 0.0f;
  
  // M5.Imu.getTemp()を使用して温度を取得
  if (M5.Imu.getType()) {
    M5.Imu.getTemp(&temp);
    Serial.print("IMU Temperature: ");
    Serial.println(temp);
    return temp;
  }
  
  // センサーが利用できない場合は仮の温度値を返す
  return 25.0f; // 仮の温度値（摂氏）
}

// Timing variables
unsigned long lastUpdateTime = 0;
unsigned long lastDisplayTime = 0;

void setup() {
  // Initialize hardware
  setupHardware();
  
  // スプラッシュ画面（起動画面）の表示
  M5.Display.fillScreen(TFT_NAVY);
  
  // ロゴの表示（PNG画像を使用）
  if (SPIFFS.exists("/logo.png")) {
    // PNG画像が存在する場合はそれを表示
    int centerX = (M5.Display.width() - 64) / 2;
    int centerY = 20;
    if (drawPNG("/logo.png", centerX, centerY)) {
      Serial.println("PNG logo displayed successfully");
    } else {
      // PNG表示に失敗した場合はバックアップとして組み込みロゴを使用
      Serial.println("Falling back to embedded logo");
      drawNavigatorLogo(32, 20, 64);
    }
  } else {
    // PNG画像がない場合は組み込みロゴを使用
    Serial.println("PNG logo not found, using embedded logo");
    drawNavigatorLogo(32, 20, 64);
  }
  
  // アプリ名の表示
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(20, 100);
  M5.Display.println("Polaris Navigator");
  
  // バージョン情報
  M5.Display.setTextSize(0.8);
  M5.Display.setCursor(35, 115);
  M5.Display.println("v1.0.0 (2025)");
  
  delay(2000); // スプラッシュ画面を2秒間表示
  
  // Initialize sensors
  setupIMU();
  setupGPS();
  
  // Initialize display
  display.begin();
  
  // Initialize raw data display
  rawDisplay.begin();
  
  // Initialize calibration manager
  calibrationManager.begin();
  
  // Initialize settings manager and menu
  settingsManager.begin();
  settingsMenu.begin();
  
  // Initialize GPS data manager
  gpsDataManager.begin();
  
  // Initialize timing
  lastUpdateTime = millis();
  
  // センサー初期化完了の表示
  M5.Display.fillScreen(TFT_NAVY);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(25, 50);
  M5.Display.println("Initialization");
  M5.Display.setCursor(40, 65);
  M5.Display.println("Complete");
  delay(1000);
}

void loop() {
  // Get current time
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastUpdateTime;
  lastUpdateTime = currentTime;
  
  // Update button state - 最優先で処理
  M5.update();
  
  // Handle button presses - ボタン処理を最優先
  handleButtonPress();
  
  // Read sensor data and update fusion
  readGPS();
  readIMU();
  
  // M5Unifiedを使用しているため、独自のセンサーフュージョンは不要
  // imuFusion.update(deltaTime);
  
  // Calculate celestial positions
  if (gpsValid) {
    calculateCelestialPositions();
  }
  
  // LCD更新は一定間隔で実行（ちらつき軽減と応答性のバランス）
  static unsigned long lastDisplayTime = 0;
  if (currentTime - lastDisplayTime >= UPDATE_INTERVAL) {
    lastDisplayTime = currentTime;
    
    // LCD更新
    updateDisplay();
    
    // 遅延を短くして応答性を向上
    delay(1);
  }
}

void setupHardware() {
  // Initialize AtomS3R
  auto cfg = M5.config();
  cfg.serial_baudrate = SERIAL_BAUD;  // Set serial communication baud rate
  cfg.clear_display = true;           // Clear display
  cfg.output_power = true;            // Enable power output
  cfg.led_brightness = 20;            // Set LED brightness (0-255)
  M5.begin(cfg);                      // Initialize M5Unified
  
  // Adjust LCD brightness (reduce flickering)
  M5.Display.setBrightness(40);       // Set LCD brightness to 40%
  
  // Optimize LCD settings (reduce flickering)
  M5.Display.setColorDepth(16);       // Set color depth to 16-bit
  M5.Display.setSwapBytes(true);      // Swap byte order
  
  // Initialize SPIFFS
  if (!initSPIFFS()) {
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(5, 50);
    M5.Display.println("SPIFFS Error!");
    delay(2000);
  } else {
    // デバッグ用：ファイル一覧を表示
    listSPIFFSFiles();
  }
  
  // Confirm LCD initialization
  M5.Display.fillScreen(TFT_NAVY);    // Set background color to navy
  M5.Display.setTextColor(TFT_WHITE); // Set text color to white
  M5.Display.setTextSize(1);          // Set text size to 1
  M5.Display.setCursor(10, 50);       // Set cursor position
  M5.Display.println("Initializing...");
  delay(1000);                        // Display for 1 second
  
  Serial.println("Polaris Navigator initializing...");
}

void setupIMU() {
  // デバッグ表示
  M5.Display.fillScreen(TFT_NAVY);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 10);
  M5.Display.println("IMU Setup...");
  
  // M5Unifiedライブラリを使用してIMUを初期化
  bool initResult = M5.Imu.init();
  Serial.print("IMU init result: ");
  Serial.println(initResult ? "Success" : "Failed");
  
  // センサー初期化チェック
  int imuType = M5.Imu.getType();
  Serial.print("IMU Type: ");
  Serial.println(imuType);
  
  if(!imuType){
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("IMU Init Failed!");
    Serial.println("IMU initialization failed! No IMU detected.");
    
    // 再試行
    Serial.println("Retrying IMU initialization...");
    delay(500);
    initResult = M5.Imu.init();
    imuType = M5.Imu.getType();
    
    if(!imuType) {
      Serial.println("IMU retry failed. Check hardware connections.");
      M5.Display.println("Check hardware!");
      delay(2000);
      return;
    } else {
      Serial.println("IMU retry successful!");
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.println("IMU Retry OK!");
    }
  }
  
  // センサー範囲設定
  // M5.Imu.setAccelRange(BMI2_16G);      // 加速度計レンジ
  // M5.Imu.setGyroRange(BMI2_2000_DPS);  // ジャイロレンジ
  // M5.Imu.setMagOpMode(BMM150_NORMAL_MODE); // 磁力計動作モード
  // 注: 磁力計の動作モードはbmm150.initialize()内で設定されます
  
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.println("IMU Initialized!");
  Serial.println("IMU initialized successfully using M5Unified");
  
  // キャリブレーション状態の表示
  M5.Display.setCursor(10, 40);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println("Calibration Status:");
  
  // キャリブレーションなしでも有効とする
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.println("Default Calibration");
  Serial.println("Using M5Unified with default calibration");
  imuCalibrated = true;  // キャリブレーションなしでも有効とする
  
  // IMUデータのテスト読み取り
  float acc[3], gyro[3], mag[3];
  
  // センサーデータを取得
  bool accOk = M5.Imu.getAccel(&acc[0], &acc[1], &acc[2]);    // 加速度 (g)
  bool gyroOk = M5.Imu.getGyro(&gyro[0], &gyro[1], &gyro[2]);  // 角速度 (dps)
  bool magOk = M5.Imu.getMag(&mag[0], &mag[1], &mag[2]);      // 地磁気 (μT)
  
  Serial.print("Accel read: ");
  Serial.println(accOk ? "OK" : "Failed");
  Serial.print("Gyro read: ");
  Serial.println(gyroOk ? "OK" : "Failed");
  Serial.print("Mag read: ");
  Serial.println(magOk ? "OK" : "Failed");
  
  M5.Display.setCursor(10, 70);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println("IMU Test Data:");
  M5.Display.setCursor(10, 80);
  M5.Display.printf("Acc: X%.2f Y%.2f Z%.2f", acc[0], acc[1], acc[2]);
  M5.Display.setCursor(10, 90);
  M5.Display.printf("Gyro: X%.1f Y%.1f Z%.1f", gyro[0], gyro[1], gyro[2]);
  M5.Display.setCursor(10, 100);
  M5.Display.printf("Mag: X%.1f Y%.1f Z%.1f", mag[0], mag[1], mag[2]);
  
  delay(2000);
  Serial.println("IMU setup complete");
}

void setupGPS() {
  // デバッグ表示
  M5.Display.fillScreen(TFT_NAVY);  // 濃い緑から紺色に変更
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 10);
  M5.Display.println("GPS Setup...");
  
  // Initialize GPS module
  M5.Display.setCursor(10, 30);
  M5.Display.println("Init GPS...");
  Serial.println("Initializing GPS with default pins (TX:5, RX:-1)");
  Serial.print("GPS_TX_PIN: ");
  Serial.println(GPS_TX_PIN);
  Serial.print("GPS_RX_PIN: ");
  Serial.println(GPS_RX_PIN);
  
  // AtomicBaseGPSクラスのデフォルトピン設定を使用
  bool gpsResult = gps.begin(GPS_BAUD);
  if (gpsResult) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("GPS OK");
    Serial.println("GPS initialized");
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("GPS Failed!");
    Serial.println("Failed to initialize GPS!");
    
    // 再試行
    Serial.println("Retrying GPS initialization...");
    delay(500);
    gpsResult = gps.begin(GPS_BAUD);
    if (gpsResult) {
      M5.Display.setTextColor(TFT_GREEN);
      M5.Display.println("GPS Retry OK");
      Serial.println("GPS retry successful");
    } else {
      M5.Display.setTextColor(TFT_RED);
      M5.Display.println("GPS Retry Failed!");
      Serial.println("GPS retry failed. Check hardware connections.");
    }
  }
  
  // Check Serial2 status
  Serial.print("Serial2 available: ");
  Serial.println(Serial2 ? "Yes" : "No");
  
  // Wait for initial GPS data
  Serial.println("Waiting for initial GPS data...");
  M5.Display.setCursor(10, 50);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println("Waiting for GPS data...");
  
  // Try to get initial GPS data with timeout
  unsigned long startTime = millis();
  bool initialDataReceived = false;
  
  while (millis() - startTime < 5000) { // 5秒間待機
    gps.update();
    if (Serial2.available() > 0) {
      initialDataReceived = true;
      break;
    }
    delay(100);
  }
  
  if (initialDataReceived) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("GPS data received");
    Serial.println("Initial GPS data received");
  } else {
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.println("No GPS data yet");
    Serial.println("No initial GPS data received within timeout");
  }
  
  // GPSが初期化できなくても、アプリケーションは続行できるようにする
  delay(1000);
}

void readGPS() {
  // Update GPS data
  gps.update();
  
  // Check if GPS data is valid
  bool currentGpsValid = gps.isValid();
  
  // GPS信号が有効な場合
  if (currentGpsValid) {
    // GPS信号が無効→有効に変わった場合、または60分経過した場合にデータを更新
    unsigned long currentTime = millis();
    bool shouldUpdate = !wasGpsValid || (currentTime - lastGpsUpdate >= 60 * 60 * 1000);
    
    // millis()のオーバーフロー対策
    if (currentTime < lastGpsUpdate) {
      shouldUpdate = true;
    }
    
    if (shouldUpdate) {
      // 最終更新時刻を更新
      lastGpsUpdate = currentTime;
      
      // GPS変数を更新
      latitude = gps.getLatitude();
      longitude = gps.getLongitude();
      altitude = gps.getAltitude();
      satellites = gps.getSatellites();
      hdop = gps.getHDOP();
      
      // GPSから時刻を更新（利用可能な場合）
      if (gps.getTime(&hour, &minute, &second) && 
          gps.getDate(&year, &month, &day)) {
        timeValid = true;
        
        // デバッグ用時刻出力
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
        
        // 有効なGPSデータをGPSDataManagerに保存
        GPSData data;
        data.latitude = latitude;
        data.longitude = longitude;
        data.altitude = altitude;
        data.satellites = satellites;
        data.hdop = hdop;
        data.year = year;
        data.month = month;
        data.day = day;
        data.hour = hour;
        data.minute = minute;
        data.second = second;
        
        // データを保存
        gpsDataManager.saveGPSData(data);
        
        // 更新理由をデバッグ出力
        if (!wasGpsValid) {
          Serial.println("GPS data updated: Signal newly acquired");
        } else {
          Serial.println("GPS data updated: 60-minute interval");
        }
      }
      
      // デバッグ出力
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
    } else {
      // 更新しない場合のデバッグ出力
      Serial.println("Using current GPS data (update not needed)");
    }
    
    // GPS有効フラグを更新
    gpsValid = true;
    wasGpsValid = true;
  } 
  // GPS信号が無効で、保存されたデータがある場合
  else if (!currentGpsValid && gpsDataManager.hasStoredData()) {
    // 保存されたGPSデータを読み込む
    GPSData savedData;
    if (gpsDataManager.loadGPSData(savedData)) {
      // 保存されたデータで変数を更新
      latitude = savedData.latitude;
      longitude = savedData.longitude;
      altitude = savedData.altitude;
      satellites = savedData.satellites;
      hdop = savedData.hdop;
      
      year = savedData.year;
      month = savedData.month;
      day = savedData.day;
      hour = savedData.hour;
      minute = savedData.minute;
      second = savedData.second;
      
      // 保存されたデータを使用していることを示すデバッグ出力
      Serial.println("Using saved GPS data:");
      Serial.print("Location: ");
      Serial.print(latitude, 6);
      Serial.print(", ");
      Serial.println(longitude, 6);
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
      
      // GPS有効フラグとタイム有効フラグを更新
      gpsValid = true;
      timeValid = true;
    }
    
    // GPS無効フラグを更新
    wasGpsValid = false;
  }
  // GPS信号が無効で、保存されたデータもない場合
  else {
    // GPS無効フラグを設定
    gpsValid = false;
    wasGpsValid = false;
    
    // デバッグ出力
    Serial.println("No GPS signal and no saved data available");
  }
}

void readIMU() {
  // M5Unifiedライブラリを使用してIMUデータを取得
  float acc[3], gyro[3], mag[3];
  bool accOk = false, gyroOk = false, magOk = false;
  
  // センサーデータを取得
  accOk = M5.Imu.getAccel(&acc[0], &acc[1], &acc[2]);    // 加速度 (g)
  gyroOk = M5.Imu.getGyro(&gyro[0], &gyro[1], &gyro[2]);  // 角速度 (dps)
  magOk = M5.Imu.getMag(&mag[0], &mag[1], &mag[2]);      // 地磁気 (μT)
  
  // 定期的にセンサー状態をレポート
  static unsigned long lastSensorReport = 0;
  if (millis() - lastSensorReport > 10000) {  // 10秒ごと
    Serial.println("IMU Sensor Status:");
    Serial.print("Accelerometer: ");
    Serial.println(accOk ? "OK" : "Failed");
    Serial.print("Gyroscope: ");
    Serial.println(gyroOk ? "OK" : "Failed");
    Serial.print("Magnetometer: ");
    Serial.println(magOk ? "OK" : "Failed");
    lastSensorReport = millis();
  }
  
  // ピッチとロールの計算
  // 重力ベクトルからピッチとロールを計算
  if (accOk) {
    pitch = atan2(acc[0], sqrt(acc[1] * acc[1] + acc[2] * acc[2])) * 180.0 / PI;
    roll = atan2(acc[1], acc[2]) * 180.0 / PI;
  }
  
  // 方位角の計算（ティルト補正あり）
  if (accOk && magOk) {
    // ピッチとロールを考慮した方位角の計算
    float pitch_rad = pitch * PI / 180.0;
    float roll_rad = roll * PI / 180.0;
    
    // 地磁気データをティルト補正
    float mag_x = mag[0] * cos(pitch_rad) + mag[2] * sin(pitch_rad);
    float mag_y = mag[0] * sin(roll_rad) * sin(pitch_rad) + mag[1] * cos(roll_rad) - mag[2] * sin(roll_rad) * cos(pitch_rad);
    
    // 補正後の地磁気データから方位角を計算
    heading = atan2(mag_y, mag_x) * 180.0 / PI;
    
    // 0-360度の範囲に変換
    if (heading < 0) {
      heading += 360.0;
    }
    
    // 異常値チェック
    static float lastValidHeading = heading;
    if (isnan(heading) || heading < 0 || heading > 360) {
      Serial.println("Warning: Invalid heading detected, using last valid value");
      heading = lastValidHeading;
    } else {
      // 有効な値の場合は保存
      lastValidHeading = heading;
    }
    
    // 簡単なローパスフィルタを適用して急激な変化を抑制
    static float filteredHeading = heading;
    filteredHeading = 0.9 * filteredHeading + 0.1 * heading;
    heading = filteredHeading;
  } else {
    // センサーデータが無効な場合は前回の値を維持
    Serial.println("Warning: Cannot calculate heading, sensor data unavailable");
  }
  
  // デバッグ出力
  Serial.print("Orientation: Heading=");
  Serial.print(heading);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
  
  // センサーデータが有効な場合のみ詳細情報を出力
  if (accOk) {
    Serial.print("Accel (g): X=");
    Serial.print(acc[0]);
    Serial.print(", Y=");
    Serial.print(acc[1]);
    Serial.print(", Z=");
    Serial.println(acc[2]);
  }
  
  if (gyroOk) {
    Serial.print("Gyro (dps): X=");
    Serial.print(gyro[0]);
    Serial.print(", Y=");
    Serial.print(gyro[1]);
    Serial.print(", Z=");
    Serial.println(gyro[2]);
  }
  
  if (magOk) {
    Serial.print("Mag (uT): X=");
    Serial.print(mag[0]);
    Serial.print(", Y=");
    Serial.print(mag[1]);
    Serial.print(", Z=");
    Serial.println(mag[2]);
  }
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
  // Static variable to record the last display mode
  static int lastMode = -1;
  static bool lastGpsValid = false;
  
  // Determine if display update is needed
  bool needUpdate = (currentMode != lastMode) || (gpsValid != lastGpsValid);
  
  // Record current state
  lastMode = currentMode;
  lastGpsValid = gpsValid;
  
  // Check IMU and GPS status
  bool imuDataAvailable = true;  // IMU data is always available
  
  // Update display based on current mode
  switch (currentMode) {
    case POLAR_ALIGNMENT:
      // Polar alignment mode - Display compass with Polaris position
      display.showPolarAlignment(heading, polarisAz, polarisAlt, pitch, roll);
      break;
    case CELESTIAL_DATA:  
      // Celestial mode - Display compass with sun/moon positions
      display.showCelestialOverlay(heading, pitch, roll, 
                                 sunAz, sunAlt, 
                                 moonAz, moonAlt, moonPhase);
      break;
    case GPS_DATA:  
      // GPS information mode - Display GPS coordinates and status
      if (gpsValid) {
        display.showGPS(latitude, longitude, altitude, satellites, hdop);
      } else {
        display.showGPSInvalid();
      }
      break;
    case RAW_DATA:
      // Raw data mode - Display raw sensor data
      if (rawDisplay.getCurrentMode() == RAW_IMU) {
        // IMUデータを強制的に再取得して表示を更新
        readIMU();
      }
      rawDisplay.update(currentRawMode);  
      break;
    case IMU_DATA:
      // IMU data mode - Display detailed IMU sensor data
      display.showIMU();
      break;
    case CALIBRATION_MODE:
      // Calibration mode - handled by main loop
      break;
  }
  
  // Add a small delay after updating display to reduce flickering
  delay(1);
  
  // Output debug information to serial
  Serial.print("Display Mode: ");
  Serial.print(currentMode);
  Serial.print(", IMU Valid: ");
  Serial.print(imuDataAvailable ? "Yes" : "No");
  Serial.print(", GPS Valid: ");
  Serial.println(gpsValid ? "Yes" : "No");
}

void handleButtonPress() {
  // Handle button press
  if (M5.BtnA.wasPressed()) {
    // Short press - cycle through display modes or sub-modes
    if (currentMode == RAW_DATA) {
      // RAW_DATAモード内ではサブモードを切り替え
      cycleRawDataMode();
    } else {
      // 他のモードでは通常のモード切り替え
      cycleDisplayMode();
    }
  } else if (M5.BtnA.pressedFor(1000)) {
    // Long press - perform mode-specific action
    handleLongPress();
  }
}

void cycleDisplayMode() {
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
      currentMode = RAW_DATA;
      // RAW_DATAモードに入る時は、サブモードをRAW_IMUに初期化
      currentRawMode = RAW_IMU;
      break;
    case RAW_DATA: 
      currentMode = CALIBRATION_MODE;
      break;
    case CALIBRATION_MODE: 
      currentMode = POLAR_ALIGNMENT;
      break;
  }
  
  // Update display
  updateDisplay();
}

void handleLongPress() {
  // Handle long press based on current mode
  switch (currentMode) {
    case POLAR_ALIGNMENT: 
      // Toggle detailed view
      // TODO: Implement detailed view toggle
      break;
      
    case GPS_DATA: 
      // Force GPS refresh
      // TODO: Implement GPS refresh
      break;
      
    case IMU_DATA: 
      // Start IMU calibration
      currentMode = CALIBRATION_MODE;
      calibrateIMU();
      // Return to IMU data mode after calibration
      currentMode = IMU_DATA;
      break;
      
    case CELESTIAL_DATA: 
      // Toggle between sun and moon focus
      // TODO: Implement celestial focus toggle
      break;
      
    case RAW_DATA: 
      // RAW_DATAモードでの長押しはメインモードを切り替え
      currentMode = CALIBRATION_MODE;
      updateDisplay();
      break;
      
    case CALIBRATION_MODE: 
      // Start/stop calibration process
      if (calibrationManager.isCalibrating()) {
        // Cancel ongoing calibration
        calibrationManager.cancelCalibration();
        Serial.println("Calibration cancelled");
      } else {
        // Start calibration
        calibrateIMU();
      }
      break;
  }
}

void cycleRawDataMode() {
  // Cycle through raw data modes
  switch (currentRawMode) {
    case RAW_IMU: 
      currentRawMode = RAW_GPS; 
      break;
    case RAW_GPS: 
      currentRawMode = RAW_CELESTIAL; 
      break;
    case RAW_CELESTIAL: 
      currentRawMode = RAW_SYSTEM; 
      break;
    case RAW_SYSTEM: 
      currentRawMode = DISPLAY_DEBUG; 
      break;
    case DISPLAY_DEBUG: 
      // DEBUGモードの後は直接初期画面に戻る
      currentMode = POLAR_ALIGNMENT;  // 直接初期画面に戻る
      updateDisplay();
      return;  // 以降の処理をスキップ
  }
  
  // Update display with new raw data mode
  updateDisplay();
  
  // Provide feedback
  Serial.print("Raw data mode: ");
  switch (currentRawMode) {
    case RAW_IMU: 
      Serial.println("IMU");
      break;
    case RAW_GPS: 
      Serial.println("GPS");
      break;
    case RAW_CELESTIAL: 
      Serial.println("CELESTIAL");
      break;
    case RAW_SYSTEM: 
      Serial.println("SYSTEM");
      break;
    case DISPLAY_DEBUG: 
      Serial.println("DEBUG");
      break;
  }
}

void calibrateIMU() {
  // M5Unifiedライブラリを使用してIMUキャリブレーションを実行
  Serial.println("Starting IMU calibration with M5Unified...");
  
  // ディスプレイにキャリブレーションモードを表示
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.5);
  M5.Display.setCursor(0, 0);
  M5.Display.println("IMU Calibration");
  M5.Display.println("Rotate device in all");
  M5.Display.println("directions to calibrate");
  M5.Display.println("magnetometer (8の字)");
  M5.Display.println("");
  M5.Display.println("Press button to cancel");
  
  // キャリブレーション開始
  Serial.println("Starting accelerometer/gyro calibration...");
  M5.Display.setCursor(0, 80);
  M5.Display.println("Step 1: Accel/Gyro");
  M5.Display.println("Place device on flat");
  M5.Display.println("surface and keep still");
  
  // 加速度/ジャイロキャリブレーション
  // M5.Imu.calibrateAccelGyro(); // 現在このメソッドは利用できません
  delay(1000);
  
  // 磁力計キャリブレーション
  Serial.println("Starting magnetometer calibration...");
  M5.Display.fillRect(0, 80, 160, 60, TFT_BLACK);
  M5.Display.setCursor(0, 80);
  M5.Display.println("Step 2: Magnetometer");
  M5.Display.println("Draw figure 8 pattern");
  M5.Display.println("with device");
  
  // キャリブレーションループ
  bool calibrationComplete = false;
  unsigned long startTime = millis();
  unsigned long calibrationDuration = 15000; // 15秒間のキャリブレーション
  
  // プログレスバーの初期設定
  int barWidth = 140;
  int barHeight = 10;
  int barX = 10;
  int barY = 120;
  M5.Display.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  
  while (!calibrationComplete) {
    // 磁力計キャリブレーション実行
    imuFusion.calibrateMagnetometer();
    
    // 経過時間からプログレスを計算
    unsigned long elapsedTime = millis() - startTime;
    float progress = min(1.0f, (float)elapsedTime / calibrationDuration);
    
    // プログレスバーを更新
    int fillWidth = (int)(progress * barWidth);
    M5.Display.fillRect(barX + 1, barY + 1, fillWidth - 2, barHeight - 2, TFT_GREEN);
    
    // キャリブレーション完了判定
    if (progress >= 1.0) {
      calibrationComplete = true;
    }
    
    // ボタン入力をチェック（キャンセル用）
    M5.update();
    if (M5.BtnA.wasPressed()) {
      // キャリブレーションをキャンセル
      Serial.println("Calibration cancelled by user");
      break;
    }
    
    // 少し待機
    delay(50);
  }
  
  // キャリブレーション完了表示
  M5.Display.fillRect(0, 80, 160, 60, TFT_BLACK);
  M5.Display.setCursor(0, 80);
  
  if (calibrationComplete) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("Calibration Complete!");
    Serial.println("IMU calibration complete");
    
    // キャリブレーションステータスを更新
    imuCalibrated = true;
    
    // キャリブレーションマネージャーとの連携
    // 注意: 現在のCalibrationManagerはBMI270とBMM150クラスに依存していて、
    // M5Unifiedライブラリに合わせて修正する必要があります
    // calibrationManager.saveCalibrationData();
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("Calibration Cancelled");
    Serial.println("IMU calibration cancelled");
  }
  
  // 少し待機してから終了
  delay(2000);
}
