#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <time.h>
#include "Secret.h"

// 宏定義
#define EEPROM_SIZE 512
#define MAGIC_NUMBER 0x42  // 用於驗證 EEPROM 資料有效性

// WiFi 資訊
const char* ssid = WIFI_SSID;      // WiFi 名稱
const char* password = WIFI_PASSWORD;     // WiFi 密碼

ESP8266WebServer server(80);
Servo up, down; // Servo 宣告

struct Alarm {
  uint8_t hour;
  uint8_t minute;
  char repeat[10];
  bool action;
};

std::vector<Alarm> alarms;

// 函數聲明
void standby();
void turnOn();
void turnOff();
void saveAlarms();
void loadAlarms();
void checkAlarms();
void handleRoot();
void handleGetAlarms();
void handleAddAlarm();
void handleDeleteAlarm();
void handleClearAlarms();
void handleTurnOn();
void handleTurnOff();

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Alarm Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link href="https://cdn.jsdelivr.net/npm/vuetify@2.6.4/dist/vuetify.min.css" rel="stylesheet">
  <script src="https://cdn.jsdelivr.net/npm/vue@2.6.14"></script>
  <script src="https://cdn.jsdelivr.net/npm/vuetify@2.6.4"></script>
</head>
<body>
  <div id="app">
    <v-app>
      <v-container>
        <v-card class="mb-5">
          <v-card-title>Immediate Control</v-card-title>
          <v-card-actions>
            <v-btn color="green" @click="turnOn">Turn On</v-btn>
            <v-btn color="red" @click="turnOff">Turn Off</v-btn>
          </v-card-actions>
        </v-card>
        <v-card class="mb-5">
          <v-card-title>Add New Alarm</v-card-title>
          <v-card-text>
            <v-row>
              <v-col cols="12" sm="6">
                <v-time-picker v-model="newAlarmTime" full-width></v-time-picker>
              </v-col>
              <v-col cols="12" sm="6">
                <v-select
                  v-model="newAlarm.repeat"
                  :items="repeatOptions"
                  item-value="value"
                  item-text="text"
                  label="Repeat"
                ></v-select>
                <v-select
                  v-model="newAlarm.action"
                  :items="actionOptions"
                  item-value="value"
                  item-text="text"
                  label="Action"
                ></v-select>
              </v-col>
            </v-row>
          </v-card-text>
          <v-card-actions>
            <v-btn color="primary" @click="addAlarm">Add Alarm</v-btn>
          </v-card-actions>
        </v-card>
        <v-card>
          <v-card-title>Alarm List</v-card-title>
          <v-card-text>
            <v-list>
              <v-list-item v-for="(alarm, index) in alarms" :key="index">
                <v-list-item-content>
                  <v-list-item-title>
                    {{ formatTime(alarm.hour, alarm.minute) }} - {{ formatAction(alarm.action) }} ({{ formatRepeat(alarm.repeat) }})
                  </v-list-item-title>
                </v-list-item-content>
                <v-list-item-action>
                  <v-btn color="red" @click="deleteAlarm(index)">Delete</v-btn>
                </v-list-item-action>
              </v-list-item>
            </v-list>
          </v-card-text>
          <v-card-actions>
            <v-btn color="blue" @click="clearAlarms">Clear All Alarms</v-btn>
          </v-card-actions>
        </v-card>
      </v-container>
    </v-app>
  </div>
  <script>
    new Vue({
      el: '#app',
      vuetify: new Vuetify(),
      data: {
        alarms: [],
        newAlarm: {
          repeat: 'once', // 預設為 Once
          action: 'on',
        },
        newAlarmTime: '12:00',
        repeatOptions: [
          { value: 'daily', text: 'Daily' },
          { value: 'hourly', text: 'Hourly' },
          { value: 'weekly', text: 'Weekly' },
          { value: 'once', text: 'Once' },
        ],
        actionOptions: [
          { value: 'on', text: 'Turn On' },
          { value: 'off', text: 'Turn Off' },
        ],
      },
      created() {
        this.loadAlarms();
      },
      methods: {
        formatTime(hour, minute) {
          const period = hour >= 12 ? 'pm' : 'am';
          const formattedHour = hour % 12 || 12;
          const formattedMinute = minute.toString().padStart(2, '0');
          return `${formattedHour}:${formattedMinute} ${period}`;
        },
        formatAction(action) {
          return action === true || action === 'on' ? 'Turn On' : 'Turn Off';
        },
        formatRepeat(repeat) {
          return repeat.charAt(0).toUpperCase() + repeat.slice(1);
        },
        async loadAlarms() {
          const response = await fetch('/get-alarms');
          this.alarms = await response.json();
        },
        async addAlarm() {
          const [hour, minute] = this.newAlarmTime.split(':').map(Number);
          this.newAlarm.hour = hour;
          this.newAlarm.minute = minute;
          const params = new URLSearchParams(this.newAlarm);
          await fetch('/add-alarm', {
            method: 'POST',
            body: params,
          });
          this.loadAlarms();
        },
        async deleteAlarm(index) {
          await fetch(`/delete-alarm?index=${index}`, { method: 'POST' });
          this.loadAlarms();
        },
        async clearAlarms() {
          await fetch('/clear-alarms', { method: 'POST' });
          this.loadAlarms();
        },
        async turnOn() {
          await fetch('/turn-on', { method: 'POST' });
        },
        async turnOff() {
          await fetch('/turn-off', { method: 'POST' });
        },
      },
    });
  </script>
