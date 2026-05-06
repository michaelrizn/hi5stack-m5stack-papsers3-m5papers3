#include "ui_stub.h"
#include <emscripten.h>

// Global M5 instance
M5StackEmulator M5;

extern "C" {
    void emulator_clear(int color) {
        EM_ASM({
            if (window.emulator && window.emulator.display) {
                window.emulator.display.clear(window.emulator.display.colorToHex($0));
            }
        }, color);
    }
    
    void emulator_setTextColor(int color) {
        EM_ASM({
            if (window.emulator && window.emulator.display) {
                window.emulator.display.setTextColor(window.emulator.display.colorToHex($0));
            }
        }, color);
    }
    
    void emulator_setTextSize(int size) {
        EM_ASM({
            if (window.emulator && window.emulator.display) {
                window.emulator.display.setTextSize($0);
            }
        }, size);
    }
    
    void emulator_drawString(const char* text, int x, int y) {
        EM_ASM({
            if (window.emulator && window.emulator.display) {
                window.emulator.display.drawString(UTF8ToString($0), $1, $2);
            }
        }, text, x, y);
    }
    
    void emulator_drawLine(int x1, int y1, int x2, int y2) {
        EM_ASM({
            if (window.emulator && window.emulator.display) {
                window.emulator.display.drawLine($0, $1, $2, $3);
            }
        }, x1, y1, x2, y2);
    }
    
    void emulator_setColor(int color) {
        EM_ASM({
            if (window.emulator && window.emulator.display) {
                window.emulator.display.setColor($0);
            }
        }, color);
    }
    
    int emulator_textWidth(const char* text) {
        return EM_ASM_INT({
            if (window.emulator && window.emulator.display) {
                return window.emulator.display.textWidth(UTF8ToString($0));
            }
            return 0;
        }, text);
    }
    
    int emulator_fontHeight() {
        return EM_ASM_INT({
            if (window.emulator && window.emulator.display) {
                return window.emulator.display.fontHeight();
            }
            return 16;
        });
    }
    
    int emulator_width() {
        return EM_ASM_INT({
            if (window.emulator && window.emulator.display) {
                return window.emulator.display.width;
            }
            return 320;
        });
    }
    
    int emulator_height() {
        return EM_ASM_INT({
            if (window.emulator && window.emulator.display) {
                return window.emulator.display.height;
            }
            return 240;
        });
    }
}