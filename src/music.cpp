#include <Arduino.h>
#include <Wire.h>
#include <SD_MMC.h>
#include <Audio.h>
#include <TouchDrvFT6X36.hpp>
#include <Arduino_GFX_Library.h>
#include <driver/i2s.h>
#include "button.h"
#include "pin_config.h"
#include <esp_log.h>
#include <AudioBoard.h>

extern SPIClass sdSPI;

void updateFiles(std::vector<String> &files) {
	files.clear();
	File root = SD_MMC.open("/music");
	if (!root || !root.isDirectory()) return;

	File file = root.openNextFile();
	while (file) {
		const String name = String(file.name());
		files.push_back(name);
		file.close();
		file = root.openNextFile();
	}
	root.close();
}

void updateButtons(Button (&Buttons)[5], const std::vector<String> &files, int cursor) {
	for (int i = 0; i < min(static_cast<int>(files.size()), 5); i++) {
		Buttons[i].text = files[i];
	}
}

void playMusic(Arduino_CO5300 *display, TouchDrvFT6X36 *touch, Audio *audio, String text) {
	Button btnExit = Button(0, display->height()-50, display->width(), 50, "exit", 3, display);
	audio->connecttoFS(SD_MMC, String("/sdcard/music/"+text).c_str(), 0);
	display->fillScreen(RGB565_BLACK);
	btnExit.draw();
	uint32_t lastTouch = millis();
	while (true) {
		audio->loop();
		if (millis()-lastTouch > 50) {
			TouchPoints points = touch->getTouchPoints();
			if (btnExit.isPressed(points)) {
				audio->stopSong();
				return;
			}
			lastTouch = millis();
		}
	}
}

void music(Arduino_CO5300 *display, TouchDrvFT6X36 *touch, Audio *audio) {
	display->fillScreen(RGB565_BLACK);
	display->setCursor(50,50);
	display->setTextColor(RGB565_WHITE);
	display->setTextSize(2);

	std::vector<String> files;
	updateFiles(files);

	int cursor = 0;
	TouchPoints points = touch->getTouchPoints();
	while (points.getPointCount() > 0) points = touch->getTouchPoints();

	constexpr int startY = 80; constexpr int btnHeight = 40; const int btnWidth = display->width() - 20;

	files.reserve(20);
	Button buttons[5];
	Button btnExit = Button(0, 0, display->width(), btnHeight, "exit", 3, display);

	for (int i = 0; i < 5; i++) {
		buttons[i].init(10, startY + i * (btnHeight + 10), btnWidth, btnHeight, "", 2, display);
	}

	updateButtons(buttons, files, cursor);

	display->fillScreen(RGB565_BLACK);
	btnExit.draw();
	for (int i = 0; i < 5; i++) {
		buttons[i].draw();
	}

	while (true) {
		TouchPoints points = touch->getTouchPoints();
		if (points.getPointCount() > 0) {
			if (btnExit.isPressed(points)) {
				return;
			}

			for (int i = 0; i < 5; i++) {
				if (!buttons[i].isPressed(points) ||  buttons[i].text == "") continue;
				playMusic(display, touch, audio, buttons[i].text);
				display->fillScreen(RGB565_BLACK);
				btnExit.draw();
				for (int i = 0; i < 5; i++) {
					buttons[i].draw();
				}
				break;
			}
		}
		delay(20);
	}
}
