#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <iostream>
#include <cstring>
#include <cstdlib>

#ifdef UI_H
#include "ui_stub.h"
#else
#include "ui_stub.h"
#endif

struct M5Display {
    static void fillRect(int x, int y, int width, int height, int color) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.fillRect($0, $1, $2, $3, $4);
            }
        }, x, y, width, height, color);
    }
    
    static void setCursor(int x, int y) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.setCursor($0, $1);
            }
        }, x, y);
    }
    
    static void setTextColor(int textColor, int bgColor = -1) {
        if (bgColor == -1) {
            EM_ASM({
                if (window.m5emulator && window.m5emulator.Display) {
                    window.m5emulator.Display.setTextColor($0);
                }
            }, textColor);
        } else {
            EM_ASM({
                if (window.m5emulator && window.m5emulator.Display) {
                    window.m5emulator.Display.setTextColor($0, $1);
                }
            }, textColor, bgColor);
        }
    }
    
    static void setTextSize(int size) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.setTextSize($0);
            }
        }, size);
    }
    
    static void print(const char* text) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.print(UTF8ToString($0));
            }
        }, text);
    }
    
    static void drawLine(int x1, int y1, int x2, int y2, int color) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.drawLine($0, $1, $2, $3, $4);
            }
        }, x1, y1, x2, y2, color);
    }
    
    static void drawRect(int x, int y, int width, int height, int color) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.drawRect($0, $1, $2, $3, $4);
            }
        }, x, y, width, height, color);
    }
    
    static void drawCircle(int x, int y, int radius, int color) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.drawCircle($0, $1, $2, $3);
            }
        }, x, y, radius, color);
    }
    
    static void fillCircle(int x, int y, int radius, int color) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.fillCircle($0, $1, $2, $3);
            }
        }, x, y, radius, color);
    }
    
    static int textWidth(const char* text) {
        return EM_ASM_INT({
            if (window.m5emulator && window.m5emulator.Display) {
                return window.m5emulator.Display.textWidth(UTF8ToString($0));
            }
            return 0;
        }, text);
    }
    
    static void setFont(void* font) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.setFont();
            }
        });
    }
    
    static void clear(int color = 0xFFFF) {
        EM_ASM({
            if (window.m5emulator && window.m5emulator.Display) {
                window.m5emulator.Display.clear($0);
            }
        }, color);
    }
};

struct M5Unified {
    static M5Display Display;
};

M5Display M5Unified::Display;

namespace {{NAMESPACE}} {
    extern void drawGameScreen();
    extern void handleTouch(int touchType, int x, int y);
    extern void initGame();
    extern void resetGame();
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void {{NAMESPACE}}_initGame() {
        {{NAMESPACE}}::initGame();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void {{NAMESPACE}}_drawGameScreen() {
        {{NAMESPACE}}::drawGameScreen();
    }
    
    EMSCRIPTEN_KEEPALIVE
    void {{NAMESPACE}}_handleTouch(int touchType, int x, int y) {
        {{NAMESPACE}}::handleTouch(touchType, x, y);
    }
    
    EMSCRIPTEN_KEEPALIVE
    void {{NAMESPACE}}_resetGame() {
        {{NAMESPACE}}::resetGame();
    }
}

EMSCRIPTEN_BINDINGS({{NAMESPACE}}) {
    emscripten::function("{{NAMESPACE}}_initGame", &{{NAMESPACE}}_initGame);
    emscripten::function("{{NAMESPACE}}_drawGameScreen", &{{NAMESPACE}}_drawGameScreen);
    emscripten::function("{{NAMESPACE}}_handleTouch", &{{NAMESPACE}}_handleTouch);
    emscripten::function("{{NAMESPACE}}_resetGame", &{{NAMESPACE}}_resetGame);
}