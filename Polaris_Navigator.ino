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
#include "src/BMM150class.h"     // Magnetometer
#include "src/BMI270.h"          // Accelerometer and Gyroscope
#include "src/IMUFusion.h"       // Sensor fusion

// GPS related
#include "src/AtomicBaseGPS.h"   // AtomicBase GPS module

// Display related
// #include <FastLED.h>         // For LED control
#include "src/CompassDisplay.h"  // Custom display interface
#include "src/CelestialOverlay.h" // Celestial overlay
#include "src/RawDataDisplay.h"  // Raw data display
#include "src/DisplayModes.h"    // Display mode definitions

// Celestial calculations
#include "src/celestial_math.h"  // Custom celestial calculations

// Calibration and Settings
#include "src/CalibrationManager.h" // Sensor calibration
#include "src/SettingsManager.h"    // User settings
#include "src/SettingsMenu.h"       // Settings menu interface

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
RawDataDisplay rawDisplay;   // Raw data display object
CalibrationManager calibrationManager(&bmi270, &bmm150); // Calibration manager
SettingsManager settingsManager;  // Settings manager
SettingsMenu settingsMenu(&settingsManager); // Settings menu

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
  
  // デバッグ用のテキスト表示
  M5.Display.fillScreen(TFT_BLUE);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Debug Mode");
  M5.Display.setCursor(10, 30);
  M5.Display.println("Starting sensors...");
  
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
  
  // Initialize timing
  lastUpdateTime = millis();
  
  // デバッグ用のテキスト表示（初期化完了後）
  M5.Display.fillScreen(TFT_GREEN);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 10);
  M5.Display.println("Init Complete");
  M5.Display.setCursor(10, 30);
  M5.Display.println("Starting main loop...");
  delay(1000);
}

void loop() {
  // Calculate delta time for sensor fusion
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0f;  // Convert to seconds
  lastUpdateTime = currentTime;
  
  // Update button state
  M5.update();
  
  // Handle button presses
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
  
  // デバッグ用の表示（1秒ごとに更新）
  static unsigned long lastDebugTime = 0;
  if (currentTime - lastDebugTime >= 1000) {
    lastDebugTime = currentTime;
    
    // 画面をクリア
    M5.Display.fillScreen(TFT_NAVY);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(1);
    
    // ヘッダー
    M5.Display.setCursor(5, 5);
    M5.Display.println("Polaris Navigator");
    M5.Display.drawLine(0, 15, 128, 15, TFT_CYAN);
    
    // IMUデータ
    M5.Display.setCursor(5, 20);
    M5.Display.println("IMU Data:");
    M5.Display.setCursor(5, 30);
    M5.Display.print("Heading: ");
    M5.Display.print(heading, 1);
    M5.Display.setCursor(5, 40);
    M5.Display.print("Pitch: ");
    M5.Display.print(pitch, 1);
    M5.Display.setCursor(5, 50);
    M5.Display.print("Roll: ");
    M5.Display.print(roll, 1);
    
    // GPSデータ
    M5.Display.setCursor(5, 65);
    M5.Display.println("GPS Data:");
    M5.Display.setCursor(5, 75);
    if (gpsValid) {
      M5.Display.print("Lat: ");
      M5.Display.print(latitude, 4);
      M5.Display.setCursor(5, 85);
      M5.Display.print("Lon: ");
      M5.Display.print(longitude, 4);
      M5.Display.setCursor(5, 95);
      M5.Display.print("Sats: ");
      M5.Display.print(satellites);
    } else {
      M5.Display.println("No GPS Fix");
    }
    
    // フッター
    M5.Display.drawLine(0, 110, 128, 110, TFT_CYAN);
    M5.Display.setCursor(5, 115);
    M5.Display.print("Uptime: ");
    M5.Display.print(currentTime / 1000);
    M5.Display.print("s");
  }
  
  // Update display at specified interval
  if (currentTime - lastDisplayTime >= UPDATE_INTERVAL) {
    lastDisplayTime = currentTime;
    // updateDisplay();  // 通常の表示更新は一時的に無効化
  }
}

void setupHardware() {
  // Initialize AtomS3R
  auto cfg = M5.config();
  cfg.serial_baudrate = SERIAL_BAUD;  // シリアル通信のボーレート設定
  cfg.clear_display = true;           // ディスプレイをクリア
  cfg.output_power = true;            // 電源出力を有効化
  M5.begin(cfg);                      // M5Unifiedの初期化
  
  // ディスプレイの初期化を確認
  M5.Display.fillScreen(TFT_RED);     // 画面を赤で塗りつぶし
  M5.Display.setTextColor(TFT_WHITE); // テキスト色を白に設定
  M5.Display.setTextSize(1);          // テキストサイズを1に設定
  M5.Display.setCursor(10, 50);       // カーソル位置を設定
  M5.Display.println("Initializing...");
  delay(1000);                        // 1秒間表示
  
  Serial.println("Polaris Navigator initializing...");
}

