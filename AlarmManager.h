#ifndef ALARMMANAGER_H
#define ALARMMANAGER_H

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
    void loadAlarms();
    void saveAlarms();
    void checkAlarms();

    void handleGetAlarms();
    void handleAddAlarm();
    void handleDeleteAlarm();
    void handleClearAlarms();
}

#endif
