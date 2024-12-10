#ifndef EEPROMSTORAGE_H
#define EEPROMSTORAGE_H

#include <vector>
#include "AlarmManager.h"

namespace EEPROMStorage {
    void saveAlarms(const std::vector<Alarm>& alarms); // 保存鬧鐘
    void loadAlarms(std::vector<Alarm>& alarms);       // 加載鬧鐘
}

#endif