</body>
</html>
)rawliteral";

// ======== Servo 控制函數 ========
void standby() {
  Serial.println("[INFO] Servo entering standby mode");
  up.write(0);
  down.write(180);
}

void turnOn() {
  Serial.println("[INFO] Servo turning ON");
  up.write(120);
  down.write(180);
  delay(500);
  standby();
}

void turnOff() {
  Serial.println("[INFO] Servo turning OFF");
  up.write(0);
  down.write(60);
  delay(500);
  standby();
}

// ======== EEPROM 功能 ========
void saveAlarms() {
  Serial.println("[INFO] Saving alarms to EEPROM");
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, MAGIC_NUMBER);
  EEPROM.write(1, alarms.size());

  int addr = 2;
  for (const auto& alarm : alarms) {
    EEPROM.write(addr++, alarm.hour);
    EEPROM.write(addr++, alarm.minute);
    for (int i = 0; i < 10; i++) {
      EEPROM.write(addr++, alarm.repeat[i]);
    }
    EEPROM.write(addr++, alarm.action);
  }

  EEPROM.commit();
  Serial.println("[INFO] Alarms saved successfully");
}

void loadAlarms() {
  Serial.println("[INFO] Loading alarms from EEPROM");
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) != MAGIC_NUMBER) {
    Serial.println("[INFO] Invalid EEPROM data. Initializing...");
    alarms.clear();
    saveAlarms();
    return;
  }

  int count = EEPROM.read(1);
  Serial.printf("[INFO] Found %d alarms in EEPROM\n", count);

  alarms.clear();
  int addr = 2;
  for (int i = 0; i < count; i++) {
    Alarm alarm;
    alarm.hour = EEPROM.read(addr++);
    alarm.minute = EEPROM.read(addr++);
    for (int j = 0; j < 10; j++) {
      alarm.repeat[j] = EEPROM.read(addr++);
    }
    alarm.action = EEPROM.read(addr++);
    alarms.push_back(alarm);
  }
  Serial.println("[INFO] Alarms loaded successfully");
}

// ======== 鬧鐘觸發邏輯 ========
void checkAlarms() {
  time_t now = time(nullptr);
  struct tm* currentTime = localtime(&now);

  int currentHour = currentTime->tm_hour;
  int currentMinute = currentTime->tm_min;

  for (int i = 0; i < alarms.size(); i++) {
    Alarm& alarm = alarms[i];
    bool shouldTrigger = (alarm.hour == currentHour && alarm.minute == currentMinute);

    if (shouldTrigger) {
      Serial.printf("[INFO] Alarm triggered: %02d:%02d (Repeat: %s)\n", alarm.hour, alarm.minute, alarm.repeat);
      if (alarm.action) {
        turnOn();
      } else {
        turnOff();
      }

      standby();

      if (strcmp(alarm.repeat, "once") == 0) {
        alarms.erase(alarms.begin() + i);
        saveAlarms();
        i--; // 調整索引
      }
    }
  }
}

