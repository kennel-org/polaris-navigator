/*
 * CelestialCalculator.h
 * 
 * 天体位置計算クラス
 * 太陽、月、北極星の方位角と高度を計算
 */

#ifndef CELESTIAL_CALCULATOR_H
#define CELESTIAL_CALCULATOR_H

class CelestialCalculator {
public:
  // コンストラクタ
  CelestialCalculator();
  
  // 天体位置の計算
  void calculate(float latitude, float longitude, float altitude, 
                int year, int month, int day, 
                int hour, int minute, int second);
  
  // 太陽の方位角を取得
  float getSunAzimuth() const { return _sunAzimuth; }
  
  // 太陽の高度を取得
  float getSunAltitude() const { return _sunAltitude; }
  
  // 月の方位角を取得
  float getMoonAzimuth() const { return _moonAzimuth; }
  
  // 月の高度を取得
  float getMoonAltitude() const { return _moonAltitude; }
  
  // 北極星の方位角を取得
  float getPolarisAzimuth() const { return _polarisAzimuth; }
  
  // 北極星の高度を取得
  float getPolarisAltitude() const { return _polarisAltitude; }
  
private:
  // 計算結果
  float _sunAzimuth;
  float _sunAltitude;
  float _moonAzimuth;
  float _moonAltitude;
  float _polarisAzimuth;
  float _polarisAltitude;
  
  // 補助計算関数
  void calculateSunPosition(float latitude, float longitude, 
                           int year, int month, int day, 
                           int hour, int minute, int second);
  
  void calculateMoonPosition(float latitude, float longitude, 
                            int year, int month, int day, 
                            int hour, int minute, int second);
  
  void calculatePolarisPosition(float latitude, float longitude);
};

#endif // CELESTIAL_CALCULATOR_H
