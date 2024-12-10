#include "Communication.h"
#include "AlarmManager.h"
#include "BlinkManager.h"
#include "Hardware.h"
#include "HTML.h"


#ifdef ESP8266

#include <WiFiManager.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
WiFiManager wifiManager;

void Communication::initializeWiFi() {
		Serial.println("[INFO] Initializing WiFiManager...");
		
		// 使用 WiFiManager 進行自動連接，進入配置模式時創建 AP
		if (!wifiManager.autoConnect("Alarm_Device")) {
				Serial.println("[ERROR] Failed to connect or timeout occurred!");
				ESP.restart();
		}

		Serial.print("[INFO] Connected to WiFi! IP Address: ");
		Serial.println(WiFi.localIP());
}

// #include <WiFi.h>
// #include <WebServer.h>
// #include "Secret.h"

// WebServer server(80);

// void Communication::initializeWiFi() {
//     const char* ssid = WIFI_SSID;
//     const char* password = WIFI_PASSWORD;
// 		if (password[0] == '\0') WiFi.begin(ssid);
//     else WiFi.begin(ssid, password);

//     while (WiFi.status() != WL_CONNECTED) {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.printf("\nWiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
// }

void Communication::initializeHTTP() {
    Serial.println("[INFO] Initializing HTTP server");

    // 網頁首頁
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", INDEX_HTML);
    });

    // 獲取所有鬧鐘
    server.on("/get-alarms", HTTP_GET, []() {
        AlarmManager::handleGetAlarms();
    });

    // 新增鬧鐘
    server.on("/add-alarm", HTTP_POST, []() {
        AlarmManager::handleAddAlarm();
    });

    // 刪除鬧鐘
    server.on("/delete-alarm", HTTP_POST, []() {
        AlarmManager::handleDeleteAlarm();
    });

    // 清除所有鬧鐘
    server.on("/clear-alarms", HTTP_POST, []() {
        AlarmManager::handleClearAlarms();
    });

    // 開啟設備
    server.on("/turn-on", HTTP_POST, []() {
        Hardware::turnOn();
        server.send(200, "text/plain", "Device turned on");
    });

    // 關閉設備
    server.on("/turn-off", HTTP_POST, []() {
        Hardware::turnOff();
        server.send(200, "text/plain", "Device turned off");
    });

    // 開始閃爍
    server.on("/blink", HTTP_POST, []() {
        if (!server.hasArg("duration") || !server.hasArg("finalState")) {
            server.send(400, "text/plain", "Missing duration or finalState");
            return;
        }

        uint32_t duration = server.arg("duration").toInt();
        bool finalState = (server.arg("finalState") == "true");
        BlinkManager::startBlink(duration, 3000, finalState);
        server.send(200, "text/plain", "Blink started");
    });

    // 停止閃爍
    server.on("/stop", HTTP_POST, []() {
        BlinkManager::stopBlink();
        server.send(200, "text/plain", "Blink stopped");
    });

    // 啟動 HTTP 服務
    server.begin();
    Serial.println("[INFO] HTTP server initialized");

    configTime(28800, 0, "pool.ntp.org");
}

void Communication::handleHTTPClient() {
    server.handleClient();
}

#endif
