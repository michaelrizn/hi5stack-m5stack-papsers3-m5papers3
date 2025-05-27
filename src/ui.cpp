// ./ui.cpp
#include "ui.h"
#include <WiFi.h> // Added for using WiFi

// Include app headers
#include "apps/text_lang_test/app_screen.h"
#include "apps/test2/app_screen.h"

// Флаг первого рендера: на нем - полное обновление, далее - быстрый DU4
bool firstRenderDone = false;

// Initialize footer
Footer footer;

// Define global variables
Message currentMessage = {"", 0};
ScreenType currentScreen = MAIN_SCREEN;
String currentPath = "/";
std::vector<String> displayedFiles;
std::vector<BufferedRow> rowsBuffer;

// Функция для установки универсального шрифта
void setUniversalFont() {
    M5.Display.setFont(UNIVERSAL_FONT);
}

// Row position structure
RowPosition getRowPosition(int row) {
    return RowPosition{0, row * 60, EPD_WIDTH, 60};
}

// Draw a single row on the screen with optional underline
void drawRow(const String& text, int row, uint16_t textColor, uint16_t bgColor, int fontSize, bool underline) {
    RowPosition pos = getRowPosition(row);
    M5.Display.fillRect(pos.x, pos.y, pos.width, pos.height, bgColor);
    M5.Display.setCursor(pos.x + 10, pos.y + 10);
    M5.Display.setTextColor(textColor, bgColor);
    M5.Display.setTextSize(fontSize);
    M5.Display.print(text);
    
    if (underline) {
        int textWidth = M5.Display.textWidth(text.c_str());
        int underlineY = pos.y + 40; // Увеличен отступ для правильного подчёркивания
        M5.Display.drawLine(pos.x + 10, underlineY, pos.x + 10 + textWidth, underlineY, TFT_BLACK);
    }
}

// Draw all rows from buffer at once
void drawRowsBuffered() {
    for (const auto& row : rowsBuffer) {
        bool isFooter = false; // Footer is handled separately
        drawRow(row.text, row.row, row.textColor, row.bgColor, row.fontSize, row.underline);
    }
    rowsBuffer.clear(); // Clear buffer after rendering
}

// Add row to buffer
void bufferRow(const String& text, int row, uint16_t textColor, uint16_t bgColor, int fontSize, bool underline) {
    rowsBuffer.emplace_back(BufferedRow{text, row, textColor, bgColor, fontSize, underline});
}

