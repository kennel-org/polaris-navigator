/*
 * StartupScreen.h
 * 
 * Startup screen and initialization display for the Polaris Navigator
 * Handles splash screen and initialization status display
 * 
 * Created: 2025-03-30
 * GitHub: https://github.com/kennel-org/polaris-navigator
 */

#ifndef STARTUP_SCREEN_H
#define STARTUP_SCREEN_H

#include <M5Unified.h>

class StartupScreen {
public:
  // Constructor
  StartupScreen();
  
  // Initialize startup screen
  void begin();
  
  // Show splash screen
  void showSplashScreen();
  
  // Show initialization progress
  void showInitProgress(const char* message, int progressPercent);
  
  // Show initialization complete
  void showInitComplete();
  
  // Show initialization error
  void showInitError(const char* errorMessage);
  
  // Set LED color
  void setLedColor(uint32_t color);
  
  // Draw logo on the startup screen (画面下部18ピクセルを除く領域に表示)
  void drawLogo();
  
private:
  // Current LED color
  uint32_t _currentLedColor;
  
  // Helper method: Blink the LED
  void blinkLed(uint32_t color1, uint32_t color2, int count, int delayMs);
};

#endif // STARTUP_SCREEN_H
