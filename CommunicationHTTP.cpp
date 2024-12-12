#ifdef ESP8266

#include "Communication.h"
#include "Network.h"
#include "AlarmManager.h"
#include "BlinkManager.h"
#include "Hardware.h"
#include "HTML.h"

Network network("Servo Switch");
ESP8266WebServer server(80);

void Communication::initializeWiFi() {
		network.setup();
}

void Communication::checkWiFi() {
		network.loop();
}

void Communication::initializeHTTP() {
    Serial.println("[INFO] Initializing HTTP server");

    // 網頁首頁
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", INDEX_HTML);
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

    // 校正設備
    server.on("/correction", HTTP_POST, []() {
        Hardware::correction();
        server.send(200, "text/plain", "Device correcting");
    });

    // 獲取所有鬧鐘
    server.on("/get-alarms", HTTP_GET, []() {
        String response = AlarmManager::getAlarmsJSON();
        server.send(200, "application/json", response);
    });

    // 新增鬧鐘
    server.on("/add-alarm", HTTP_POST, []() {
        if (!server.hasArg("hour") || !server.hasArg("minute") || !server.hasArg("repeat") || !server.hasArg("action")) {
            server.send(400, "text/plain", "Missing required fields");
            return;
        }

        Alarm alarm;
        alarm.hour = server.arg("hour").toInt();
        alarm.minute = server.arg("minute").toInt();
        strncpy(alarm.repeat, server.arg("repeat").c_str(), sizeof(alarm.repeat) - 1);
        alarm.action = (server.arg("action") == "true");
        alarm.isBlink = (server.hasArg("isBlink") && server.arg("isBlink") == "true");
        alarm.blinkDuration = alarm.isBlink ? server.arg("blinkDuration").toInt() : 0;

        AlarmManager::addAlarm(alarm);
        server.send(200, "text/plain", "Alarm added");
    });

    // 刪除鬧鐘
    server.on("/delete-alarm", HTTP_POST, []() {
        if (!server.hasArg("index")) {
            server.send(400, "text/plain", "Missing index parameter");
            return;
        }

        uint8_t index = server.arg("index").toInt();
        if (index >= AlarmManager::getAlarms().size()) {
            server.send(400, "text/plain", "Invalid index");
            return;
        }

        AlarmManager::deleteAlarm(index);
        server.send(200, "text/plain", "Alarm deleted");
    });

    // 清除所有鬧鐘
    server.on("/clear-alarms", HTTP_POST, []() {
        AlarmManager::clearAlarms();
        server.send(200, "text/plain", "All alarms cleared");
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
