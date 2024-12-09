#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <time.h>
#include "Secret.h"

#define EEPROM_SIZE 512
#define MAGIC_NUMBER 0x42

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

ESP8266WebServer server(80);
Servo up, down;

struct Alarm {
  uint8_t hour;
  uint8_t minute;
  char repeat[10];
  bool action;           // 最終狀態（true: Turn On, false: Turn Off）
  bool isBlink;          // 是否執行 Blink
  uint16_t blinkDuration; // Blink 持續時間（毫秒）
};

std::vector<Alarm> alarms;

// Blink 狀態
volatile bool stopBlink = false;
bool blinkActive = false;
unsigned long blinkStartTime = 0;
unsigned long lastBlinkTime = 0;
uint16_t blinkInterval = 500;
bool blinkState = false;
bool blinkFinalState = false;
uint16_t blinkDuration = 0;

// 函數聲明
void standby();
void turnOn();
void turnOff();
void blink(uint16_t duration, uint16_t interval, bool finalState);
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
void handleBlink();
void handleStop();

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
        <!-- Immediate Control -->
        <v-card class="mb-5">
          <v-card-title>Immediate Control</v-card-title>
          <v-card-text>
            <v-row>
              <v-col cols="6">
                <v-text-field
                  v-model="immediateBlinkDuration"
                  label="Blink Duration (ms)"
                  type="number"
                ></v-text-field>
              </v-col>
              <v-col cols="6">
                <v-select
                  v-model="immediateBlinkFinalState"
                  :items="finalStateOptions"
                  label="Final State"
                ></v-select>
              </v-col>
            </v-row>
          </v-card-text>
          <v-card-actions>
            <v-btn color="green" @click="turnOn">Turn On</v-btn>
            <v-btn color="red" @click="turnOff">Turn Off</v-btn>
            <v-btn color="orange" @click="triggerBlink">Blink</v-btn>
            <v-btn color="blue" @click="triggerStop">Stop</v-btn>
          </v-card-actions>
        </v-card>
        <!-- Add New Alarm -->
        <v-card class="mb-5">
          <v-card-title>Add New Alarm</v-card-title>
          <v-card-text>
            <v-row>
              <v-col cols="12" sm="6">
                <v-time-picker v-model="newAlarmTime" full-width></v-time-picker>
              </v-col>
              <v-col cols="12" sm="6">
                <v-select
                  v-model="newAlarm.action"
                  :items="actionOptions"
                  label="Action"
                  @change="updateBlinkFields"
                ></v-select>
                <v-select
                  v-model="newAlarm.repeat"
                  :items="repeatOptions"
                  label="Repeat"
                ></v-select>
              </v-col>
              <!-- Blink 的時長選項 -->
              <v-col cols="12" sm="6" v-if="newAlarm.isBlink">
                <v-text-field
                  v-model="newAlarm.blinkDuration"
                  label="Blink Duration (ms)"
                  type="number"
                ></v-text-field>
              </v-col>
              <!-- Blink 的終止狀態選項 -->
              <v-col cols="12" sm="6" v-if="newAlarm.isBlink">
                <v-select
                  v-model="newAlarm.finalState"
                  :items="finalStateOptions"
                  label="Final State"
                ></v-select>
              </v-col>
            </v-row>
          </v-card-text>
          <v-card-actions>
            <v-btn color="primary" @click="addAlarm">Add Alarm</v-btn>
          </v-card-actions>
        </v-card>
        <!-- Alarm List -->
        <v-card>
          <v-card-title>Alarm List</v-card-title>
          <v-card-text>
            <v-list>
              <v-list-item v-for="(alarm, index) in alarms" :key="index">
                <v-list-item-content>
                  <v-list-item-title>
                    {{ formatTime(alarm.hour, alarm.minute) }} - {{ formatAction(alarm) }}
                  </v-list-item-title>
                  <v-list-item-subtitle v-if="alarm.isBlink">
                    Blink Duration: {{ alarm.blinkDuration }} ms, Final State: {{ alarm.action ? 'On' : 'Off' }}
                  </v-list-item-subtitle>
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
        alarms: [], // 確保初始化為數組
        newAlarm: {
          repeat: 'once',
          action: 'on',
          isBlink: false,
          blinkDuration: 1000,
          finalState: true,
        },
        newAlarmTime: '12:00',
        immediateBlinkDuration: 1000,
        immediateBlinkFinalState: true,
        repeatOptions: [
          { value: 'once', text: 'Once' },
          { value: 'daily', text: 'Daily' },
          { value: 'hourly', text: 'Hourly' },
          { value: 'weekly', text: 'Weekly' },
        ],
        actionOptions: [
          { value: 'on', text: 'Turn On' },
          { value: 'off', text: 'Turn Off' },
          { value: 'blink', text: 'Blink' },
        ],
        finalStateOptions: [
          { value: true, text: 'On' },
          { value: false, text: 'Off' },
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
        formatAction(alarm) {
          return alarm.isBlink ? 'Blink' : alarm.action ? 'Turn On' : 'Turn Off';
        },
        updateBlinkFields() {
          if (this.newAlarm.action === 'blink') {
            this.newAlarm.isBlink = true;
          } else {
            this.newAlarm.isBlink = false;
            this.newAlarm.blinkDuration = null;
            this.newAlarm.finalState = null;
          }
        },
        async addAlarm() {
          const [hour, minute] = this.newAlarmTime.split(':').map(Number);
          const alarm = {
            hour,
            minute,
            repeat: this.newAlarm.repeat,
            action: this.newAlarm.action === 'blink' ? this.newAlarm.finalState : (this.newAlarm.action === 'on' ? true : this.newAlarm.action === 'off' ? false : null),
            isBlink: this.newAlarm.action === 'blink',
            blinkDuration: this.newAlarm.isBlink ? this.newAlarm.blinkDuration : 0,
          };

          // 初始化 alarms
          if (!Array.isArray(this.alarms)) {
            this.alarms = [];
          }

          this.alarms.push(alarm);

          const params = new URLSearchParams(alarm);
          const response = await fetch('/add-alarm', {
            method: 'POST',
            body: params,
          });

          if (response.ok) {
            this.loadAlarms();
          } else {
            console.error('[ERROR] Failed to add alarm:', await response.text());
          }
        },
        async loadAlarms() {
          const response = await fetch('/get-alarms');
          const alarms = await response.json();
          this.alarms = Array.isArray(alarms) ? alarms : [];
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
        async triggerBlink() {
          const params = new URLSearchParams({
            duration: this.immediateBlinkDuration,
            finalState: this.immediateBlinkFinalState,
          });
          await fetch('/blink', { method: 'POST', body: params });
        },
        async triggerStop() {
          await fetch('/stop', { method: 'POST' });
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

void startBlink(uint16_t duration, uint16_t interval, bool finalState) {
  Serial.printf("[INFO] Starting blink: duration=%dms, interval=%dms, finalState=%s\n",
                duration, interval, finalState ? "ON" : "OFF");
  stopBlink = false;
  blinkActive = true;
  blinkStartTime = millis();
  lastBlinkTime = millis();
  blinkDuration = duration;
  blinkInterval = interval;
  blinkFinalState = finalState;
  blinkState = false;
}

void updateBlink() {
  if (!blinkActive) return;

  unsigned long currentTime = millis();

  if (stopBlink || (currentTime - blinkStartTime >= blinkDuration)) {
    blinkActive = false;
    if (!stopBlink) {
      if (blinkFinalState) {
        turnOn();
      } else {
        turnOff();
      }
    }
    Serial.println("[INFO] Blink completed or interrupted");
    return;
  }

  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;
    if (blinkState) {
      turnOn();
    } else {
      turnOff();
    }
    blinkState = !blinkState;
  }
}

// ======== EEPROM 操作 ========
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
    EEPROM.write(addr++, alarm.isBlink);
    EEPROM.write(addr++, alarm.blinkDuration >> 8);
    EEPROM.write(addr++, alarm.blinkDuration & 0xFF);
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
    alarm.isBlink = EEPROM.read(addr++);
    alarm.blinkDuration = (EEPROM.read(addr++) << 8) | EEPROM.read(addr++);
    alarms.push_back(alarm);
  }
  Serial.println("[INFO] Alarms loaded successfully");
}

