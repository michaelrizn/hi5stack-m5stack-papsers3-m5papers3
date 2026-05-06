#ifndef UI_STUB_H
#define UI_STUB_H

#include <string>
#include <chrono>

const int TFT_BLACK = 0x0000;
const int TFT_WHITE = 0xFFFF;
const int TFT_RED = 0xF800;
const int TFT_GREEN = 0x07E0;
const int TFT_BLUE = 0x001F;
const int TFT_YELLOW = 0xFFE0;
const int TFT_MAGENTA = 0xF81F;
const int TFT_CYAN = 0x07FF;
const int TFT_DARKGREY = 0x8410;
const int TFT_LIGHTGREY = 0xC618;

const int BLACK = TFT_BLACK;
const int WHITE = TFT_WHITE;
const int RED = TFT_RED;
const int GREEN = TFT_GREEN;
const int BLUE = TFT_BLUE;
const int YELLOW = TFT_YELLOW;
const int MAGENTA = TFT_MAGENTA;
const int CYAN = TFT_CYAN;
const int DARKGREY = TFT_DARKGREY;
const int LIGHTGREY = TFT_LIGHTGREY;

inline unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

class String {
public:
    std::string data;
    
    String() = default;
    String(const char* str) : data(str) {}
    String(const std::string& str) : data(str) {}
    String(int num) : data(std::to_string(num)) {}
    String(float num) : data(std::to_string(num)) {}
    
    const char* c_str() const { return data.c_str(); }
    size_t length() const { return data.length(); }
    
    String operator+(const String& other) const {
        return String(data + other.data);
    }
    
    String operator+(const char* other) const {
        return String(data + other);
    }
    
    String operator+(int num) const {
        return String(data + std::to_string(num));
    }
    
    bool operator==(const String& other) const {
        return data == other.data;
    }
    
    bool operator!=(const String& other) const {
        return data != other.data;
    }
    
    char operator[](size_t index) const {
        return data[index];
    }
    
    String& operator+=(const String& other) {
        data += other.data;
        return *this;
    }
    
    String& operator+=(const char* other) {
        data += other;
        return *this;
    }
    
    String& operator+=(int num) {
        data += std::to_string(num);
        return *this;
    }
    
    int toInt() const {
        try {
            return std::stoi(data);
        } catch (...) {
            return 0;
        }
    }
};

namespace keyboards {
    inline void drawNumbersKeyboard() {
    }

    inline String getNumberKeyFromTouch(int x, int y) {
        return String("");
    }
}

extern "C" {
    void emulator_clear(int color);
    void emulator_setTextColor(int color);
    void emulator_setTextSize(int size);
    void emulator_drawString(const char* text, int x, int y);
    void emulator_drawLine(int x1, int y1, int x2, int y2);
    void emulator_setColor(int color);
    int emulator_textWidth(const char* text);
    int emulator_fontHeight();
    int emulator_width();
    int emulator_height();
}

class DisplayEmulator {
public:
    void clear(int color = WHITE) {
        emulator_clear(color);
    }
    
    void setTextColor(int color) {
        emulator_setTextColor(color);
    }
    
    void setTextSize(int size) {
        emulator_setTextSize(size);
    }
    
    void drawString(const String& text, int x, int y) {
        emulator_drawString(text.c_str(), x, y);
    }
    
    void drawLine(int x1, int y1, int x2, int y2) {
        emulator_drawLine(x1, y1, x2, y2);
    }
    
    void setColor(int color) {
        emulator_setColor(color);
    }
    
    int textWidth(const String& text) {
        return emulator_textWidth(text.c_str());
    }
    
    int fontHeight() {
        return emulator_fontHeight();
    }
    
    int width() {
        return emulator_width();
    }
    
    int height() {
        return emulator_height();
    }
};

class M5StackEmulator {
public:
    DisplayEmulator Display;
};

extern M5StackEmulator M5;

#endif