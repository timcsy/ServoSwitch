#ifndef SCRATCH_H
#define SCRATCH_H

#include "Network.h"
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

class Scratch {
	public:
		Scratch() {
      network = new Network("ESP8266 WiFi Configuration");
      webSocket.begin();
      webSocket.onEvent(webSocketEvent);
    }
    
		void setup() {
      network->setup();
      while (WiFi.status() != WL_CONNECTED) {
				serialLoop();
        network->loop();
			}
		}

		void loop() { // This should be put in the beginning of the loop()
      serialLoop();
      network->loop();
      webSocket.loop();
		}

    String serialReadline() {
      String msg = "";
      while (true) {
        if (Serial.available() > 0) {
          char c = Serial.read();
          if (c == '\n') return msg;
          msg += c;
        }
      }
      return msg;
    }

    void serialLoop() {
      if (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n') {
          Serial.println("\n\nEnter the WiFi AP SSID:");
          String ssid = serialReadline();
          if (ssid == "") ssid = "ESP8266 WiFi Configuration";

          Serial.println("Enter the password:");
          String password = serialReadline();

          Serial.println("Enter y if you want to reset the stored WiFi configuration:");
          bool reset;
          String resetStr = serialReadline();
          if (resetStr == "y") reset = true;
          else reset = false;

          network->setAP(ssid, password);
          network->setup(reset);
        }
      }
    }

    static void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
      switch(type) {
        case WStype_CONNECTED:
          {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            // send message to client
            webSocket.sendTXT(num, "0");
          }
          break;
        case WStype_TEXT:
          // Serial.printf("[%u] get Text: %s\n", num, payload);

          onMessage(num, payload);

          // send message to client
          // webSocket.sendTXT(num, payload);

          // send data to all connected clients
          // webSocket->broadcastTXT("message here");
          break;
        case WStype_BIN:
          Serial.printf("[%u] get binary length: %u\n", num, length);
          hexdump(payload, length);

          // send message to client
          // webSocket->sendBIN(num, payload, length);
          break;
        case WStype_DISCONNECTED:
          Serial.printf("[%u] Disconnected!\n", num);
          break;
      }
    }

    static void onMessage(uint8_t id, uint8_t * payload) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
      }

      String command = doc["cmd"];
      if (command == "echo") {
        webSocket.sendTXT(id, payload);
      } else if (command == "wd") { // write digital
        int pin = doc["pin"];
        pinMode(pin, OUTPUT);
        digitalWrite(pin, (int)doc["val"]);
      } else if (command == "wa") { // write analog
        int pin = doc["pin"];
        pinMode(pin, OUTPUT);
        analogWrite(pin, (int)doc["val"]);
      } else if (command == "rd") { // read digital
        int pin = doc["pin"];
        int value = digitalRead(pin);
        String msg = String("{\"cmd\":\"rd\",\"pin\":") + pin + ",\"val\":" + value + "}";
        webSocket.sendTXT(id, msg.c_str());
      } else if (command == "ra") { // read analog
        int value = analogRead(A0);
        String msg = String("{\"cmd\":\"ra\",\"val\":") + value + "}";
        webSocket.sendTXT(id, msg.c_str());
      } else if (command == "pm") { // pin mode
        pinMode((int)doc["pin"], (int)doc["val"]);
      }
    }

    static WebSocketsServer webSocket;

	private:
    Network * network;
};

WebSocketsServer Scratch::webSocket = WebSocketsServer(8000);

#endif