// ======== HTTP 請求處理 ========
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleGetAlarms() {
  StaticJsonDocument<1024> doc;
  for (const auto& alarm : alarms) {
    JsonObject obj = doc.createNestedObject();
    obj["hour"] = alarm.hour;
    obj["minute"] = alarm.minute;
    obj["repeat"] = alarm.repeat;
    obj["action"] = alarm.action;
    obj["isBlink"] = alarm.isBlink;
    obj["blinkDuration"] = alarm.blinkDuration;
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleAddAlarm() {
  if (!server.hasArg("hour") || !server.hasArg("minute") || !server.hasArg("repeat") || !server.hasArg("action")) {
    server.send(400, "text/plain", "Missing required fields");
    return;
  }

  Alarm alarm = {
    (uint8_t)server.arg("hour").toInt(),
    (uint8_t)server.arg("minute").toInt(),
  };
  strncpy(alarm.repeat, server.arg("repeat").c_str(), 10);
  alarm.action = (server.arg("action") == "true");
  alarm.isBlink = (server.hasArg("isBlink") && server.arg("isBlink") == "true");
  alarm.blinkDuration = alarm.isBlink ? server.arg("blinkDuration").toInt() : 0;

  alarms.push_back(alarm);
  saveAlarms();
  server.send(200, "text/plain", "Alarm added");
}

void handleDeleteAlarm() {
  if (!server.hasArg("index")) {
    server.send(400, "text/plain", "Missing index parameter");
    return;
  }

  uint8_t index = server.arg("index").toInt();
  if (index >= alarms.size()) {
    server.send(400, "text/plain", "Invalid index");
    return;
  }

  alarms.erase(alarms.begin() + index);
  saveAlarms();
  server.send(200, "text/plain", "Alarm deleted");
}

void handleClearAlarms() {
  alarms.clear();
  saveAlarms();
  server.send(200, "text/plain", "All alarms cleared");
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

void handleBlink() {
  if (!server.hasArg("duration") || !server.hasArg("finalState")) {
    server.send(400, "text/plain", "Missing duration or finalState");
    return;
  }

  uint16_t duration = server.arg("duration").toInt();
  bool finalState = (server.arg("finalState") == "true");

  startBlink(duration, 500, finalState);
  server.send(200, "text/plain", "Blink started");
}

void handleStop() {
  stopBlink = true;
  standby();
  server.send(200, "text/plain", "Stopped");
}

// ======== 鬧鐘觸發邏輯 ========
void checkAlarms() {
  time_t now = time(nullptr);
  struct tm* currentTime = localtime(&now);

  int currentHour = currentTime->tm_hour;
  int currentMinute = currentTime->tm_min;

  for (size_t i = 0; i < alarms.size(); i++) {
    Alarm& alarm = alarms[i];
    bool shouldTrigger = (alarm.hour == currentHour && alarm.minute == currentMinute);

    if (shouldTrigger) {
      if (alarm.isBlink) {
        startBlink(alarm.blinkDuration, 500, alarm.action);
      } else {
        if (alarm.action) {
          turnOn();
        } else {
          turnOff();
        }
      }

      if (strcmp(alarm.repeat, "once") == 0) {
        alarms.erase(alarms.begin() + i);
        saveAlarms();
        i--;
      }
    }
  }
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
  }
  Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());

  loadAlarms();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/get-alarms", HTTP_GET, handleGetAlarms);
  server.on("/add-alarm", HTTP_POST, handleAddAlarm);
  server.on("/delete-alarm", HTTP_POST, handleDeleteAlarm);
  server.on("/clear-alarms", HTTP_POST, handleClearAlarms);
  server.on("/turn-on", HTTP_POST, handleTurnOn);
  server.on("/turn-off", HTTP_POST, handleTurnOff);
  server.on("/blink", HTTP_POST, handleBlink);
  server.on("/stop", HTTP_POST, handleStop);

  server.begin();
  configTime(28800, 0, "pool.ntp.org");
}

// ======== 主程式迴圈 ========
void loop() {
  server.handleClient();
  checkAlarms();
  updateBlink();
}
