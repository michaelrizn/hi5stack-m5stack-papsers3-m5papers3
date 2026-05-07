#include "main_screen.h"
#include "../ui.h"
#include "../battery.h"
#include "../sdcard.h"
#include "../sd_gateway.h"
#include <WiFi.h>

namespace screens {
    void drawMainScreen() {
        ::bufferRow("hi5stack", 2, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        ::bufferRow("Voltage: " + String(getBatteryVoltage(), 2) + "V", 3, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        ::bufferRow("SD Card: " + String(isSDCardMounted() ? "Mounted" : "Not Mounted"), 4, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);


        if (WiFi.status() == WL_CONNECTED) {
            String ip = WiFi.localIP().toString();
            ::bufferRow("Wi-Fi: " + ip, 5, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);
        } else {
            ::bufferRow("Wi-Fi: Not connected", 5, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);
        }


        String sdgwState = sd_gateway::isActive() ? "On port :8080" : "Off";
        ::bufferRow("SD Gateway: " + sdgwState, 6, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);

        ::bufferRow("Apps", 7, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);

        ::bufferRow("Games", 8, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, true);

        for (int row = 9; row <= 13; ++row) {
            ::bufferRow("", row, TFT_BLACK, TFT_WHITE, FONT_SIZE_ALL, false);
        }
    }
}
