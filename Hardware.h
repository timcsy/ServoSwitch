#ifndef HARDWARE_H
#define HARDWARE_H

namespace Hardware {
    void initialize();   // 初始化伺服器控制
    void standby();      // 伺服器待機模式
    void turnOn();       // 開啟伺服器
    void turnOff();      // 關閉伺服器
}

#endif
