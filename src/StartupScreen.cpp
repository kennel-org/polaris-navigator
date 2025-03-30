/*
 * StartupScreen.cpp
 * 
 * Implementation for the Polaris Navigator startup screen
 * 
 * Created: 2025-03-30
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "StartupScreen.h"
#include "icon.h" // ロゴ表示用のアイコンデータをインクルード

// Color definitions
#define COLOR_RED    0xFF0000
#define COLOR_GREEN  0x00FF00
#define COLOR_BLUE   0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_PURPLE 0xFF00FF
#define COLOR_CYAN   0x00FFFF
#define COLOR_WHITE  0xFFFFFF
#define COLOR_BLACK  0x000000

// Constructor
StartupScreen::StartupScreen() {
  _currentLedColor = COLOR_BLACK;
}

// Initialize startup screen
void StartupScreen::begin() {
  // Set display settings
  M5.Display.setRotation(0); // Portrait orientation
  M5.Display.setTextSize(1); // Small font size
}

// Set LED color (for RGB LED)
void StartupScreen::setLedColor(uint32_t color) {
  // Save current color
  _currentLedColor = color;
  
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

// Helper method: Blink the LED between two colors
void StartupScreen::blinkLed(uint32_t color1, uint32_t color2, int count, int delayMs) {
  // Simplified implementation for now
  for (int i = 0; i < count; i++) {
    setLedColor(color1);
    delay(delayMs);
    setLedColor(color2);
    delay(delayMs);
  }
  // Return to the first color
  setLedColor(color1);
}

// Draw logo on the startup screen
void StartupScreen::drawLogo() {
  // 画面サイズを取得
  int screenWidth = M5.Display.width();
  int screenHeight = M5.Display.height();
  
  // ロゴのサイズ（icon.hから取得）
  int logoWidth = width;
  int logoHeight = height;
  
  // ロゴの表示位置を計算（画面中央に配置）
  int x = (screenWidth - logoWidth) / 2;
  
  // 画面下部18ピクセルを除く領域に表示するため、
  // 上部に配置（ロゴの高さに応じて調整）
  int availableHeight = screenHeight - 18; // 下部18ピクセルを除く
  int y = (availableHeight - logoHeight) / 2;
  
  // 負の値にならないように調整
  if (y < 0) y = 0;
  
  // アイコンデータの解析と描画
  char *data = header_data;
  uint8_t pixel[3]; // RGB値を格納する配列
  
  // メモリ使用量を抑えるため、1行ずつ描画
  for (int row = 0; row < logoHeight; row++) {
    for (int col = 0; col < logoWidth; col++) {
      // ピクセルデータを取得
      HEADER_PIXEL(data, pixel);
      
      // RGB565形式に変換
      uint16_t color = ((pixel[0] & 0xF8) << 8) | ((pixel[1] & 0xFC) << 3) | (pixel[2] >> 3);
      
      // ピクセルを描画（透過処理：黒色は描画しない）
      if (!(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)) {
        M5.Display.drawPixel(x + col, y + row, color);
      }
    }
  }
}

// Show splash screen
void StartupScreen::showSplashScreen() {
  // 画面の初期化
  M5.Display.clear();
  
  // ロゴを表示
  drawLogo();
  
  // 描画エリアを消去
  M5.Display.fillRect(0, M5.Display.height() - 18, M5.Display.width(), 18, TFT_BLACK);
  
  // 画面下部18ピクセルに2行だけで表示
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.0);
  
  // 1行目: アプリ名
  M5.Display.setCursor(10, M5.Display.height() - 18);
  M5.Display.println("Polaris Nav");
  
  // 2行目: 起動メッセージ
  M5.Display.setCursor(25, M5.Display.height() - 8);
  M5.Display.println("Starting...");
  
  // Set LED to blue during startup
  setLedColor(COLOR_BLUE);
}

// Show initialization progress
void StartupScreen::showInitProgress(const char* message, int progressPercent) {
  // Ensure progress is within bounds
  if (progressPercent < 0) progressPercent = 0;
  if (progressPercent > 100) progressPercent = 100;
  
  // 描画エリアを消去（下部18ピクセルのみ）
  M5.Display.fillRect(0, M5.Display.height() - 18, M5.Display.width(), 18, TFT_BLACK);
  
  // 1行目: 進捗メッセージとパーセント
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.0);
  M5.Display.setCursor(5, M5.Display.height() - 18);
  M5.Display.printf("%s", message);
  
  // プログレスバーは表示しない
}

// Show initialization complete
void StartupScreen::showInitComplete() {
  // 描画エリアを消去（下部18ピクセルのみ）
  M5.Display.fillRect(0, M5.Display.height() - 18, M5.Display.width(), 18, TFT_BLACK);
  
  // 輝度を明示的に再設定して一貫性を保つ
  M5.Display.setBrightness(40);  // 40%に設定（setupHardwareと同じ値）
  
  // 完了メッセージを中央に表示
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.0);
  
  // テキストを中央揃えにするための計算
  int textWidth = 12 * 12; // "Init Complete"の幅を概算（文字数×ピクセル幅）
  int xPos = (M5.Display.width() - textWidth) / 2;
  if (xPos < 0) xPos = 5; // 負の値にならないように調整
  
  M5.Display.setCursor(xPos, M5.Display.height() - 12);
  M5.Display.println("Init Complete");
  
  // Set LED to green to indicate success (輝度を抑えた緑色に設定)
  setLedColor(0x007F00);  // 暗めの緑色を使用
  
  // Short delay to show completion
  delay(1000);
}

// Show initialization error
void StartupScreen::showInitError(const char* errorMessage) {
  // 描画エリアを消去（下部18ピクセルのみ）
  M5.Display.fillRect(0, M5.Display.height() - 18, M5.Display.width(), 18, TFT_BLACK);
  
  // 1行目: エラーメッセージ
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.0);
  M5.Display.setCursor(5, M5.Display.height() - 18);
  M5.Display.println(errorMessage);
  
  // Set LED to red to indicate error
  setLedColor(COLOR_RED);
  
  // Blink LED to draw attention to error
  blinkLed(COLOR_RED, COLOR_BLACK, 3, 200);
  
  // Longer delay for error message
  delay(2000);
}
