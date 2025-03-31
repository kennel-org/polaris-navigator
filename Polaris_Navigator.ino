/*
 * Polaris Navigator
 * 
 * A polar alignment assistant device for astrophotography
 * using AtomS3R (with IMU) and AtomicBase GPS.
 * 
 * Created: 2025-03-23
 * GitHub: https://github.com/kennel-org/polaris-navigator
 * 
 * AtomS3R IMU座標系の定義:
 * - X軸: デバイスの右方向（USBコネクタが下向きの場合）
 * - Y軸: デバイスの上方向（USBコネクタが下向きの場合）
 * - Z軸: デバイスの画面から外向き（画面に垂直）
 * 
 * 極軸合わせ時の標準的な持ち方:
 * - デバイスの上面（-X方向）を天の北極/南極に向ける
 * - ディスプレイ面を上向きにして読みやすくする
 * 
 * 角度の定義:
 * - ピッチ: X-Z平面での傾き（+/-90度）、0度が水平
 * - ロール: Y-Z平面での傾き（+/-180度）、0度が水平
 * - 方位角: 北を0度とした水平面内での角度（0-360度）
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
#include "src/StartupScreen.h"   // Startup screen

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
StartupScreen startupScreen;    // Startup screen object

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

// グローバル変数に前のモード情報を保存
DisplayMode previousMode = POLAR_ALIGNMENT;
RawDataMode previousRawMode = RAW_IMU;

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

// グローバル変数を追加
bool longPressHandled = false; // 長押し処理が実行済みかどうかのフラグ

void setup() {
  // Initialize hardware
  setupHardware();
  
  // Initialize startup screen
  startupScreen.begin();
  
  // Show splash screen
  startupScreen.showSplashScreen();
  
  // Wait for splash screen display
  delay(2000);
  
  // Initialize sensors
  setupIMU();
  setupGPS();
  display.begin();
  
  // Initialize other components
  rawDisplay.begin();
  calibrationManager.begin();
  settingsManager.begin();
  settingsMenu.begin();
  gpsDataManager.begin();
  
  // Initialize timing
  lastUpdateTime = millis();
  
  // Show initialization complete
  startupScreen.showInitComplete();
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
  
  // 画面の初期化のみを行う（背景色は設定しない）
  M5.Display.clear();
  
  // 下部18ピクセルに2行だけで表示
  M5.Display.fillRect(0, M5.Display.height() - 18, M5.Display.width(), 18, TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.0);
  M5.Display.setCursor(5, M5.Display.height() - 18);
  M5.Display.println("Polaris Navigator");
  M5.Display.setCursor(5, M5.Display.height() - 8);
  M5.Display.println("Initializing...");
  
  Serial.println("Polaris Navigator initializing...");
}

void setupIMU() {
  // 初期化状態を表示（StartupScreenクラスを使用）
  startupScreen.showInitProgress("IMU Init...", 30);
  
  // M5Unifiedライブラリを使用してIMUを初期化
  bool initResult = M5.Imu.init();
  Serial.print("IMU init result: ");
  Serial.println(initResult ? "Success" : "Failed");
  
  // センサー初期化チェック
  int imuType = M5.Imu.getType();
  Serial.print("IMU Type: ");
  Serial.println(imuType);
  
  if(!imuType){
    // 初期化失敗の場合
    startupScreen.showInitError("IMU Init Failed!");
    
    Serial.println("IMU initialization failed! No IMU detected.");
    
    // 再試行
    Serial.println("Retrying IMU initialization...");
    delay(500);
    initResult = M5.Imu.init();
    imuType = M5.Imu.getType();
    
    if(!imuType) {
      Serial.println("IMU retry failed. Check hardware connections.");
      startupScreen.showInitError("IMU Failed!");
      delay(1000);
      return;
    }
  }
  
  // センサー初期化成功
  startupScreen.showInitProgress("IMU OK", 50);
  Serial.println("IMU initialized successfully");
}

void setupGPS() {
  // 初期化状態を表示（StartupScreenクラスを使用）
  startupScreen.showInitProgress("GPS Init...", 60);
  
  // デバッグ情報をシリアルに出力
  Serial.println("Initializing GPS with default pins (TX:5, RX:-1)");
  Serial.print("GPS_TX_PIN: ");
  Serial.println(GPS_TX_PIN);
  Serial.print("GPS_RX_PIN: ");
  Serial.println(GPS_RX_PIN);
  
  // AtomicBaseGPSクラスのデフォルトピン設定を使用
  bool gpsResult = gps.begin(GPS_BAUD);
  if (gpsResult) {
    startupScreen.showInitProgress("GPS OK", 70);
    Serial.println("GPS initialized");
  } else {
    startupScreen.showInitProgress("GPS Failed, Retry...", 65);
    Serial.println("Failed to initialize GPS!");
    
    // 再試行
    Serial.println("Retrying GPS initialization...");
    delay(500);
    gpsResult = gps.begin(GPS_BAUD);
    if (gpsResult) {
      startupScreen.showInitProgress("GPS Retry OK", 70);
      Serial.println("GPS retry successful");
    } else {
      startupScreen.showInitError("GPS Failed");
      Serial.println("GPS retry failed. Check hardware connections.");
      delay(1000);
      return;
    }
  }
  
  // Check Serial2 status
  Serial.print("Serial2 available: ");
  Serial.println(Serial2 ? "Yes" : "No");
  
  // Wait for initial GPS data
  Serial.println("Waiting for initial GPS data...");
  startupScreen.showInitProgress("Waiting for GPS...", 75);
  
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
    startupScreen.showInitProgress("GPS Signal OK", 80);
    Serial.println("Initial GPS data received");
  } else {
    startupScreen.showInitProgress("No GPS Signal", 80);
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
  
  // AtomS3R IMU座標系に合わせて軸を調整
  // 注: AtomS3Rの物理的な向きに合わせて、軸の対応関係を調整
  //     X軸: デバイスの右方向
  //     Y軸: デバイスの上方向
  //     Z軸: デバイスの画面から外向き
  
  // 極軸合わせ用の座標系に変換
  // 極軸合わせでは、デバイスの上面（-X方向）を天の北極/南極に向ける
  float acc_adj[3], mag_adj[3];
  
  // 加速度データの軸調整（軸の入れ替えと符号の反転）
  // 問題: デバイスをロール方向に傾けるとピッチが変化する
  // 解決: 軸の対応関係を修正
  acc_adj[0] = acc[1];  // X軸をY軸に変更（デバイスの上方向を右方向と再定義）
  acc_adj[1] = -acc[0]; // Y軸を-X軸に変更（デバイスの右方向を下方向と再定義）
  acc_adj[2] = acc[2];  // Z軸はそのまま（画面垂直方向）
  
  // 地磁気データの軸調整（加速度と同様の調整）
  mag_adj[0] = mag[1];  // X軸をY軸に変更
  mag_adj[1] = -mag[0]; // Y軸を-X軸に変更
  mag_adj[2] = mag[2];  // Z軸はそのまま
  
  // ピッチとロールの計算
  // 重力ベクトルからピッチとロールを計算
  if (accOk) {
    // ピッチ: X-Z平面での傾き（+/-90度）
    // 修正: 調整後のX軸を基準にした傾き角度
    pitch = atan2(acc_adj[0], sqrt(acc_adj[1] * acc_adj[1] + acc_adj[2] * acc_adj[2])) * 180.0 / PI;
    
    // ロール: Y-Z平面での傾き（+/-180度）
    // 修正: 調整後のY軸とZ軸の関係から計算
    roll = atan2(acc_adj[1], acc_adj[2]) * 180.0 / PI;
    
    // デバッグ: 生のセンサー値と計算された角度の関係を確認
    Serial.print("Raw Accel vs Angles: ");
    Serial.print("X="); Serial.print(acc[0]);
    Serial.print(", Y="); Serial.print(acc[1]);
    Serial.print(", Z="); Serial.print(acc[2]);
    Serial.print(" -> Pitch="); Serial.print(pitch);
    Serial.print(", Roll="); Serial.println(roll);
  }
  
  // 方位角の計算（ティルト補正あり）
  if (accOk && magOk) {
    // ピッチとロールを考慮した方位角の計算
    float pitch_rad = pitch * PI / 180.0;
    float roll_rad = roll * PI / 180.0;
    
    // 地磁気データをティルト補正
    // 水平面に投影するための補正計算
    float mag_x = mag_adj[0] * cos(pitch_rad) + mag_adj[2] * sin(pitch_rad);
    float mag_y = mag_adj[0] * sin(roll_rad) * sin(pitch_rad) + 
                 mag_adj[1] * cos(roll_rad) - 
                 mag_adj[2] * sin(roll_rad) * cos(pitch_rad);
    
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
    
    Serial.print("Accel Adjusted: X=");
    Serial.print(acc_adj[0]);
    Serial.print(", Y=");
    Serial.print(acc_adj[1]);
    Serial.print(", Z=");
    Serial.println(acc_adj[2]);
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
    
    Serial.print("Mag Adjusted: X=");
    Serial.print(mag_adj[0]);
    Serial.print(", Y=");
    Serial.print(mag_adj[1]);
    Serial.print(", Z=");
    Serial.println(mag_adj[2]);
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
    // ボタン押下をデバッグ出力
    Serial.println("Button was pressed (short press)");
    
    // Short press - cycle through display modes or sub-modes
    if (currentMode == RAW_DATA) {
      // RAW_DATAモード内ではサブモードを切り替え
      Serial.println("Cycling RAW data mode");
      cycleRawDataMode();
    } else {
      // 他のモードでは通常のモード切り替え
      Serial.println("Cycling display mode");
      cycleDisplayMode();
    }
  } else if (M5.BtnA.pressedFor(1000)) {
    // Long press - perform mode-specific action
    if (!longPressHandled) {
      Serial.println("Long press detected and handling it now");
      handleLongPress();
      longPressHandled = true;
    } else {
      Serial.println("Long press already handled, ignoring");
    }
  } else {
    // ボタンが押されていない場合はフラグをリセット
    if (longPressHandled && !M5.BtnA.isPressed()) {
      Serial.println("Resetting longPressHandled flag");
      longPressHandled = false;
    }
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
  // デバッグ出力
  Serial.print("Long press detected in mode: ");
  Serial.println(currentMode);
  if (currentMode == RAW_DATA) {
    Serial.print("Current RAW mode: ");
    Serial.println(currentRawMode);
  }
  
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
      // 現在のモードを保存
      previousMode = currentMode;
      previousRawMode = currentRawMode;
      Serial.println("Saving previous mode: IMU_DATA");
      
      // Start IMU calibration
      currentMode = CALIBRATION_MODE;
      Serial.println("Entering CALIBRATION_MODE");
      updateDisplay(); // モード変更を画面に反映
      
      // キャリブレーション実行
      calibrateIMU();
      break;
      
    case CELESTIAL_DATA: 
      // Toggle between sun and moon focus
      // TODO: Implement celestial focus toggle
      break;
      
    case RAW_DATA: 
      // RAW_DATAモードでの長押しはキャリブレーションを開始
      // 前のRAWモードを記憶
      previousMode = currentMode;
      previousRawMode = currentRawMode;
      Serial.println("Saving previous mode: RAW_DATA, submode: " + String(currentRawMode));
      
      // キャリブレーションモードに設定
      currentMode = CALIBRATION_MODE;
      Serial.println("Entering CALIBRATION_MODE");
      updateDisplay(); // モード変更を画面に反映
      
      // キャリブレーション実行
      calibrateIMU();
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
  // デバッグ出力
  Serial.println("Starting calibrateIMU function");
  Serial.print("Current mode: ");
  Serial.println(currentMode);
  Serial.print("Previous mode: ");
  Serial.println(previousMode);
  Serial.print("Previous RAW mode: ");
  Serial.println(previousRawMode);
  
  // キャリブレーション開始
  Serial.println("Starting IMU calibration...");
  
  // ディスプレイにキャリブレーションモードを表示
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(0, 0);
  M5.Display.println("IMU Calibration");
  M5.Display.println("Rotate device in all");
  M5.Display.println("directions to calibrate");
  M5.Display.println("magnetometer (8の字)");
  M5.Display.println("");
  M5.Display.println("Press button to cancel");
  
  // 磁力計キャリブレーション
  M5.Display.setCursor(0, 80);
  M5.Display.println("Draw figure 8 pattern");
  M5.Display.println("with device");
  
  // キャリブレーション変数
  bool calibrationComplete = false;
  bool calibrationCancelled = false;
  unsigned long startTime = millis();
  unsigned long calibrationDuration = 15000; // 15秒間のキャリブレーション
  
  // キャリブレーション用データ
  int16_t min_x = 32767, max_x = -32768;
  int16_t min_y = 32767, max_y = -32768;
  int16_t min_z = 32767, max_z = -32768;
  int sampleCount = 0;
  
  // ボタン状態をリセット - 重要: 前の状態をクリアする
  delay(500); // ボタン状態が安定するまで少し待機
  M5.update();
  
  // キャリブレーションループ
  while (!calibrationComplete && !calibrationCancelled) {
    // 現在の経過時間を計算
    unsigned long elapsedTime = millis() - startTime;
    float progress = min(1.0f, (float)elapsedTime / calibrationDuration);
    
    // 時間経過でキャリブレーション完了
    if (progress >= 1.0) {
      calibrationComplete = true;
      Serial.println("Calibration completed by timeout");
      continue; // ループを抜ける
    }
    
    // ボタン入力をチェック（キャンセル用）
    M5.update();
    
    // ボタンの状態をデバッグ出力
    static bool lastBtnState = false;
    bool currentBtnState = M5.BtnA.isPressed();
    
    if (currentBtnState != lastBtnState) {
      Serial.print("In calibration loop, button state changed to: ");
      Serial.println(currentBtnState ? "PRESSED" : "RELEASED");
      lastBtnState = currentBtnState;
    }
    
    if (M5.BtnA.wasPressed()) {
      // キャリブレーションをキャンセル
      Serial.println("Calibration cancelled by button press");
      calibrationCancelled = true;
      continue; // ループを抜ける
    }
    
    // 磁気センサーデータを読み取り
    bmm150.readMagnetometer();
    
    // キャリブレーションデータを更新
    if (bmm150.raw_mag_x < min_x) min_x = bmm150.raw_mag_x;
    if (bmm150.raw_mag_x > max_x) max_x = bmm150.raw_mag_x;
    
    if (bmm150.raw_mag_y < min_y) min_y = bmm150.raw_mag_y;
    if (bmm150.raw_mag_y > max_y) max_y = bmm150.raw_mag_y;
    
    if (bmm150.raw_mag_z < min_z) min_z = bmm150.raw_mag_z;
    if (bmm150.raw_mag_z > max_z) max_z = bmm150.raw_mag_z;
    
    sampleCount++;
    
    // 進捗バーを表示
    int barWidth = 120;
    int barHeight = 10;
    int barX = 20;
    int barY = 120;
    
    M5.Display.fillRect(barX, barY, barWidth, barHeight, TFT_DARKGREY);
    M5.Display.fillRect(barX, barY, (int)(barWidth * progress), barHeight, TFT_GREEN);
    
    // 進捗テキスト表示
    M5.Display.fillRect(0, 140, 160, 10, TFT_BLACK);
    M5.Display.setCursor(20, 140);
    M5.Display.print((int)(progress * 100));
    M5.Display.print("% ");
    M5.Display.print(sampleCount);
    M5.Display.print(" samples");
    
    // 少し待機
    delay(50);
  }
  
  // キャリブレーション結果表示
  M5.Display.fillRect(0, 80, 160, 80, TFT_BLACK);
  M5.Display.setCursor(0, 80);
  
  if (calibrationComplete) {
    // キャリブレーション完了処理
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.println("Calibration Complete!");
    Serial.println("IMU calibration complete");
    
    // キャリブレーションデータを計算して適用
    float hard_iron_x = (min_x + max_x) / 2.0f;
    float hard_iron_y = (min_y + max_y) / 2.0f;
    float hard_iron_z = (min_z + max_z) / 2.0f;
    
    // スケーリング係数を計算
    float range_x = (max_x - min_x) / 2.0f;
    float range_y = (max_y - min_y) / 2.0f;
    float range_z = (max_z - min_z) / 2.0f;
    
    // ゼロ除算を防止
    if (range_x < 1.0f) range_x = 1.0f;
    if (range_y < 1.0f) range_y = 1.0f;
    if (range_z < 1.0f) range_z = 1.0f;
    
    // 平均範囲を計算
    float avg_range = (range_x + range_y + range_z) / 3.0f;
    
    // スケーリング係数
    float scale_x = avg_range / range_x;
    float scale_y = avg_range / range_y;
    float scale_z = avg_range / range_z;
    
    // BMM150クラスにキャリブレーションデータを設定
    bmm150.setCalibrationData(hard_iron_x, hard_iron_y, hard_iron_z, 
                             scale_x, scale_y, scale_z);
    
    // キャリブレーション完了フラグを設定
    bmm150.setCalibrationStatus(true);
    imuCalibrated = true;
    
    // デバッグ出力
    Serial.print("Samples collected: ");
    Serial.println(sampleCount);
  } else {
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.println("Calibration Cancelled");
    Serial.println("IMU calibration cancelled");
  }
  
  // 少し待機してから終了
  delay(2000);
  
  // 明示的に前のモードに戻す
  Serial.println("Returning to previous mode");
  Serial.print("Previous mode: ");
  Serial.println(previousMode);
  Serial.print("Previous RAW mode: ");
  Serial.println(previousRawMode);
  
  // モードを元に戻す
  currentMode = previousMode;
  currentRawMode = previousRawMode;
  
  // 長押し処理フラグをリセット
  longPressHandled = false;
  
  // ボタン状態をリセット
  M5.update();
  
  // 画面を更新して前のモードの表示に戻す
  Serial.println("Updating display to show previous mode");
  updateDisplay();
  
  // 追加の安全策として、ボタン状態が完全にリセットされるまで待機
  delay(500);
  M5.update();
  Serial.println("Calibration function completed");
}

void loop() {
  // Get current time
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastUpdateTime;
  lastUpdateTime = currentTime;
  
  // Update button state - 最優先で処理
  M5.update();
  
  // ボタン状態をデバッグ出力
  static bool prevBtnPressed = false;
  static bool prevBtnLongPressed = false;
  
  bool btnPressed = M5.BtnA.isPressed();
  bool btnLongPressed = M5.BtnA.pressedFor(1000);
  
  // ボタン状態が変化した場合のみ出力
  if (btnPressed != prevBtnPressed) {
    Serial.print("Button state changed: ");
    Serial.println(btnPressed ? "PRESSED" : "RELEASED");
    prevBtnPressed = btnPressed;
  }
  
  if (btnLongPressed != prevBtnLongPressed) {
    Serial.print("Long press state: ");
    Serial.println(btnLongPressed ? "DETECTED" : "RELEASED");
    prevBtnLongPressed = btnLongPressed;
  }
  
  // モード情報を定期的に出力
  static unsigned long lastDebugTime = 0;
  if (currentTime - lastDebugTime >= 1000) { // 1秒ごとに出力
    lastDebugTime = currentTime;
    Serial.print("Current mode: ");
    Serial.print(currentMode);
    if (currentMode == RAW_DATA) {
      Serial.print(", Raw submode: ");
      Serial.print(currentRawMode);
    }
    Serial.print(", longPressHandled: ");
    Serial.println(longPressHandled);
  }
  
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
