#include <Arduino.h>
#include <Wire.h>
#include <SD_MMC.h>
#include <TouchDrvFT6X36.hpp>
#include <Arduino_GFX_Library.h>
#include "button.h"
#include <AudioBoard.h>
#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioLibs/I2SCodecStream.h>
#include <AudioTools/Disk/AudioSourceSDMMC.h>

extern SPIClass sdSPI;

extern Arduino_CO5300 display;
extern TouchDrvFT6X36 touch;

extern AudioDriverES8311Class driver;
extern AudioBoard audioBoard;
extern AudioSourceSDMMC source;
extern I2SCodecStream i2s;
extern MP3DecoderHelix decoder;
extern AudioPlayer player;


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
		Buttons[i].text = files[i+cursor];
	}
}

void playMusic(const String& text) {
	display.fillScreen(RGB565_BLACK);

	auto btnExit = Button(0, display.height()-50, display.width(), 50, "exit", 3, &display);
	auto btnVolUp = Button(20, display.height()/3*2, 100, 100, "+", 5, &display);
	auto btnVolDown = Button(display.width()-100-20, display.height()/3*2, 100, 100, "-", 5, &display);
	auto btnSP = Button(display.width()/2-50, display.height()/3*2, 100, 100, "SP", 5, &display);

	display.setCursor(50, display.height()/3);
	display.setTextSize(2);
	display.println(text);
	btnExit.draw();
	btnVolDown.draw();
	btnVolUp.draw();
	btnSP.draw();

	player.stop();
	player.setVolume(0.005);
	audioBoard.setVolume(0);
	float volume = 0.005;
	player.setPath(String("/music/"+text).c_str());
	audioBoard.setVolume(80);
	player.play();
	bool playing = true;
	uint32_t lastTouch = millis();
	while (true) {
		if (playing && player.isActive()) player.copy();
		if (millis()-lastTouch > 200) {
			TouchPoints points = touch.getTouchPoints();
			if (btnExit.isPressed(points)) {
				player.stop();
				return;
			}
			if (btnVolUp.isPressed(points) && volume <=1) {
				volume += 0.005;
				player.setVolume(volume);
			}
			if (btnVolDown.isPressed(points) && volume > 0) {
				volume -= 0.005;
				player.setVolume(volume);
			}
			if (btnSP.isPressed(points)) {
				player.stop();
				playing = !playing;
				while (touch.getTouchPoints().getPointCount() > 0);
				if (playing) player.play();
				else player.stop();
			}
			lastTouch = millis();
		}
		if (!player.isActive() && playing) {
			player.stop();
			return;
		}
		yield();
	}
}

void music() {
	display.fillScreen(RGB565_BLACK);
	display.setCursor(50,50);
	display.setTextColor(RGB565_WHITE);
	display.setTextSize(2);

	std::vector<String> files;
	updateFiles(files);

	int cursor = 0;
	TouchPoints points = touch.getTouchPoints();
	while (points.getPointCount() > 0) points = touch.getTouchPoints();

	constexpr int btnHeight = 40; const int btnWidth = display.width() - 20;

	files.reserve(20);
	Button buttons[5];
	Button btnExit = Button(0, 0, display.width(), btnHeight, "exit", 3, &display);
	Button btnUp = Button(20, display.height()/3*2, 100, 100, "+", 5, &display);
	Button btnDown = Button(display.width()-100-20, display.height()/3*2, 100, 100, "-", 5, &display);

	for (int i = 0; i < 5; i++) {
		constexpr int startY = 80;
		buttons[i].init(10, startY + i * (btnHeight + 10), btnWidth, btnHeight, "", 2, &display);
	}

	updateButtons(buttons, files, cursor);

	display.fillScreen(RGB565_BLACK);
	btnExit.draw();
	for (int i = 0; i < 5; i++) {
		buttons[i].draw();
	}
	btnUp.draw();
	btnDown.draw();
	while (true) {
		points = touch.getTouchPoints();
		if (points.getPointCount() > 0) {
			if (btnExit.isPressed(points)) {
				return;
			}
			if (btnUp.isPressed(points) && cursor > 0) {
				cursor--;
				updateButtons(buttons, files, cursor);
			}
			if (btnDown.isPressed(points) && cursor + 5 < files.size()) {
				cursor++;
				updateButtons(buttons, files, cursor);
			}

			for (int i = 0; i < 5; i++) {
				if (!buttons[i].isPressed(points) ||  buttons[i].text == "") continue;
				playMusic(buttons[i].text);
				break;
			}
			display.fillScreen(RGB565_BLACK);
			btnExit.draw();
			btnUp.draw();
			btnDown.draw();
			for (int i = 0; i < 5; i++) {
				buttons[i].draw();
			}
			while (points.getPointCount() != 0) points = touch.getTouchPoints();
		}
		delay(20);
	}
}
