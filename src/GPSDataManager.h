/*
 * GPSDataManager.h
 * 
 * GPS情報の保存と読み込みを管理するクラス
 * 
 * Created: 2025-03-29
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef GPS_DATA_MANAGER_H
#define GPS_DATA_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

// GPS情報の構造体
struct GPSData {
  float latitude;
  float longitude;
  float altitude;
  int satellites;
  float hdop;
  
  // 時刻情報
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  
  // 最終更新時刻（内部管理用）
  unsigned long lastUpdateTime;
};

class GPSDataManager {
public:
  // コンストラクタ
  GPSDataManager();
  
  // 初期化
  void begin();
  
  // GPS情報の保存
  bool saveGPSData(const GPSData& data);
  
  // GPS情報の読み込み
  bool loadGPSData(GPSData& data);
  
  // 保存されたGPS情報があるかどうか
  bool hasStoredData();
  
  // 最終更新時刻の取得
  unsigned long getLastUpdateTime();
  
  // 保存間隔の設定（分単位、デフォルト60分）
  void setSaveInterval(unsigned long intervalMinutes);
  
  // 保存間隔が経過したかどうか
  bool shouldSaveData(unsigned long currentTime);
  
private:
  Preferences _preferences;
  unsigned long _lastSaveTime;
  unsigned long _saveIntervalMs; // 保存間隔（ミリ秒）
  bool _hasStoredData;
  
  // 保存用の名前空間
  const char* NAMESPACE = "gpsdata";
  
  // 保存キー
  const char* KEY_LATITUDE = "lat";
  const char* KEY_LONGITUDE = "lon";
  const char* KEY_ALTITUDE = "alt";
  const char* KEY_SATELLITES = "sat";
  const char* KEY_HDOP = "hdop";
  const char* KEY_YEAR = "year";
  const char* KEY_MONTH = "month";
  const char* KEY_DAY = "day";
  const char* KEY_HOUR = "hour";
  const char* KEY_MINUTE = "min";
  const char* KEY_SECOND = "sec";
  const char* KEY_LAST_UPDATE = "lupd";
  const char* KEY_HAS_DATA = "has";
};

#endif // GPS_DATA_MANAGER_H
