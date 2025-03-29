#ifndef LOGO_H
#define LOGO_H

// ロゴの幅と高さ
#define ICON_WIDTH 64
#define ICON_HEIGHT 64

// 前方宣言
void drawStar(int16_t x, int16_t y, int16_t size, uint16_t color);

// 星を描画する関数
void drawStar(int16_t x, int16_t y, int16_t size, uint16_t color) {
  // 5角形の星を描画
  float angle = -PI/2; // 上から始める
  int16_t x1, y1, x2, y2;
  
  for (int i = 0; i < 5; i++) {
    x1 = x + size * cos(angle);
    y1 = y + size * sin(angle);
    x2 = x + size/2 * cos(angle + PI/5);
    y2 = y + size/2 * sin(angle + PI/5);
    
    M5.Display.drawLine(x, y, x1, y1, color);
    M5.Display.drawLine(x, y, x2, y2, color);
    
    angle += 2*PI/5;
  }
  
  // 星の中心に点を打つ
  M5.Display.drawPixel(x, y, color);
}

// ナビゲーターロゴを描画する関数
void drawNavigatorLogo(int16_t x, int16_t y, int16_t size) {
  // PNG表示機能は一時的に無効化
  // 代わりに描画関数でロゴを生成
  
  // 背景色（ネイビー）
  M5.Display.fillCircle(x + size/2, y + size/2, size/2, TFT_NAVY);
  
  // 外側の円（金色）
  M5.Display.drawCircle(x + size/2, y + size/2, size/2, TFT_GOLD);
  M5.Display.drawCircle(x + size/2, y + size/2, size/2 - 1, TFT_GOLD);
  
  // コンパスの内側の円
  M5.Display.drawCircle(x + size/2, y + size/2, size/2 - 5, TFT_DARKGREY);
  
  // コンパスの目盛り
  for (int i = 0; i < 12; i++) {
    float angle = i * 30 * DEG_TO_RAD;
    int x1 = x + size/2 + (size/2 - 8) * sin(angle);
    int y1 = y + size/2 - (size/2 - 8) * cos(angle);
    int x2 = x + size/2 + (size/2 - 5) * sin(angle);
    int y2 = y + size/2 - (size/2 - 5) * cos(angle);
    M5.Display.drawLine(x1, y1, x2, y2, TFT_DARKGREY);
  }
  
  // 北の赤い三角形
  int tx1 = x + size/2;
  int ty1 = y + 5;
  int tx2 = x + size/2 - 4;
  int ty2 = y + 12;
  int tx3 = x + size/2 + 4;
  int ty3 = y + 12;
  M5.Display.fillTriangle(tx1, ty1, tx2, ty2, tx3, ty3, TFT_RED);
  
  // キャラクターの顔（簡略化）
  // 顔の輪郭
  M5.Display.fillCircle(x + size/2, y + size/2, size/4, TFT_LIGHTGREY);
  
  // 目
  M5.Display.fillCircle(x + size/2 - 5, y + size/2 - 2, 3, TFT_BLACK);
  M5.Display.fillCircle(x + size/2 + 5, y + size/2 - 2, 3, TFT_BLACK);
  
  // 耳
  M5.Display.fillTriangle(
    x + size/2 - 12, y + size/2 - 5,
    x + size/2 - 5, y + size/2 - 12,
    x + size/2 - 5, y + size/2,
    TFT_LIGHTGREY
  );
  M5.Display.fillTriangle(
    x + size/2 + 12, y + size/2 - 5,
    x + size/2 + 5, y + size/2 - 12,
    x + size/2 + 5, y + size/2,
    TFT_LIGHTGREY
  );
  
  // 星（北極星）
  drawStar(x + size/2 + size/4, y + size/2 - size/4, 3, TFT_CYAN);
}

#endif // LOGO_H
