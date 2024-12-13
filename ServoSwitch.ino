#include "Hardware.h"
#include "Communication.h"
#include "AlarmManager.h"
#include "BlinkManager.h"
#include "QRCodeLabel.h"

void setup() {
    Serial.begin(115200);

#ifdef ESP8266
    // OLED 螢幕設定
    QRCodeLabel::initialize(0x3c, D2, D1); // SDA = D2, SCL = D1
    pinMode(D5, OUTPUT);
    digitalWrite(D5, HIGH);  // VCC = D5
#elif defined(ESP32)
    // OLED 螢幕設定
    QRCodeLabel::initialize(0x3c, 21, 22); // SDA = 21, SCL = 22
    pinMode(19, OUTPUT);
    digitalWrite(19, HIGH);  // VCC = 19
#endif

    // 初始化伺服器控制
    Hardware::initialize();

#ifdef ESP8266
    // 初始化 Wi-Fi（包含 WiFiManager）
    Communication::initializeWiFi();
    // 初始化 HTTP
    Communication::initializeHTTP();
#elif defined(ESP32)
    // 初始化 BLE
    Communication::initializeBLE();
#endif

    // 載入鬧鐘設定
    AlarmManager::loadAlarms();
}

void loop() {
#ifdef ESP8266
    Communication::checkWiFi();
    // 處理 HTTP 請求
    Communication::handleHTTPClient();
#endif

    // 檢查鬧鐘
    AlarmManager::checkAlarms();

    // 更新閃爍狀態
    BlinkManager::updateBlink();
}
