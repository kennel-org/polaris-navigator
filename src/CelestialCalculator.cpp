/*
 * CelestialCalculator.cpp
 * 
 * 天体位置計算クラスの実装
 */

#include "CelestialCalculator.h"
#include <Arduino.h>
#include <math.h>

// コンストラクタ
CelestialCalculator::CelestialCalculator() {
  // 初期値を設定
  _sunAzimuth = 0.0f;
  _sunAltitude = 0.0f;
  _moonAzimuth = 0.0f;
  _moonAltitude = 0.0f;
  _polarisAzimuth = 0.0f;
  _polarisAltitude = 0.0f;
}

// 天体位置の計算
void CelestialCalculator::calculate(float latitude, float longitude, float altitude, 
                                  int year, int month, int day, 
                                  int hour, int minute, int second) {
  // 太陽位置の計算
  calculateSunPosition(latitude, longitude, year, month, day, hour, minute, second);
  
  // 月位置の計算
  calculateMoonPosition(latitude, longitude, year, month, day, hour, minute, second);
  
  // 北極星位置の計算
  calculatePolarisPosition(latitude, longitude);
}

// 太陽位置の計算（簡易版）
void CelestialCalculator::calculateSunPosition(float latitude, float longitude, 
                                             int year, int month, int day, 
                                             int hour, int minute, int second) {
  // 簡易計算（実際の実装では天文アルゴリズムを使用）
  // 現在の時刻に基づいて太陽の位置を近似
  float timeOfDay = hour + minute / 60.0f + second / 3600.0f; // 0-24の時間
  
  // 太陽の方位角（東から南、西、北と時計回りに0-360度）
  _sunAzimuth = (timeOfDay / 24.0f * 360.0f + 180.0f);
  if (_sunAzimuth >= 360.0f) _sunAzimuth -= 360.0f;
  
  // 太陽の高度（-90度から90度、0度が地平線）
  float hourAngle = abs(timeOfDay - 12.0f); // 正午からの時間差
  _sunAltitude = 90.0f - (hourAngle / 12.0f * 90.0f);
  
  // 緯度による補正
  _sunAltitude = _sunAltitude - abs(latitude) / 90.0f * 30.0f;
  
  // デバッグ出力
  Serial.print("Sun position calculated: Az=");
  Serial.print(_sunAzimuth);
  Serial.print(", Alt=");
  Serial.println(_sunAltitude);
}

// 月位置の計算（簡易版）
void CelestialCalculator::calculateMoonPosition(float latitude, float longitude, 
                                              int year, int month, int day, 
                                              int hour, int minute, int second) {
  // 簡易計算（実際の実装では天文アルゴリズムを使用）
  // 太陽から約12時間ずれた位置に月があると仮定
  float timeOfDay = hour + minute / 60.0f + second / 3600.0f; // 0-24の時間
  float moonTimeOffset = timeOfDay + 12.0f;
  if (moonTimeOffset >= 24.0f) moonTimeOffset -= 24.0f;
  
  // 月の方位角
  _moonAzimuth = (moonTimeOffset / 24.0f * 360.0f + 180.0f);
  if (_moonAzimuth >= 360.0f) _moonAzimuth -= 360.0f;
  
  // 月の高度
  float hourAngle = abs(moonTimeOffset - 12.0f); // 正午からの時間差
  _moonAltitude = 90.0f - (hourAngle / 12.0f * 90.0f);
  
  // 緯度による補正
  _moonAltitude = _moonAltitude - abs(latitude) / 90.0f * 30.0f;
  
  // デバッグ出力
  Serial.print("Moon position calculated: Az=");
  Serial.print(_moonAzimuth);
  Serial.print(", Alt=");
  Serial.println(_moonAltitude);
}

// 北極星位置の計算
void CelestialCalculator::calculatePolarisPosition(float latitude, float longitude) {
  // 北極星は常に北（方位角0度）にあり、高度は観測者の緯度に近似
  _polarisAzimuth = 0.0f; // 北
  
  // 北半球では、北極星の高度は概ね観測地点の緯度に等しい
  if (latitude >= 0) {
    _polarisAltitude = latitude;
  } else {
    // 南半球では北極星は見えない（地平線下）
    _polarisAltitude = 0.0f;
  }
  
  // デバッグ出力
  Serial.print("Polaris position calculated: Az=");
  Serial.print(_polarisAzimuth);
  Serial.print(", Alt=");
  Serial.println(_polarisAltitude);
}
