#ifndef PNG_LOADER_H
#define PNG_LOADER_H

#include <M5Unified.h>
#include <SPIFFS.h>

// SPIFFSの初期化
bool initSPIFFS() {
  static bool initialized = false;
  if (!initialized) {
    initialized = SPIFFS.begin(true);
    if (!initialized) {
      Serial.println("SPIFFS Mount Failed");
    } else {
      Serial.println("SPIFFS Mounted Successfully");
    }
  }
  return initialized;
}

// SPIFFSのファイル一覧を表示（デバッグ用）
void listSPIFFSFiles() {
  if (!initSPIFFS()) return;
  
  File root = SPIFFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory");
    return;
  }

  Serial.println("Files in SPIFFS:");
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      Serial.print("  ");
      Serial.print(file.name());
      Serial.print("  Size: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  root.close();
}

// シンプルなPNG表示関数（PNGdecライブラリを使用しない）
bool drawPNG(const char* filename, int x, int y) {
  // この実装では、実際にはPNGを描画せず、代わりに
  // ファイルの存在確認のみを行います
  if (!initSPIFFS()) return false;
  
  if (SPIFFS.exists(filename)) {
    Serial.print("PNG file found: ");
    Serial.println(filename);
    return true;
  } else {
    Serial.print("PNG file not found: ");
    Serial.println(filename);
    return false;
  }
}

#endif // PNG_LOADER_H
