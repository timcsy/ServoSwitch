#ifdef ESP32

#include "Communication.h"
#include "Hardware.h"
#include "AlarmManager.h"
#include "BlinkManager.h"
#include "QRCodeLabel.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// UUID 定義
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CMD_UUID "87654321-4321-4321-4321-987654321fed"
#define RESP_UUID "fedcba98-8765-4321-4321-123456789abc"

// BLE 相關變數
BLECharacteristic* commandCharacteristic = nullptr;
BLECharacteristic* responseCharacteristic = nullptr;
bool deviceConnected = false;

// 發送 BLE 回應
void sendResponse(const String& response) {
    if (deviceConnected && responseCharacteristic) {
        responseCharacteristic->setValue(response.c_str());
        responseCharacteristic->notify();
        Serial.printf("[INFO] Sent response: %s\n", response.c_str());
    }
}

// BLE 指令處理
void processCommand(const String& command) {
    if (command == "TURN_ON") {
        Hardware::turnOn();
        sendResponse("Device turned on");
    } else if (command == "TURN_OFF") {
        Hardware::turnOff();
        sendResponse("Device turned off");
    } else if (command.startsWith("BLINK:")) {
        int delimiter = command.indexOf(':');
        if (delimiter != -1) {
            uint32_t duration = command.substring(delimiter + 1).toInt();
            BlinkManager::startBlink(duration, 3000, true);
            sendResponse("Blink started");
        } else {
            sendResponse("Invalid BLINK command format");
        }
    } else if (command == "STOP_BLINK") {
        BlinkManager::stopBlink();
        sendResponse("Blink stopped");
    } else if (command == "CORRECTION") {
        Hardware::correction();
        sendResponse("Device correcting");
    } else if (command == "GET_ALARMS") {
        // 使用 JSON 傳遞鬧鐘列表
        String response = AlarmManager::getAlarmsJSON();
        sendResponse(response);
    } else if (command.startsWith("ADD_ALARM:")) {
        // 格式: ADD_ALARM:<hour>:<minute>:<repeat>:<action>:<isBlink>:<blinkDuration>
        String tokens[7];
        int start = 0, end, tokenIndex = 0;
        while ((end = command.indexOf(':', start)) != -1 && tokenIndex < 7) {
            tokens[tokenIndex++] = command.substring(start, end);
            start = end + 1;
        }
        tokens[tokenIndex++] = command.substring(start);

        if (tokenIndex == 7) {
            Alarm alarm;
            alarm.hour = tokens[1].toInt();
            alarm.minute = tokens[2].toInt();
            strncpy(alarm.repeat, tokens[3].c_str(), sizeof(alarm.repeat) - 1);
            alarm.action = (tokens[4] == "true");
            alarm.isBlink = (tokens[5] == "true");
            alarm.blinkDuration = tokens[6].toInt();

            AlarmManager::addAlarm(alarm);
            sendResponse("Alarm added successfully");
        } else {
            sendResponse("Invalid ADD_ALARM format");
        }
    } else if (command.startsWith("DELETE_ALARM:")) {
        // 格式: DELETE_ALARM:<index>
        int delimiter = command.indexOf(':');
        if (delimiter != -1) {
            int index = command.substring(delimiter + 1).toInt();
            if (AlarmManager::deleteAlarm(index)) {
                sendResponse("Alarm deleted successfully");
            } else {
                sendResponse("Failed to delete alarm: invalid index");
            }
        } else {
            sendResponse("Invalid DELETE_ALARM command format");
        }
    } else if (command == "CLEAR_ALARMS") {
        AlarmManager::clearAlarms();
        sendResponse("All alarms cleared successfully");
    } else {
        sendResponse("Unknown command");
    }
}

// BLE 回調處理類
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("[INFO] BLE device connected");
    }

    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("[INFO] BLE device disconnected");
    }
};

class CommandCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) override {
        String command = characteristic->getValue().c_str();
        if (command.isEmpty()) return;

        Serial.printf("[INFO] Received command: %s\n", command.c_str());
        processCommand(command);
    }
};

// 初始化 BLE
void Communication::initializeBLE() {
    Serial.println("[INFO] Initializing BLE...");
    BLEDevice::init("Servo Switch");

    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    BLEService* service = server->createService(SERVICE_UUID);

    // 初始化指令特徵
    commandCharacteristic = service->createCharacteristic(
        CMD_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    commandCharacteristic->setCallbacks(new CommandCallbacks());

    // 初始化回應特徵
    responseCharacteristic = service->createCharacteristic(
        RESP_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    responseCharacteristic->addDescriptor(new BLE2902());

    service->start();

    // 開始廣播
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();

    Serial.println("[INFO] BLE initialized and advertising started");

    QRCodeLabel::show("https://timcsy.github.io/ServoSwitch/", "Device: Servo Switch");
}
#endif