// ======== HTTP 請求處理 ========
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
  Serial.println("[INFO] Root page served");
}

void handleGetAlarms() {
  StaticJsonDocument<1024> doc;
  for (const auto& alarm : alarms) {
    JsonObject obj = doc.createNestedObject();
    obj["hour"] = alarm.hour;
    obj["minute"] = alarm.minute;
    obj["repeat"] = alarm.repeat;
    obj["action"] = alarm.action; // 確保 action 是 true/false 布林值
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
  Serial.println("[INFO] Sent alarm list to client");
}

void handleAddAlarm() {
  if (!server.hasArg("hour") || !server.hasArg("minute") || !server.hasArg("repeat") || !server.hasArg("action")) {
    server.send(400, "text/plain", "Missing required fields");
    Serial.println("[INFO] Failed to add alarm: Missing fields");
    return;
  }

  Alarm alarm = {
    (uint8_t)server.arg("hour").toInt(),
    (uint8_t)server.arg("minute").toInt(),
  };
  strncpy(alarm.repeat, server.arg("repeat").c_str(), 10);
  alarm.action = (server.arg("action") == "on");

  alarms.push_back(alarm);
  saveAlarms();
  server.send(200, "text/plain", "Alarm added");
  Serial.printf("[INFO] Alarm added: %02d:%02d, Repeat: %s, Action: %s\n",
                alarm.hour, alarm.minute, alarm.repeat, alarm.action ? "ON" : "OFF");
}

void handleDeleteAlarm() {
  if (!server.hasArg("index")) {
    server.send(400, "text/plain", "Missing index parameter");
    Serial.println("[INFO] Failed to delete alarm: Missing index");
    return;
  }

  uint8_t index = server.arg("index").toInt();
  if (index >= alarms.size()) {
    server.send(400, "text/plain", "Invalid index");
    Serial.printf("[INFO] Invalid index for delete: %d\n", index);
    return;
  }

  alarms.erase(alarms.begin() + index);
  saveAlarms();
  server.send(200, "text/plain", "Alarm deleted");
  Serial.printf("[INFO] Alarm deleted at index %d\n", index);
}

void handleClearAlarms() {
  alarms.clear();
  saveAlarms();
  server.send(200, "text/plain", "All alarms cleared");
  Serial.println("[INFO] All alarms cleared");
}

void handleTurnOn() {
  turnOn();
  server.send(200, "text/plain", "Turned On");
  Serial.println("[INFO] Turn ON command received");
}

void handleTurnOff() {
  turnOff();
  server.send(200, "text/plain", "Turned Off");
  Serial.println("[INFO] Turn OFF command received");
}

// ======== 設置 ========
void setup() {
  Serial.begin(115200);

  up.attach(D4, 0, 10000);
  down.attach(D8, 0, 10000);
  standby();

  if (password[0] == '\0') WiFi.begin(ssid);
  else WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n[INFO] WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());

  loadAlarms();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/get-alarms", HTTP_GET, handleGetAlarms);
  server.on("/add-alarm", HTTP_POST, handleAddAlarm);
  server.on("/delete-alarm", HTTP_POST, handleDeleteAlarm);
  server.on("/clear-alarms", HTTP_POST, handleClearAlarms);
  server.on("/turn-on", HTTP_POST, handleTurnOn);
  server.on("/turn-off", HTTP_POST, handleTurnOff);

  server.begin();

  configTime(28800, 0, "pool.ntp.org");
  Serial.println("[INFO] Server started");
}

// ======== 主程式迴圈 ========
void loop() {
  server.handleClient();
  checkAlarms();
}
