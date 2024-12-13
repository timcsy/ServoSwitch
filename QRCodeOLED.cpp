#ifdef ESP8266

#include "QRCodeLabel.h"
#include <qrcodeoled.h>
#include <SSD1306.h>

// 靜態變數保存 OLED 狀態
SSD1306* display;
QRcodeOled* qrcode;
String currentQRText = "";
String currentLabelText = "";

// 初始化 OLED 顯示器
void QRCodeLabel::initialize(uint8_t i2c_address, uint8_t sda_pin, uint8_t scl_pin) {
		display = new SSD1306(i2c_address, sda_pin, scl_pin);  // 設定 I2C 位址和引腳
    qrcode = new QRcodeOled(display);

		// 初始化顯示器
		display->init();
		display->flipScreenVertically();
		display->setFont(ArialMT_Plain_10);

		// 清除顯示屏
		display->clear();
		display->display();
}

// 顯示 QR Code 和底部文本（僅在內容變化時刷新）
void QRCodeLabel::show(const String& qrText, const String& labelText) {
		if (qrText == currentQRText && labelText == currentLabelText) {
				return; // 如果內容無變化則不執行
		}

		// 更新狀態
		currentQRText = qrText;
		currentLabelText = labelText;

		// 清除顯示屏
		display->clear();

		// 生成並顯示 QR Code
		qrcode->init(128, 50);
		qrcode->create(qrText);

		// 繪製底部文本區域背景
		display->setColor(WHITE);
		display->fillRect(0, 50, 128, 14); // 根據需要調整文本區域高度

		// 在文本區域上顯示文字
		display->setColor(BLACK);
		display->setTextAlignment(TEXT_ALIGN_CENTER);
		display->drawString(64, 50, labelText);

		// 更新顯示
		display->display();
}

#endif
