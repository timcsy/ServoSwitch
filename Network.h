#ifndef NETWORK_H
#define NETWORK_H

#include <ESP8266WiFi.h>          // ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     // Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "QRCodeLabel.h"

class Network {
	public:
		Network(String ssid = "ESP8266 WiFi Configuration", String password = ""):
			ssid(ssid), password(password), wifiManager(NULL), isConnecting(false) {}
		
		void setup(bool reset = false) { // This should be put in the beginning of the setup()
			Serial.println("[INFO] Initializing WiFiManager...");

      if (wifiManager != NULL) delete wifiManager;
      wifiManager = new WiFiManager();
      wifiManager->setConfigPortalBlocking(false);
      
			connect(ssid, password, reset);
		}

		void loop() { // This should be put in the beginning of the loop()
      wifiManager->process();
      // To check if the connection is still valid
			while (WiFi.status() != WL_CONNECTED && !isConnecting) {
				connect(ssid, password);
			}
      if (WiFi.status() == WL_CONNECTED) isConnecting = false;
		}

		void connect(String ssid = "ESP8266 WiFi Configuration", String password = "", bool reset = false) {
      isConnecting = true;

			QRCodeLabel::show("http://192.168.4.1", "http://192.168.4.1");

      if (reset) wifiManager->resetSettings();
      if (password == "") wifiManager->autoConnect(ssid.c_str());
			else wifiManager->autoConnect(ssid.c_str(), password.c_str());

			Serial.print("[INFO] Connected to WiFi! IP Address: ");
			Serial.println(WiFi.localIP());
      if (WiFi.status() == WL_CONNECTED) {
          String url = String("http://") + WiFi.localIP().toString();
			    QRCodeLabel::show(url, url);
      }
		}

    void setAP(String ssid = "ESP8266 WiFi Configuration", String password = "") {
      this->ssid = ssid;
      this->password = password;
    }
	
	private:
		String ssid;
		String password;
    WiFiManager * wifiManager;
    bool isConnecting;
};

#endif
