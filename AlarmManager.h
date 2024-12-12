#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

#include <Arduino.h>
#include <stdint.h>
#include <vector>

struct Alarm {
    uint8_t hour;
    uint8_t minute;
    char repeat[10];
    bool action;
    bool isBlink;
    uint32_t blinkDuration;
};

namespace AlarmManager {
    void addAlarm(const Alarm& alarm);           // 添加鬧鐘
    std::vector<Alarm> getAlarms();              // 獲取所有鬧鐘
    String getAlarmsJSON();
    void loadAlarms();
    void saveAlarms();
    bool deleteAlarm(size_t index);
    void clearAlarms();
    void checkAlarms();
}

#endif
