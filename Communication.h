#ifndef COMMUNICATION_H
#define COMMUNICATION_H

namespace Communication {
    void initializeWiFi();          // 初始化 Wi-Fi（使用 WiFiManager）
		void checkWiFi();               // 檢查 WiFi 連線
    void initializeHTTP();          // 初始化 HTTP 伺服器
    void initializeBLE();           // 初始化 BLE（僅 ESP32 支持）
    void handleHTTPClient();        // 處理 HTTP 客戶端請求
}

#endif
