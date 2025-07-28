#ifndef TEST_GAME_H
#define TEST_GAME_H

#include <M5Unified.h>

namespace games_test {
    void drawGameScreen();
    void drawDashedBorder();
    void handleTouch(int touchedRow, int x, int y);
    void initGame();
    void resetGame();
}

#endif