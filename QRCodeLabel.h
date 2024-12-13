#ifndef QRCODELABEL_H
#define QRCODELABEL_H

#include <Arduino.h>
#include <stdint.h>

namespace QRCodeLabel {
    // 初始化 OLED 顯示器
    void initialize(uint8_t i2c_address, uint8_t sda_pin, uint8_t scl_pin);

    // 顯示 QR Code 和底部文本（僅在內容變化時刷新）
    void show(const String& qrText, const String& labelText);
}

#endif
