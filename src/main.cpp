#include <Arduino.h>
#include <SensorPCF85063.hpp>
#include <TouchDrvFT6X36.hpp>
#include <XPowersLib.h>
#include <Arduino_GFX_Library.h>
#include <SD_MMC.h>
#include "button.h"
#include "pin_config.h"
#include "apps.h"
#include <AudioBoard.h>
#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioLibs/I2SCodecStream.h>
#include <AudioTools/Disk/AudioSourceSDMMC.h>

Arduino_ESP32QSPI bus(
	LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
	LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */, true);

Arduino_CO5300 display(&bus, LCD_RESET /* RST */,
						0 /* rotation */, LCD_WIDTH, LCD_HEIGHT,
						22 /* col_offset1 */,
						0 /* row_offset1 */,
						0 /* col_offset2 */,
						0 /* row_offset2 */);

XPowersAXP2101 PMU;
SensorPCF85063 rtc;
TouchDrvFT6X36 touch;
AudioDriverES8311Class driver;
DriverPins pins;
AudioBoard audioBoard(driver, pins);
AudioSourceSDMMC source("/", "mp3");
I2SCodecStream i2s(audioBoard);

MP3DecoderHelix decoder;
AudioPlayer player;

constexpr int btnWidth = 54 * 2;
constexpr int btnHeight = 60;
constexpr int totalGap = LCD_WIDTH - btnWidth * 3;
constexpr int gap = totalGap / 4;
constexpr int btnAppsHeight = 60;

Button buttonHour(gap, LCD_WIDTH / 2 - btnHeight / 2 + 28, btnWidth, btnHeight, "", 4, &display);
Button buttonMinute(gap * 2 + btnWidth, LCD_WIDTH / 2 - btnHeight / 2 + 28, btnWidth, btnHeight, "", 4, &display);
Button buttonSecond(gap * 3 + btnWidth * 2, LCD_WIDTH / 2 - btnHeight / 2 + 28, btnWidth, btnHeight, "", 4, &display);
Button buttonApps(0, LCD_HEIGHT - (btnAppsHeight), LCD_WIDTH, btnAppsHeight, "Apps", 3, &display);

int selectedOption = 0;
bool displayOn = true, redraw = true;
float lastTimeUpdate = 0;
RTC_DateTime now;

void setup() {
	pinMode(BTN_TOP, INPUT);
	pinMode(BTN_DOWN, INPUT);
	Wire.end();
	Wire.begin(IIC_SDA, IIC_SCL);

	if (!rtc.begin(Wire, IIC_SDA, IIC_SCL)) {
		while (true);
	}

	if (!PMU.begin(Wire, 0x34, IIC_SDA, IIC_SCL)) {
		while (true);
	}

	PMU.setALDO1Voltage(3300);
	PMU.enableALDO1();

	PMU.setALDO2Voltage(3400);
	PMU.enableALDO2();

	PMU.setALDO3Voltage(3300);
	PMU.enableALDO3();

	PMU.setALDO4Voltage(3300);
	PMU.enableALDO4();

	PMU.setBLDO1Voltage(3400);
	PMU.enableBLDO1();

	delay(200);

	if (!touch.begin(Wire, 0x38)) {
		while (true);
	}

	display.begin(80000000);

	display.displayOn();
	display.setBrightness(140);
	display.fillScreen(RGB565_BLACK);
	display.setCursor(40, 40);
	display.setTextSize(3);

	rtc.start();
	now = rtc.getDateTime();
	lastTimeUpdate = millis();


	SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);
	if (!SD_MMC.begin("/sdcard", true)) {
		display.println("FAILED TO INIT SDMMC");
		while (true);
	}

	pins.addI2C(PinFunction::CODEC, 14, 15);
	pins.addI2S(PinFunction::CODEC, 16, 41, 45, 40, 42);
	pins.addPin(PinFunction::PA, 46, PinLogic::Output);

	if (!audioBoard.begin()) {
		display.println("FAILED TO INIT AUDIO");
		while (true);
	}

	audioBoard.setPAPower(true);

	auto cfg = i2s.defaultConfig(TX_MODE);
	cfg.sample_rate = 22050;
	cfg.bits_per_sample = 16;
	cfg.channels = 1;

	i2s.begin(cfg);
	player.setAudioSource(source);
	player.setDecoder(decoder);
	player.setOutput(i2s);

	audioBoard.setVolume(80);
	player.setVolume(0.1);
	player.setAutoNext(false);
	player.begin(0, true);

	while (digitalRead(BTN_DOWN));
}

