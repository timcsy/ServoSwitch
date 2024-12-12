#ifndef HARDWARE_H
#define HARDWARE_H

namespace Hardware {
    void initialize();   // 初始化伺服馬達
    void standby();      // 待命模式
    void turnOn();       // 開啟開關(驅動上方伺服馬達)
    void turnOff();      // 關閉開關(驅動下方伺服馬達)
    void correction();   // 伺服馬達範圍以及 90 度校正
}

#endif
