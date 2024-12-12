#include "AlarmManager.h"
#include "EEPROMStorage.h"
#include "BlinkManager.h"
#include "Hardware.h"
#include <time.h>
#include <ArduinoJson.h>

// 鬧鐘列表
std::vector<Alarm> alarms;

void AlarmManager::addAlarm(const Alarm& alarm) {
    alarms.push_back(alarm);
    saveAlarms(); // 每次添加新鬧鐘後保存
    Serial.println("[INFO] Alarm added successfully");
}

std::vector<Alarm> AlarmManager::getAlarms() {
    return alarms; // 返回所有鬧鐘
}

String AlarmManager::getAlarmsJSON() {
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
    Serial.println("[INFO] Convert alarms to JSON");
    return json;
}

void AlarmManager::loadAlarms() {
    Serial.println("[INFO] Loading alarms from EEPROM");
    EEPROMStorage::loadAlarms(alarms);
}

void AlarmManager::saveAlarms() {
    Serial.println("[INFO] Saving alarms to EEPROM");
    EEPROMStorage::saveAlarms(alarms);
}

bool AlarmManager::deleteAlarm(size_t index) {
    if (index < alarms.size()) {
        alarms.erase(alarms.begin() + index);
        saveAlarms(); // 保存變更
        Serial.printf("[INFO] Alarm [%d] deleted successfully\n", index);
        return true;
    }
    return false;
}

void AlarmManager::clearAlarms() {
    alarms.clear();
    saveAlarms(); // 保存變更
    Serial.println("[INFO] All alarms cleared successfully");
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