void addTime(const int hour, const int minute, const int second) {
	const RTC_DateTime current = rtc.getDateTime();
	rtc.setDateTime(0, 0, 0,
					(current.getHour() + hour) % 24,
					(current.getMinute() + minute) % 60,
					(current.getSecond() + second) % 60
	);
	redraw = true;
}

void loop() {
	if (millis()-lastTimeUpdate > 1000 || redraw) {
		now = rtc.getDateTime();
		lastTimeUpdate = millis();
		if (selectedOption == 1) buttonHour.color = RGB565_NAVY;
		else buttonHour.color = RGB565_DIMGRAY;
		buttonHour.draw(String(now.getHour()));

		if (selectedOption == 2) buttonMinute.color = RGB565_NAVY;
		else buttonMinute.color = RGB565_DIMGRAY;
		buttonMinute.draw(String(now.getMinute()));

		if (selectedOption == 3) buttonSecond.color = RGB565_NAVY;
		else buttonSecond.color = RGB565_DIMGRAY;
		buttonSecond.draw(String(now.getSecond()));

		buttonApps.draw();
		redraw = false;
	}

	display.setTextColor(RGB565_WHITESMOKE, RGB565_BLACK);
	display.setTextSize(2);
	display.setCursor(40, 40);
	if (PMU.isCharging()) {
		display.printf("CHARGING ");
	} else {
		display.printf("BAT:%d", PMU.getBatteryPercent());
		display.print("%  ");
	}
	display.println();

	TouchPoints points = touch.getTouchPoints();
	const int lastSelectedOpt = selectedOption;
	if (buttonHour.isPressed(points)) selectedOption = selectedOption == 1 ? 0 : 1;
	if (buttonMinute.isPressed(points)) selectedOption = selectedOption == 2 ? 0 : 2;
	if (buttonSecond.isPressed(points)) selectedOption = selectedOption == 3 ? 0 : 3;
	if (lastSelectedOpt != selectedOption) redraw = true;
	if (points.getPointCount() > 0 && selectedOption != 0) {
		const TouchPoint point = points.getPoint(0);
		int val = 0;
		if (point.y < display.height() / 2 - btnHeight) val = 1;
		else if (point.y > display.height() / 2 + btnHeight) val = -1;

		if (selectedOption == 1) addTime(val, 0, 0);
		else if (selectedOption == 2) addTime(0, val, 0);
		else if (selectedOption == 3) addTime(0, 0, val);

		while (points.getPointCount() > 0) points = touch.getTouchPoints();
	}
	if (buttonApps.isPressed(points)) {
		while (points.getPointCount() > 0) points = touch.getTouchPoints();
		Button btnCalc(20, 50, LCD_WIDTH-40, btnHeight, "Calculator", 3, &display);
		Button btnMusic(20, 50+btnHeight+10, LCD_WIDTH-40, btnHeight, "Music", 3, &display);
		Button btnExit(0, LCD_HEIGHT-btnHeight, LCD_WIDTH, btnAppsHeight, "Back", 3, &display);
		display.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT-btnHeight, RGB565_BLACK);
		btnCalc.draw();
		btnMusic.draw();
		btnExit.draw();
		bool redraw_ = false;
		while (true) {
			if (redraw_) {
				display.fillScreen(RGB565_BLACK);
				btnCalc.draw();
				btnMusic.draw();
				btnExit.draw();
				redraw_ = false;
			}
			points = touch.getTouchPoints();
			if (btnCalc.isPressed(points)) {
				calc();
				redraw_ = true;
			}
			if (btnMusic.isPressed(points)) {
				music();
				redraw_ = true;
			}
			if (btnExit.isPressed(points)) break;
			while (points.getPointCount() > 0) points = touch.getTouchPoints();
		}
		while (points.getPointCount() > 0) points = touch.getTouchPoints();
		display.fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT-btnHeight, RGB565_BLACK);
		redraw = true;
	}

	if (digitalRead(BTN_DOWN)) {
		if (selectedOption == 0) {
			display.displayOff();
			PMU.shutdown();
		}
	}
}
