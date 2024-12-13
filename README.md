# Servo Switch 無線懶人開關

## 特色
- 操作彈性客製化：可通過寫程式（附預設專案程式碼）、API 遙控
- 支援 WiFi（ESP8266 系列開發版）或藍芽（ESP32 系列開發版）
- 具備可儲存的定時、閃爍功能
- 非侵入式安裝：不需要拆除原本開關，不留殘膠，黏著穩固
- 材料取得方便，都是一些基本元件，成本不高
- 可用 OLED 螢幕顯示設定網頁的 QRCode，方便操作

## 設置

### 硬體

材料：
- ESP8266 或 ESP32 系列開發版 x 1
  - [ESP8266串口wifi模塊 NodeMcu Lua WIFI V3 物聯網 開發 CH340](https://www.ruten.com.tw/item/show?21647522108581)
  - [ESP32 開發板](https://www.ruten.com.tw/item/show?22133499835548)
- SG90 伺服馬達 180 度 x 2
  - [SG90 伺服馬達 舵機 9克伺服機 180度](https://www.ruten.com.tw/item/show?21513817238652)
- 3M 無痕迷你膠條 x 2 小片
  - [3M™ 無痕™ 迷你膠條 18964, 文具通路獨家包](https://www.3m.com.tw/3M/zh_TW/p/d/v000064184/)
- 杜邦線（公母 x 6，母母 x 4 如果選用 OLED）
  - [杜邦線 10公分 公對母 10條1單位 彩色排線](https://www.ruten.com.tw/item/show?21535514541815)
  - [杜邦線 10公分 母對母 40條1單位 彩色排線](https://www.ruten.com.tw/item/show?21535514562032)
- 電源（充電頭或是行動電源）、Micro USB 電源線
- OLED SSD1306 x 1 （可選）
  - [0.96寸 白色 I2C IIC通信 12864 OLED顯示屏模塊 提供例程](https://www.ruten.com.tw/item/show?21715781100384)

接線（如果選用 [ESP8266 系列開發版](https://honeststore.com.tw/esp8266-pin-out/)）：
- OLED 螢幕
  - GND 接 GND
  - VCC 接 D5
  - SDA 接 D2
  - SCL 接 D1
- 上方伺服馬達
  - 棕線接 GND
  - 紅線接 3V3
  - 橘線接 D4
- 下方伺服馬達
  - 棕線接 GND
  - 紅線接 3V3
  - 橘線接 D8

接線（如果選用 [ESP32 系列開發版](https://zerotech.club/esp32-gpio/)）：
- OLED 螢幕
  - GND 接 GND
  - VCC 接 GPIO19
  - SDA 接 21
  - SCL 接 22
- 上方伺服馬達
  - 棕線接 GND
  - 紅線接 5V
  - 橘線接 GPIO12
- 下方伺服馬達
  - 棕線接 GND
  - 紅線接 3V3
  - 橘線接 GPIO23

請把伺服馬達附的單邊舵角安裝上去，並且按以下的說明校正之後用螺絲固定。

### 軟體

請先下載 Arduino IDE

如果是 ESP8266 系列開發版，請參考 [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino) 來安裝設定

如果是 ESP32 系列開發版，請參考 [Arduino core for the ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP32-C6 and ESP32-H2](https://github.com/espressif/arduino-esp32?tab=readme-ov-file) 來安裝設定

去 Arduino IDE 的程式庫管理員安裝以下套件：
- [ArduinoJson](https://arduinojson.org/)
- [WiFiManager](https://github.com/tzapu/WiFiManager)（如果選用 ESP8266 系列開發版）
- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo)（如果選用 ESP32 系列開發版）
- [QRcodeOled](https://github.com/yoprogramo/QRcodeOled)（如果選用 OLED）

可以依照需求修改程式，最後選擇你的開發板與連接埠，將 `ServoSwitch.ino` 燒錄至開發版。


## 使用說明

### ESP8266 系列開發版

由序列埠或是 OLED 來取得板子的 IP 位址，並且去 http://你的IP位址 進行 UI 操作。

### ESP32 系列開發版

在瀏覽器（iOS 或是 iPadOS 用戶請使用 [Bluefy 瀏覽器](https://apps.apple.com/tw/app/bluefy-web-ble-browser/id1492822055)，因為蘋果的一般瀏覽器無支援 [Web Bluetooth API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API)）開啟 https://timcsy.github.io/ServoSwitch/，並且連接到藍芽裝置 `Servo Switch`。

連接成功後即可進行 UI 操作。

### UI 操作

有提供即時操作的功能（懶人開關），開啟、關閉以及閃爍（閃爍的部分可以指定時長與終止狀態，3 秒會切換一次狀態）。

`CALIBRATE` 是校正的功能，在一開始安裝伺服馬達的時候可以使用，會在一段時間內讓馬達從 0 度轉向 180 度，會後會停在 90 度，請讓單邊舵角的方向跟馬達的機身平行，並且指向前面。之後就可以鎖上螺絲，兩顆馬達都校正之後可以試試 `TURN ON` 或是 `TURN OFF`。

也有提供鬧鐘設定功能，可以設定狀態、時間以及重複的頻率，也可以瀏覽刪除鬧鐘。