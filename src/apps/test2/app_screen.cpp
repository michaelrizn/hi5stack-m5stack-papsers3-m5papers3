#include "app_screen.h"
#include "../../ui.h"

namespace apps_test2 {
    void drawAppScreen() {
        ::updateHeader();

        
        RowPosition contentStart = getRowPosition(2);
        RowPosition footerStart = getRowPosition(15);
        M5.Display.fillRect(contentStart.x, contentStart.y, contentStart.width, footerStart.y - contentStart.y, TFT_WHITE);

        
        ::bufferRow("Test2", 5, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL * 2, false);
    }
}