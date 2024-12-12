#include "Hardware.h"
#ifdef ESP8266
#include <Servo.h>
#elif defined(ESP32)
#include <ESP32Servo.h>
#endif

Servo up, down;

void Hardware::initialize() {
#ifdef ESP8266
    up.attach(D4, 0, 10000);
    down.attach(D8, 0, 10000);
#elif defined(ESP32)
    up.attach(2, 0, 10000);
    down.attach(15, 0, 10000);
#endif
    standby();
}

void Hardware::standby() {
    up.write(0);
    down.write(180);
}

void Hardware::turnOn() {
    Serial.println("[INFO] Servo turning ON");
    up.write(120);
    down.write(180);
    delay(500);
    standby();
}

void Hardware::turnOff() {
    Serial.println("[INFO] Servo turning OFF");
    up.write(0);
    down.write(60);
    delay(500);
    standby();
}

void Hardware::correction() {
    Serial.println("[INFO] Servo degree from 0 to 180");
    for (int angle = 0; angle <= 180; angle += 10) { // 每次增加 10 度
        up.write(angle);
        down.write(angle);
        delay(500); // 等待 0.5 秒觀察角度
    }
    up.write(90);
    down.write(90);
    Serial.println("[INFO] Servo corrected to center position");
}
