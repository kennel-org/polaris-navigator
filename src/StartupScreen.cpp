/*
 * StartupScreen.cpp
 * 
 * Implementation for the Polaris Navigator startup screen
 * 
 * Created: 2025-03-30
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#include "StartupScreen.h"

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

// Show splash screen
void StartupScreen::showSplashScreen() {
  // 画面の初期化のみを行う（背景色は設定しない）
  M5.Display.clear();
  
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
  
  // 描画エリアを消去
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
  // 描画エリアを消去
  M5.Display.fillRect(0, M5.Display.height() - 18, M5.Display.width(), 18, TFT_BLACK);
  
  // 1行目: 完了メッセージ
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(1.0);
  M5.Display.setCursor(5, M5.Display.height() - 18);
  M5.Display.println("Init Complete");
  
  // 2行目: 指示メッセージ
  M5.Display.setCursor(15, M5.Display.height() - 8);
  M5.Display.println("Press button");
  
  // Set LED to green to indicate success
  setLedColor(COLOR_GREEN);
  
  // Short delay to show completion
  delay(1000);
}

// Show initialization error
void StartupScreen::showInitError(const char* errorMessage) {
  // 描画エリアを消去
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
