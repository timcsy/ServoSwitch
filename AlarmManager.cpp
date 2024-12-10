#include "AlarmManager.h"
#include "EEPROMStorage.h"
#include "BlinkManager.h"
#include "Hardware.h"
#include <time.h>
#include <ArduinoJson.h>

#ifdef ESP8266
#include <ESP8266WebServer.h>
extern ESP8266WebServer server;
#elif defined(ESP32)
#include <WebServer.h>
extern WebServer server;
#endif

// 鬧鐘列表
std::vector<Alarm> alarms;

void AlarmManager::addAlarm(const Alarm& alarm) {
    alarms.push_back(alarm);
    saveAlarms(); // 每次添加新鬧鐘後保存
}

std::vector<Alarm> AlarmManager::getAlarms() {
    return alarms; // 返回所有鬧鐘
}

void AlarmManager::loadAlarms() {
    Serial.println("[INFO] Loading alarms from EEPROM");
    EEPROMStorage::loadAlarms(alarms);
}

void AlarmManager::saveAlarms() {
    Serial.println("[INFO] Saving alarms to EEPROM");
    EEPROMStorage::saveAlarms(alarms);
}

void AlarmManager::checkAlarms() {
    time_t now = time(nullptr);
    struct tm* currentTime = localtime(&now);

    int currentHour = currentTime->tm_hour;
    int currentMinute = currentTime->tm_min;

    for (size_t i = 0; i < alarms.size(); ++i) {
        Alarm& alarm = alarms[i];
        bool shouldTrigger = (alarm.hour == currentHour && alarm.minute == currentMinute);

        if (shouldTrigger) {
            if (alarm.isBlink) {
                BlinkManager::startBlink(alarm.blinkDuration, 3000, alarm.action);
            } else if (alarm.action) {
                Hardware::turnOn();
            } else {
                Hardware::turnOff();
            }

            // 如果是一次性鬧鐘，觸發後刪除
            if (strcmp(alarm.repeat, "once") == 0) {
                alarms.erase(alarms.begin() + i);
                saveAlarms();
                --i; // 調整索引，避免跳過下一個鬧鐘
            }
        }
    }
}

void AlarmManager::handleGetAlarms() {
    StaticJsonDocument<1024> doc;
    for (const auto& alarm : alarms) {
        JsonObject obj = doc.createNestedObject();
        obj["hour"] = alarm.hour;
        obj["minute"] = alarm.minute;
        obj["repeat"] = alarm.repeat;
        obj["action"] = alarm.action;
        obj["isBlink"] = alarm.isBlink;
        obj["blinkDuration"] = alarm.blinkDuration;
    }

    String json;
    serializeJson(doc, json);
    Serial.println("[INFO] Sending alarms as JSON");
    server.send(200, "application/json", json);
}

void AlarmManager::handleAddAlarm() {
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

    alarms.push_back(alarm);
    saveAlarms();
    server.send(200, "text/plain", "Alarm added");
    Serial.println("[INFO] Alarm added successfully");
}

void AlarmManager::handleDeleteAlarm() {
    if (!server.hasArg("index")) {
        server.send(400, "text/plain", "Missing index parameter");
        return;
    }

    uint8_t index = server.arg("index").toInt();
    if (index >= alarms.size()) {
        server.send(400, "text/plain", "Invalid index");
        return;
    }

    alarms.erase(alarms.begin() + index);
    saveAlarms();
    server.send(200, "text/plain", "Alarm deleted");
    Serial.println("[INFO] Alarm deleted successfully");
}

void AlarmManager::handleClearAlarms() {
    alarms.clear();
    saveAlarms();
    server.send(200, "text/plain", "All alarms cleared");
    Serial.println("[INFO] All alarms cleared successfully");
}
