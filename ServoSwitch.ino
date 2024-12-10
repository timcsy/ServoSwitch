#include "Hardware.h"
#include "Communication.h"
#include "AlarmManager.h"
#include "BlinkManager.h"

void setup() {
    Serial.begin(115200);

    // 初始化伺服器控制
    Hardware::initialize();

    #ifdef ESP8266
    // 初始化 Wi-Fi（包含 WiFiManager）
    Communication::initializeWiFi();

    // 初始化 HTTP 和 BLE
    Communication::initializeHTTP();
    #endif
    Communication::initializeBLE();

    // 載入鬧鐘設定
    AlarmManager::loadAlarms();
}

void loop() {
    #ifdef ESP8266
    // 處理 HTTP 請求
    Communication::handleHTTPClient();
    #endif

    // 檢查鬧鐘
    AlarmManager::checkAlarms();

    // 更新閃爍狀態
    BlinkManager::updateBlink();
}