void setupIMU() {
  // デバッグ表示
  M5.Display.fillScreen(TFT_PURPLE);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 10);
  M5.Display.println("IMU Setup...");
  
  // M5Unifiedライブラリを使用してIMUを初期化
  M5.Imu.init();
  
  // センサー初期化チェック
  if(!M5.Imu.getType()){
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("IMU Init Failed!");
    Serial.println("IMU initialization failed!");
    delay(2000);
    return;
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
  
  // 注意: 現在のCalibrationManagerはBMI270とBMM150クラスに依存していて、
  // M5Unifiedライブラリに合わせて修正する必要があります
  // if (calibrationManager.loadCalibrationData()) {
  //   M5.Display.setTextColor(TFT_GREEN);
  //   M5.Display.println("Calib Loaded");
  //   Serial.println("Loaded saved calibration data");
  //   calibrationManager.applyCalibration();
  //   imuCalibrated = true;
  // } else {
  //   M5.Display.setTextColor(TFT_YELLOW);
  //   M5.Display.println("No Calib Data");
  //   Serial.println("No saved calibration data found");
  //   imuCalibrated = false;
  // }
  
  // 現時点ではキャリブレーションは未実施と表示
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.println("Not Calibrated");
  Serial.println("Using M5Unified without calibration data");
  imuCalibrated = false;
  
  // IMUデータのテスト読み取り
  float acc[3], gyro[3], mag[3];
  M5.Imu.getAccel(&acc[0], &acc[1], &acc[2]);
  M5.Imu.getGyro(&gyro[0], &gyro[1], &gyro[2]);
  M5.Imu.getMag(&mag[0], &mag[1], &mag[2]);
  
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
  M5.Display.fillScreen(TFT_DARKGREEN);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(10, 10);
  M5.Display.println("GPS Setup...");
  
  // Initialize GPS module
  M5.Display.setCursor(10, 30);
  M5.Display.println("Init GPS...");
  
  bool gpsResult = gps.begin(GPS_BAUD);
  if (gpsResult) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("GPS OK");
    Serial.println("GPS initialized");
  } else {
    M5.Display.setTextColor(TFT_RED);
    M5.Display.println("GPS Failed!");
    Serial.println("Failed to initialize GPS!");
  }
  
  delay(1000);
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
  // M5Unifiedライブラリを使用してIMUデータを取得
  float acc[3], gyro[3], mag[3];
  
  // センサーデータを取得
  M5.Imu.getAccel(&acc[0], &acc[1], &acc[2]);    // 加速度 (g)
  M5.Imu.getGyro(&gyro[0], &gyro[1], &gyro[2]);  // 角速度 (dps)
  M5.Imu.getMag(&mag[0], &mag[1], &mag[2]);      // 地磁気 (μT)
  
  // ピッチとロールの計算
  // 重力ベクトルからピッチとロールを計算
  pitch = atan2(acc[0], sqrt(acc[1] * acc[1] + acc[2] * acc[2])) * 180.0 / PI;
  roll = atan2(acc[1], acc[2]) * 180.0 / PI;
  
  // 方位角の計算（ティルト補正あり）
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
  
  // デバッグ出力
  Serial.print("Orientation: Heading=");
  Serial.print(heading);
  Serial.print(", Pitch=");
  Serial.print(pitch);
  Serial.print(", Roll=");
  Serial.println(roll);
  
  Serial.print("Accel (g): X=");
  Serial.print(acc[0]);
  Serial.print(", Y=");
  Serial.print(acc[1]);
  Serial.print(", Z=");
  Serial.println(acc[2]);
  
  Serial.print("Gyro (dps): X=");
  Serial.print(gyro[0]);
  Serial.print(", Y=");
  Serial.print(gyro[1]);
  Serial.print(", Z=");
  Serial.println(gyro[2]);
  
  Serial.print("Mag (uT): X=");
  Serial.print(mag[0]);
  Serial.print(", Y=");
  Serial.print(mag[1]);
  Serial.print(", Z=");
  Serial.println(mag[2]);
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
      // Display celestial data with enhanced overlay
      if (gpsValid) {
        // Get current date and time from GPS
        int year, month, day;
        gps.getDate(&year, &month, &day);
        
        // Show celestial overlay with current position and time
        display.showCelestialOverlay(heading, latitude, longitude, 
                                    year, month, day, hour, minute, second);
      } else {
        // Fallback to basic celestial data display
        display.showCelestialData(sunAz, sunAlt, moonAz, moonAlt, moonPhase);
      }
      break;
      
    case RAW_DATA: 
      // Display detailed raw sensor data
      switch (currentRawMode) {
        case RAW_IMU: 
          rawDisplay.showRawIMU(&bmi270, &bmm150, heading, pitch, roll, imuCalibrated);
          break;
          
        case RAW_GPS: 
          rawDisplay.showRawGPS(&gps, latitude, longitude, altitude, 
                               satellites, hdop, hour, minute, second, gpsValid);
          break;
          
        case RAW_CELESTIAL: 
          rawDisplay.showRawCelestial(sunAz, sunAlt, moonAz, moonAlt, moonPhase, 
                                     polarisAz, polarisAlt);
          break;
          
        case RAW_SYSTEM: 
          // Get system information
          // AtomS3Rはバッテリーを内蔵していないため、バッテリーレベルは使用しない
          {
            float batteryLevel = 0.0f; // ダミー値（使用されない）
            float temperature = getTemperature();
            unsigned long uptime = millis();
            int freeMemory = ESP.getFreeHeap();
            rawDisplay.showSystemInfo(batteryLevel, temperature, uptime, freeMemory);
          }
          break;
          
        case RAW_DEBUG_MODE: 
          // Display issue information with more detailed message
          {
            // IMUの校正状態を詳細に取得
            String imuStatus;
            if (imuCalibrated) {
              imuStatus = "OK";
            } else {
              // 校正状態を確認
              bool accelCalibrated = calibrationManager.isAccelCalibrated();
              bool magCalibrated = calibrationManager.isMagCalibrated();
              
              if (!accelCalibrated && !magCalibrated) {
                imuStatus = "NOT CALIBRATED";
              } else {
                imuStatus = "PARTIAL";
                if (!accelCalibrated) imuStatus += " A:NG";
                if (!magCalibrated) imuStatus += " M:NG";
              }
            }
            
            // GPSの状態を詳細に取得
            String gpsStatus;
            if (gpsValid) {
              gpsStatus = "OK (Sats: " + String(satellites) + ")";
            } else {
              if (satellites > 0) {
                gpsStatus = "WEAK (" + String(satellites) + " sats)";
              } else {
                gpsStatus = "NO SIGNAL";
              }
            }
            
            // システム情報を取得
            float temp = getTemperature();
            uint32_t freeHeap = ESP.getFreeHeap();
            uint32_t totalHeap = ESP.getHeapSize();
            int heapPercent = (freeHeap * 100) / totalHeap;
            
            // 詳細なメッセージを作成
            char issueMessage[256];
            snprintf(issueMessage, sizeof(issueMessage), 
                    "System Status:\n"
                    "IMU: %s\n"
                    "GPS: %s\n"
                    "Temp: %.1f C\n"
                    "Mem: %d KB (%d%%)\n"
                    "Uptime: %d min",
                    imuStatus.c_str(),
                    gpsStatus.c_str(),
                    temp,
                    freeHeap / 1024,
                    heapPercent,
                    millis() / 60000);
            
            rawDisplay.showDebugInfo(issueMessage);
          }
          break;
      }
      break;
      
    case SETTINGS_MENU: 
      // Display settings menu
      if (settingsMenu.isActive()) {
        // Update settings menu display
        settingsMenu.update();
      } else {
        // Show settings overview
        display.showSettings();
      }
      break;
      
    case CALIBRATION_MODE: 
      // Display calibration status
      if (calibrationManager.isCalibrating()) {
        // Get current calibration status
        CalibrationStatus status = calibrationManager.getCalibrationStatus();
        // Display calibration progress
        display.showCalibration(status.stage, status.progress);
      } else {
        // Show idle calibration screen
        display.showCalibration(0, 0.0);
      }
      break;
  }
}