// Update the header with battery information
void updateHeader() {
    bufferRow("Battery: " + String(getBatteryPercentage()) + "%", 0, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
    if (currentMessage.text != "") {
        bufferRow(currentMessage.text, 1, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
    } else {
        bufferRow("", 1, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
    }
}

// Initialize UI
void setupUI() {
    // Устанавливаем универсальный шрифт
    setUniversalFont();
    
    // Setup initial screen elements
    currentScreen = MAIN_SCREEN;
    
    // Setup footer buttons for main screen
    std::vector<FooterButton> mainFooterButtons = {
        {"Home", homeAction},
        {"Off", showOffScreen},
        {"Rfrsh", refreshUI},
        {"Files", filesAction}
    };
    footer.setButtons(mainFooterButtons);
    
    renderCurrentScreen();
}

// Render the current screen with all elements
void renderCurrentScreen() {
    M5.Display.startWrite();

    // Устанавливаем универсальный шрифт перед отрисовкой
    setUniversalFont();

    rowsBuffer.clear();
    updateHeader();

    // Clear content area before drawing
    RowPosition contentStart = getRowPosition(2);
    RowPosition footerStart = getRowPosition(15);
    M5.Display.fillRect(contentStart.x, contentStart.y, contentStart.width, footerStart.y - contentStart.y, TFT_WHITE);

    switch(currentScreen) {
        case MAIN_SCREEN: {
            std::vector<FooterButton> mainFooterButtons = {
                {"Home", homeAction},
                {"Off", showOffScreen},
                {"Rfrsh", refreshUI},
                {"Files", filesAction}
            };
            footer.setButtons(mainFooterButtons);
            screens::drawMainScreen();
            break;
        }
        case FILES_SCREEN: {
            std::vector<FooterButton> filesFooterButtons = {
                {"Home", homeAction},
                {"Off", showOffScreen},
                {"Rfrsh", refreshUI},
                {"Files", filesAction}
            };
            footer.setButtons(filesFooterButtons);
            screens::drawFilesScreen();
            break;
        }
        case OFF_SCREEN:
            screens::drawOffScreen();
            break;
        case TXT_VIEWER_SCREEN:
        case IMG_VIEWER_SCREEN: {
            std::vector<FooterButton> viewerFooterButtons = {
                {"Home", homeAction},
                {"Off", showOffScreen},
                {"Freeze", freezeAction},
                {"Files", filesAction}
            };
            footer.setButtons(viewerFooterButtons);
            if (currentScreen == TXT_VIEWER_SCREEN) {
                screens::drawTxtViewerScreen(currentPath);
            } else {
                screens::drawImgViewerScreen(currentPath);
            }
            break;
        }
        case CLEAR_SCREEN:
            screens::drawClearScreen();
            break;
        case WIFI_SCREEN: // Handle Wi-Fi screen
            screens::drawWifiScreen();
            break;
        case APPS_SCREEN: // Handle Apps screen
            screens::drawAppsScreen();
            break;
        case TEXT_LANG_TEST_SCREEN: // Handle Text language font test app screen
            // Set footer buttons for text language test app screen
            {
                std::vector<FooterButton> appFooterButtons = {
                    {"Home", homeAction},
                    {"Off", showOffScreen},
                    {"", nullptr}, // Empty button for spacing
                    {"", nullptr}  // Empty button for spacing
                };
                footer.setButtons(appFooterButtons);
                apps_text_lang_test::drawAppScreen();
            }
            break;
        case TEST2_APP_SCREEN: // Handle Test2 app screen
            // Set footer buttons for test2 app screen
            {
                std::vector<FooterButton> appFooterButtons = {
                    {"Home", homeAction},
                    {"Off", showOffScreen},
                    {"", nullptr}, // Empty button for spacing
                    {"", nullptr}  // Empty button for spacing
                };
                footer.setButtons(appFooterButtons);
                apps_test2::drawAppScreen();
            }
            break;
        case SD_GATEWAY_SCREEN: {
            std::vector<FooterButton> sdgwFooterButtons = {
                {"Home", homeAction},
                {"Off", showOffScreen},
                {"Rfrsh", refreshUI},
                {"Files", filesAction}
            };
            footer.setButtons(sdgwFooterButtons);
            screens::drawSdGatewayScreen();
            break;
        }
        default:
            break;
    }

    // Draw buffered rows
    drawRowsBuffered();

    // Draw footer
    footer.draw(footer.isVisible());

    M5.Display.endWrite();
    if (!firstRenderDone) {
        // Первое обновление - полное
        M5.Display.display();
        firstRenderDone = true;
    } else {
        // Частичное обновление региона контента (от строки 2 до футера)
        RowPosition start = getRowPosition(2);
        RowPosition footerPos = getRowPosition(15);
        int regionY = start.y;
        int regionHeight = footerPos.y - start.y;
        M5.Display.display(start.x, regionY, start.width, regionHeight);
    }
}

// Update UI by clearing old messages
void updateUI() {
    if (currentMessage.text != "" && (millis() - currentMessage.timestamp > 1000)) { // 1 second
        clearMessage();
        renderCurrentScreen();
    }
}

// Display a message in header 2
void displayMessage(const String& msg) {
    currentMessage.text = msg;
    currentMessage.timestamp = millis();
    renderCurrentScreen();
}

// Clear the message in header 2
void clearMessage() {
    currentMessage.text = "";
    renderCurrentScreen();
}

// Функция для переноса длинных строк текста
std::vector<String> wordWrap(const String& text, int maxWidth) {
    std::vector<String> lines;
    if (text.length() == 0) {
        return lines;
    }
    
    // Устанавливаем шрифт для корректного расчета ширины текста
    ::setUniversalFont();
    
    String line = "";
    String word = "";
    int spaceWidth = M5.Display.textWidth(" ");
    
    for (unsigned int i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == ' ' || c == '\n') {
            // Если текущая строка + слово + пробел шире максимальной ширины
            if (line.length() > 0 && 
                M5.Display.textWidth((line + word + " ").c_str()) > maxWidth) {
                lines.push_back(line);
                line = word + " ";
            } else {
                line += word + " ";
            }
            word = "";
            
            // Если встретили перенос строки, завершаем текущую строку
            if (c == '\n') {
                line.trim();
                lines.push_back(line);
                line = "";
            }
        } else {
            word += c;
        }
    }
    
    // Добавляем последнее слово
    if (word.length() > 0) {
        // Если текущая строка + последнее слово шире максимальной ширины
        if (line.length() > 0 && 
            M5.Display.textWidth((line + word).c_str()) > maxWidth) {
            lines.push_back(line);
            line = word;
        } else {
            line += word;
        }
    }
    
    // Добавляем последнюю строку, если она не пустая
    if (line.length() > 0) {
        line.trim();
        lines.push_back(line);
    }
    
    return lines;
}

// Navigate to a specific path
void navigateTo(const String& path) {
    currentPath = path;
    currentScreen = FILES_SCREEN;
    screens::resetPagination(); // Use namespace to call resetPagination
    
    // Setup footer buttons for files screen
    std::vector<FooterButton> filesFooterButtons = {
        {"Home", homeAction},
        {"Off", showOffScreen},
        {"Rfrsh", refreshUI},
        {"Files", filesAction}
    };
    footer.setButtons(filesFooterButtons);
    
    renderCurrentScreen();
}

// Navigate up a directory
void navigateUp() {
    if (currentPath == "/") return;

    while (currentPath.length() > 1 && currentPath.endsWith("/")) {
        currentPath = currentPath.substring(0, currentPath.length() - 1);
    }

    int lastSlash = currentPath.lastIndexOf('/');
    currentPath = (lastSlash > 0) ? currentPath.substring(0, lastSlash + 1) : "/";
    currentScreen = FILES_SCREEN;
    screens::resetPagination(); // Use namespace to call resetPagination
    
    // Setup footer buttons for files screen
    std::vector<FooterButton> filesFooterButtons = {
        {"Home", homeAction},
        {"Off", showOffScreen},
        {"Rfrsh", refreshUI},
        {"Files", filesAction}
    };
    footer.setButtons(filesFooterButtons);
    
    renderCurrentScreen();
}