/*
 * GPSDataManager.cpp
 * 
 * GPS情報の保存と読み込みを管理するクラスの実装
 * 
 * Created: 2025-03-29
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "GPSDataManager.h"

// コンストラクタ
GPSDataManager::GPSDataManager() {
  _lastSaveTime = 0;
  _saveIntervalMs = 60 * 60 * 1000; // デフォルト: 60分（フラッシュメモリの寿命を考慮）
  _hasStoredData = false;
}

// 初期化
void GPSDataManager::begin() {
  // Preferencesの初期化
  _preferences.begin(NAMESPACE, false);
  
  // 保存されたデータがあるか確認
  _hasStoredData = _preferences.getBool(KEY_HAS_DATA, false);
  
  Serial.print("GPS Data Manager initialized. Stored data: ");
  Serial.println(_hasStoredData ? "Yes" : "No");
}

// GPS情報の保存
bool GPSDataManager::saveGPSData(const GPSData& data) {
  // 現在時刻を取得
  unsigned long currentTime = millis();
  
  // 保存間隔をチェック
  if (!shouldSaveData(currentTime)) {
    Serial.println("Skipping GPS data save (interval not reached)");
    return false;
  }
  
  // 各データを保存
  _preferences.putFloat(KEY_LATITUDE, data.latitude);
  _preferences.putFloat(KEY_LONGITUDE, data.longitude);
  _preferences.putFloat(KEY_ALTITUDE, data.altitude);
  _preferences.putInt(KEY_SATELLITES, data.satellites);
  _preferences.putFloat(KEY_HDOP, data.hdop);
  
  _preferences.putInt(KEY_YEAR, data.year);
  _preferences.putInt(KEY_MONTH, data.month);
  _preferences.putInt(KEY_DAY, data.day);
  _preferences.putInt(KEY_HOUR, data.hour);
  _preferences.putInt(KEY_MINUTE, data.minute);
  _preferences.putInt(KEY_SECOND, data.second);
  
  _preferences.putULong(KEY_LAST_UPDATE, currentTime);
  _preferences.putBool(KEY_HAS_DATA, true);
  
  // 状態を更新
  _lastSaveTime = currentTime;
  _hasStoredData = true;
  
  Serial.println("GPS data saved to flash memory");
  Serial.print("Location: ");
  Serial.print(data.latitude, 6);
  Serial.print(", ");
  Serial.println(data.longitude, 6);
  
  return true;
}

// GPS情報の読み込み
bool GPSDataManager::loadGPSData(GPSData& data) {
  // 保存されたデータがない場合
  if (!_hasStoredData) {
    Serial.println("No stored GPS data available");
    return false;
  }
  
  // 各データを読み込み
  data.latitude = _preferences.getFloat(KEY_LATITUDE, 0.0);
  data.longitude = _preferences.getFloat(KEY_LONGITUDE, 0.0);
  data.altitude = _preferences.getFloat(KEY_ALTITUDE, 0.0);
  data.satellites = _preferences.getInt(KEY_SATELLITES, 0);
  data.hdop = _preferences.getFloat(KEY_HDOP, 99.99);
  
  data.year = _preferences.getInt(KEY_YEAR, 2025);
  data.month = _preferences.getInt(KEY_MONTH, 3);
  data.day = _preferences.getInt(KEY_DAY, 29);
  data.hour = _preferences.getInt(KEY_HOUR, 0);
  data.minute = _preferences.getInt(KEY_MINUTE, 0);
  data.second = _preferences.getInt(KEY_SECOND, 0);
  
  data.lastUpdateTime = _preferences.getULong(KEY_LAST_UPDATE, 0);
  
  Serial.println("Loaded GPS data from flash memory");
  Serial.print("Location: ");
  Serial.print(data.latitude, 6);
  Serial.print(", ");
  Serial.println(data.longitude, 6);
  
  return true;
}

// 保存されたGPS情報があるかどうか
bool GPSDataManager::hasStoredData() {
  return _hasStoredData;
}

// 最終更新時刻の取得
unsigned long GPSDataManager::getLastUpdateTime() {
  return _preferences.getULong(KEY_LAST_UPDATE, 0);
}

// 保存間隔の設定（分単位）
void GPSDataManager::setSaveInterval(unsigned long intervalMinutes) {
  _saveIntervalMs = intervalMinutes * 60 * 1000;
  Serial.print("GPS data save interval set to ");
  Serial.print(intervalMinutes);
  Serial.println(" minutes");
}

// 保存間隔が経過したかどうか
bool GPSDataManager::shouldSaveData(unsigned long currentTime) {
  // 初回保存の場合
  if (_lastSaveTime == 0) {
    return true;
  }
  
  // millis()のオーバーフロー対策
  if (currentTime < _lastSaveTime) {
    _lastSaveTime = 0;
    return true;
  }
  
  // 保存間隔をチェック
  return (currentTime - _lastSaveTime) >= _saveIntervalMs;
}
