#ifndef BLINKMANAGER_H
#define BLINKMANAGER_H

#include <stdint.h>

namespace BlinkManager {
    void startBlink(uint32_t duration, uint32_t interval, bool finalState); // 開始閃爍
    void updateBlink(); // 更新閃爍狀態
    void stopBlink();   // 停止閃爍
}

#endif
