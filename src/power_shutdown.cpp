#include "power_shutdown.h"

#include <M5Unified.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_sleep.h>

namespace power_shutdown {
    void fallbackDeepSleep() {
        Serial.println("[Power] powerOff returned, fallback deep sleep");
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
        esp_deep_sleep_start();
    }

    void powerOff() {
        Serial.printf("[Power] board=%d pmic=%d\n", static_cast<int>(M5.getBoard()), static_cast<int>(M5.Power.getType()));
        Serial.println("[Power] preparing peripherals for power off");

        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        SD.end();
        SPI.end();

        M5.Power.setLed(0);
        M5.Power.setExtOutput(false);
        M5.Display.waitDisplay();
        M5.Display.sleep();
        M5.Display.waitDisplay();

        Serial.println("[Power] calling M5.Power.powerOff()");
        Serial.flush();
        M5.Power.powerOff();

        fallbackDeepSleep();
    }
}
