#include "sdcard.h"
#include "debug_config.h"

String sdCardStatus = "SD: Unknown";

bool initSDCard() {
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN, SPI, 15000000)) {
        Serial.println("[Error] SD initialization failed.");
        sdCardStatus = "SD: Error";
        return false;
    }
    Serial.println("[Info] SD initialized successfully.");
    sdCardStatus = "SD: Connected";
    return true;
}

bool isSDCardMounted() {

    if (!SD.exists("/")) {
        Serial.println("[Error] SD card not mounted.");
        sdCardStatus = "SD: Disconnected";
        return false;
    }
    sdCardStatus = "SD: Connected";
    return true;
}

void updateSDCardStatus() {
    isSDCardMounted();
}

void listFiles(const char* dirname) {
    File root = SD.open(dirname);
    if (!root) {
        Serial.println("[Error] Failed to open directory!");
        return;
    }
    while (true) {
        File file = root.openNextFile();
        if (!file) {
            break;
        }
        #ifdef DEBUG_FILES
        Serial.print(file.name());
        if (file.isDirectory()) {
            Serial.println("/");
        } else {
            Serial.printf(" (file, %llu bytes)\n", file.size());
        }
        #endif
        file.close();
    }
}
