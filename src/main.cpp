#include <algorithm>
#include <M5Unified.h>
#include <esp_sleep.h>
#include <WiFi.h>

#include "battery.h"
#include "sdcard.h"
#include "debug_config.h"
#include "ui.h"
#include "footer.h"
#include "settings.h"
#include "screens/wifi_screen.h"
#include "screens/apps_screen.h"
#include "screens/games_screen.h"
#include "keyboards/eng_keyboard.h"
#include "sd_gateway.h"
#include "network/wifi_manager.h"
#include "apps/text_lang_test/app_screen.h"
#include "apps/geometry_test/app_screen.h"
#include "apps/swipe_test/app_screen.h"
#include "apps/reader/app_screen.h"
#include "apps/calculator/app_screen.h"
#include "games/minesweeper/game.h"
#include "games/sudoku/game.h"
#include "games/test/game.h"

bool isRendering = false;
bool ui_needs_update = true;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE_DELAY = 300;
bool previousTouchState = false;
int lastTouchX = 0, lastTouchY = 0;


template <typename T>
T customClamp(T value, T minVal, T maxVal) {
    return (value < minVal) ? minVal : (value > maxVal) ? maxVal : value;
}

void setup() {
    M5.begin();
    M5.Display.begin();
    Serial.begin(115200);

    M5.Display.fillScreen(TFT_WHITE);
    M5.Display.setRotation(0);


    if (!initSDCard()) {
        Serial.println("Failed to initialize SD card");
    } else {
        Serial.println("SD card initialized successfully");
    }


    Settings settings;

    setupUI();

    M5.Display.display();
}

void loop() {
    M5.update();
    if (ui_needs_update) {
        updateUI();
        ui_needs_update = false;
    }
    sd_gateway::loop();
    

    WiFiManager::getInstance().updateScanResults();
    

    if (currentScreen == GEOMETRY_TEST_SCREEN) {
        apps_geometry_test::updateAnimation();
    }
    

    if (currentScreen == SWIPE_TEST_SCREEN) {
        apps_swipe_test::updateAnimation();
    }

    if (isRendering) {
    
        return;
    }


    bool currentTouchState = M5.Display.touch();
    
    if (currentTouchState && !isRendering && (millis() - lastTouchTime > TOUCH_DEBOUNCE_DELAY)) {
        lgfx::touch_point_t tp;
        if (M5.Display.getTouchRaw(&tp, 1)) {
            M5.Display.convertRawXY(&tp, 1);
            int16_t x = tp.x;
            int16_t y = tp.y;
            
            lastTouchX = x;
            lastTouchY = y;
            
            lastTouchTime = millis();
            if (currentScreen != SWIPE_TEST_SCREEN) {
                isRendering = true;
            }
            ui_needs_update = true;
            


            if (footer.isVisible() && y >= (EPD_HEIGHT - 60) && 
                currentScreen != MINESWEEPER_GAME_SCREEN && 
                currentScreen != SUDOKU_GAME_SCREEN && 
                currentScreen != TEST_GAME_SCREEN) {
                int buttonCount = footer.getButtonCount();
                if (buttonCount == 0) return;

                int buttonWidth = EPD_WIDTH / buttonCount;
                int buttonIndex = x / buttonWidth;
                buttonIndex = customClamp(buttonIndex, 0, buttonCount - 1);

                if (buttonIndex < footer.getButtonCount()) {
                    #ifdef DEBUG_TOUCH
                    Serial.println(footer.getButtons()[buttonIndex].label + " button pressed");
                    #endif
                    footer.invokeButtonAction(buttonIndex);
                }
                isRendering = false;
            } else {

                if (currentScreen == FILES_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    screens::handleTouch(touchedRow, x, y);
                }

                else if (currentScreen == MAIN_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    if (touchedRow == 5) {
                        displayMessage("Wi-Fi pressed");
                        screens::resetWiFiScreen();
                        currentScreen = WIFI_SCREEN;
                        renderCurrentScreen();
                    } else if (touchedRow == 6) {
                        if (WiFi.status() != WL_CONNECTED) {
                            displayMessage("Wi-Fi not connected");
                        } else {
                            sd_gateway::toggleOrShow();
                        }
                        renderCurrentScreen();
                    } else if (touchedRow == 7) {
                        displayMessage("Apps pressed");
                        currentScreen = APPS_SCREEN;
                        renderCurrentScreen();
                    } else if (touchedRow == 8) {
                        displayMessage("Games pressed");
                        currentScreen = GAMES_SCREEN;
                        renderCurrentScreen();
                    }
                }

                else if (currentScreen == WIFI_SCREEN) {
                    screens::handleWiFiScreenTouch(x, y);
                }

                else if (currentScreen == APPS_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    if (touchedRow >= 3) {
                         screens::handleAppsSelection(touchedRow);
                    }
                }

                else if (currentScreen == GAMES_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    if (touchedRow >= 3) {
                         handleGamesScreenTouch(touchedRow, x, y);
                    }
                    isRendering = false;
                }

                else if (currentScreen == SWIPE_TEST_SCREEN) {
                    apps_swipe_test::handleTouch(x, y, true);

                }

                else if (currentScreen == READER_APP_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    apps_reader::handleTouch(touchedRow, x, y);
                    isRendering = false;
                } else if (currentScreen == CALCULATOR_APP_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    apps_calculator::handleTouch(touchedRow, x, y);
                    isRendering = false;
                } else if (currentScreen == MINESWEEPER_GAME_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    games_minesweeper::handleTouch(touchedRow, x, y);
                    isRendering = false;
                } else if (currentScreen == SUDOKU_GAME_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    games_sudoku::handleTouch(touchedRow, x, y);
                    isRendering = false;
                } else if (currentScreen == TEST_GAME_SCREEN) {
                    int touchedRow = getRowFromY(y);
                    games_test::handleTouch(touchedRow, x, y);
                    isRendering = false;
                } else {
                    isRendering = false;
                }
            }
            

        }

    }
    

    if (previousTouchState && !currentTouchState && currentScreen == SWIPE_TEST_SCREEN) {
        apps_swipe_test::handleTouch(lastTouchX, lastTouchY, false);
        isRendering = false;
    }
    
    previousTouchState = currentTouchState;
}
