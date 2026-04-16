#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <Audio.h>
#include <TouchDrvFT6X36.hpp>
#include <Arduino_GFX_Library.h>
#include <driver/i2s.h>
#include "button.h"
#include "pin_config.h"
#include <esp_log.h>
#include <AudioBoard.h>

// Audio audio;
// AudioDriverES8311Class driver;

void updateFiles(std::vector<String> &files) {
	files.clear();
	File root = SD.open("/music");
	if (!root || !root.isDirectory()) return;

	root.rewindDirectory();

	File file = root.openNextFile();
	while (file) {
		const String name = String(file.name());
		if (!file.isDirectory())
			files.push_back(name);
		file = root.openNextFile();
	}
}

void updateButtons(Button (&Buttons)[5], const std::vector<String> &files, int cursor) {
	for (int i = 0; i < min(static_cast<int>(files.size()), 5); i++) {
		Buttons[i].text = files[i];
	}
}

void playMusic(Arduino_CO5300 *display, TouchDrvFT6X36 *touch, String text) {
}

void music(Arduino_CO5300 *display, TouchDrvFT6X36 *touch) {
	SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
	if (!SD.begin(SD_CS, SPI)) {
		return;
	}
	int cursor = 0;
	TouchPoints points = touch->getTouchPoints();
	while (points.getPointCount() > 0) points = touch->getTouchPoints();

	// audio.setPinout(I2S_CLK, I2S_CMD, I2S_DATA);
	// audio.forceMono(true);
	// audio.setVolume(50);

	constexpr int startY = 80; constexpr int btnHeight = 40; const int btnWidth = display->width() - 20;

	std::vector<String> files;
	files.reserve(20);
	static Button buttons[5];
	static Button btnExit = Button(0, 0, display->width(), btnHeight, "exit", 3, display);

	for (int i = 0; i < 5; i++) {
		buttons[i].init(10, startY + i * (btnHeight + 10), btnWidth, btnHeight, "", 2, display);
	}
	// updateFiles(files);

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
				playMusic(display, touch, buttons[i].text);
				display->fillScreen(RGB565_BLACK);
				btnExit.draw();
				for (auto b: buttons) {
					b.draw();
				}
				break;
			}
		}
		delay(20);
	}
}
