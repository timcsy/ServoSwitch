#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Servo Switch</title>
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
            <v-row justify="left" class="mb-2">
              <v-col cols="auto">
                <v-btn color="green" @click="turnOn">Turn On</v-btn>
              </v-col>
              <v-col cols="auto">
                <v-btn color="red" @click="turnOff">Turn Off</v-btn>
              </v-col>
            </v-row>
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
            <v-row justify="left" class="mb-2">
              <v-col cols="auto">
                <v-btn color="orange" @click="triggerBlink">Blink</v-btn>
              </v-col>
              <v-col cols="auto">
                <v-btn color="blue" @click="stopBlink">Stop</v-btn>
              </v-col>
            </v-row>
            <v-row justify="left">
              <v-col cols="auto">
                <v-btn color="purple" @click="correctServo">Calibrate</v-btn>
              </v-col>
            </v-row>
          </v-card-text>
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
                  <v-list-item-subtitle v-if="!alarm.isBlink">
                    Repeat: {{ formatRepeat(alarm.repeat) }}
                  </v-list-item-subtitle>
                  <v-list-item-subtitle v-if="alarm.isBlink">
                    Repeat: {{ formatRepeat(alarm.repeat) }}<br>Blink Duration: {{ alarm.blinkDuration }} ms<br>Final State: {{ alarm.action ? 'On' : 'Off' }}
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
          blinkDuration: 10000,
          finalState: true,
        },
        newAlarmTime: '12:00',
        immediateBlinkDuration: 10000,
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
        async stopBlink() {
          await fetch('/stop', { method: 'POST' });
        },
        async correctServo() {
          await fetch('/correction', { method: 'POST' });
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
        formatTime(hour, minute) {
          const period = hour >= 12 ? 'pm' : 'am';
          const formattedHour = hour % 12 || 12;
          const formattedMinute = minute.toString().padStart(2, '0');
          return `${formattedHour}:${formattedMinute} ${period}`;
        },
        formatAction(alarm) {
          return alarm.isBlink ? 'Blink' : alarm.action ? 'Turn On' : 'Turn Off';
        },
        formatRepeat(repeat) {
          return repeat.charAt(0).toUpperCase() + repeat.slice(1);
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
      },
    });
  </script>
</body>
</html>
)rawliteral";