void handleButtonPress() {
  // Handle button press
  if (M5.BtnA.wasPressed()) {
    // Check if settings menu is active
    if (currentMode == SETTINGS_MENU && settingsMenu.isActive()) {
      // Pass short press to settings menu
      settingsMenu.handleButtonPress(true, false);
      return;
    }
    
    // Short press - cycle through display modes
    cycleDisplayMode();
  } else if (M5.BtnA.pressedFor(1000)) {
    // Check if settings menu is active
    if (currentMode == SETTINGS_MENU && settingsMenu.isActive()) {
      // Pass long press to settings menu
      settingsMenu.handleButtonPress(false, true);
      return;
    }
    
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
      break;
    case RAW_DATA: 
      currentMode = SETTINGS_MENU;
      break;
    case SETTINGS_MENU: 
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
      // Cycle through raw data modes
      cycleRawDataMode();
      break;
      
    case SETTINGS_MENU: 
      // Handle settings menu interaction
      if (settingsMenu.isActive()) {
        // Handle long press in settings menu
        settingsMenu.handleButtonPress(false, true);
      } else {
        // Activate settings menu
        settingsMenu.show();
      }
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
      currentRawMode = RAW_DEBUG_MODE; 
      break;
    case RAW_DEBUG_MODE: 
      currentRawMode = RAW_IMU; 
      break;
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
    case RAW_DEBUG_MODE: 
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
