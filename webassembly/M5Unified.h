#ifndef M5UNIFIED_H
#define M5UNIFIED_H

#include "ui_stub.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// Определяем константы цветов
#define BLACK 0x0000
#define WHITE 0xFFFF
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define YELLOW 0xFFE0
#define CYAN 0x07FF
#define MAGENTA 0xF81F

// Определяем min/max для совместимости
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

// Заглушка для M5Display
class M5DisplayClass {
public:
    void fillRect(int x, int y, int w, int h, uint16_t color) {}
    void setCursor(int x, int y) {}
    void setTextColor(uint16_t color) {}
    void setTextSize(int size) {}
    void print(const char* text) {}
    void print(int value) {}
    void print(float value) {}
    void drawLine(int x0, int y0, int x1, int y1, uint16_t color = 0xFFFF) {}
    void drawRect(int x, int y, int w, int h, uint16_t color = 0xFFFF) {}
    void drawCircle(int x, int y, int r, uint16_t color = 0xFFFF) {}
    void fillCircle(int x, int y, int r, uint16_t color = 0xFFFF) {}
    int textWidth(const char* text) { return strlen(text) * 6; }
    int textWidth(const String& text) { return text.length() * 6; }
    void clear() {}
    void fillScreen(uint16_t color) {}
    void drawString(const char* text, int x, int y) {}
    void drawString(const String& text, int x, int y) {}
    void setColor(uint16_t color) {}
    int fontHeight() { return 8; }
    int width() { return 540; }
    int height() { return 960; }
};

// M5Unified заглушка для WebAssembly
class M5UnifiedClass {
public:
    M5DisplayClass Display;
    
    void begin() {}
    void update() {}
};

#endif // M5UNIFIED_H