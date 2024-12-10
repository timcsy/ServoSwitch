#include "BlinkManager.h"
#include "Hardware.h"
#include <Arduino.h>

// Blink 狀態變數
static bool blinking = false;
static bool blinkStop = false;
static bool blinkState = false;
static bool blinkFinalState = false;
static unsigned long blinkStartTime = 0;
static unsigned long lastBlinkTime = 0;
static uint32_t blinkDuration = 0;
static uint32_t blinkInterval = 3000;

void BlinkManager::startBlink(uint32_t duration, uint32_t interval, bool finalState) {
    Serial.printf("[INFO] Starting blink: duration=%dms, interval=%dms, finalState=%s\n",
                  duration, interval, finalState ? "ON" : "OFF");
    blinking = true;
    blinkStop = false;
    blinkState = false;
    blinkFinalState = finalState;
    blinkStartTime = millis();
    lastBlinkTime = millis();
    blinkDuration = duration;
    blinkInterval = interval;
}

void BlinkManager::updateBlink() {
    if (!blinking) return;

    unsigned long currentTime = millis();

    // 停止 Blink 的條件
    if (blinkStop || (currentTime - blinkStartTime >= blinkDuration)) {
        blinking = false;
        if (!blinkStop) {
            if (blinkFinalState) {
                Hardware::turnOn();
            } else {
                Hardware::turnOff();
            }
        }
        Serial.println("[INFO] Blink completed or interrupted");
        return;
    }

    // 切換 Blink 狀態
    if (currentTime - lastBlinkTime >= blinkInterval) {
        lastBlinkTime = currentTime;
        if (blinkState) {
            Hardware::turnOff();
        } else {
            Hardware::turnOn();
        }
        blinkState = !blinkState;
    }
}

void BlinkManager::stopBlink() {
    Serial.println("[INFO] Stopping blink");
    blinkStop = true;
    blinking = false;
    Hardware::standby(); // 確保設備回到待機狀態
}
