#include "Hardware.h"
#ifdef ESP8266
#include <Servo.h>
#elif defined(ESP32)
#include <ESP32Servo.h>
#endif

Servo up, down;

void Hardware::initialize() {
    up.attach(2, 0, 10000); // D4 = 2
    down.attach(15, 0, 10000); // D8 = 15
    standby();
}

void Hardware::standby() {
    up.write(0);
    down.write(180);
}

void Hardware::turnOn() {
    up.write(120);
    down.write(180);
    delay(500);
    standby();
}

void Hardware::turnOff() {
    up.write(0);
    down.write(60);
    delay(500);
    standby();
}
