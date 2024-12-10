#include "EEPROMStorage.h"
#include <EEPROM.h>
#include <Arduino.h>

// 定義 EEPROM 大小和魔術數字
#define EEPROM_SIZE 512
#define MAGIC_NUMBER 0x42

void EEPROMStorage::saveAlarms(const std::vector<Alarm>& alarms) {
    Serial.println("[INFO] Saving alarms to EEPROM");
    EEPROM.begin(EEPROM_SIZE);

    // 寫入魔術數字和鬧鐘數量
    EEPROM.write(0, MAGIC_NUMBER);
    EEPROM.write(1, alarms.size());

    // 從地址 2 開始存儲鬧鐘數據
    int addr = 2;
    for (const auto& alarm : alarms) {
        EEPROM.write(addr++, alarm.hour);
        EEPROM.write(addr++, alarm.minute);

        // 存儲重複設置
        for (int i = 0; i < sizeof(alarm.repeat); ++i) {
            EEPROM.write(addr++, alarm.repeat[i]);
        }

        // 存儲動作狀態、閃爍狀態和閃爍持續時間
        EEPROM.write(addr++, alarm.action);
        EEPROM.write(addr++, alarm.isBlink);

        // 存儲 Blink 持續時間（4 字節）
        EEPROM.write(addr++, (alarm.blinkDuration >> 24) & 0xFF);
        EEPROM.write(addr++, (alarm.blinkDuration >> 16) & 0xFF);
        EEPROM.write(addr++, (alarm.blinkDuration >> 8) & 0xFF);
        EEPROM.write(addr++, alarm.blinkDuration & 0xFF);
    }

    EEPROM.commit(); // 提交數據到 EEPROM
    Serial.println("[INFO] Alarms saved successfully");
}

void EEPROMStorage::loadAlarms(std::vector<Alarm>& alarms) {
    Serial.println("[INFO] Loading alarms from EEPROM");
    EEPROM.begin(EEPROM_SIZE);

    // 驗證魔術數字
    if (EEPROM.read(0) != MAGIC_NUMBER) {
        Serial.println("[WARNING] Invalid EEPROM data, initializing...");
        alarms.clear();
        return;
    }

    // 讀取鬧鐘數量
    int count = EEPROM.read(1);
    Serial.printf("[INFO] Found %d alarms in EEPROM\n", count);

    alarms.clear();
    int addr = 2;
    for (int i = 0; i < count; ++i) {
        Alarm alarm;

        // 讀取鬧鐘時間
        alarm.hour = EEPROM.read(addr++);
        alarm.minute = EEPROM.read(addr++);

        // 讀取重複設置
        for (int j = 0; j < sizeof(alarm.repeat); ++j) {
            alarm.repeat[j] = EEPROM.read(addr++);
        }

        // 讀取動作狀態、閃爍狀態和閃爍持續時間
        alarm.action = EEPROM.read(addr++);
        alarm.isBlink = EEPROM.read(addr++);

        // 讀取 Blink 持續時間（4 字節）
        alarm.blinkDuration = (EEPROM.read(addr++) << 24) |
                              (EEPROM.read(addr++) << 16) |
                              (EEPROM.read(addr++) << 8) |
                              EEPROM.read(addr++);

        alarms.push_back(alarm);
    }

    Serial.println("[INFO] Alarms loaded successfully");
}
