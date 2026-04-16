#include <Arduino.h>
#include <SensorPCF85063.hpp>
#include <TouchDrvFT6X36.hpp>
#include <XPowersLib.h>
#include <Arduino_GFX_Library.h>
#include <Fonts/FreeMono9pt7b.h>
#include "button.h"
#include "pin_config.h"

void calc(Arduino_CO5300 *display, TouchDrvFT6X36 *touch) {
	constexpr int kb_offset = 130;
	constexpr int gap = 10;
	constexpr int btn_width = (LCD_WIDTH - gap * 5) / 4;
	constexpr int btn_height = (LCD_HEIGHT - kb_offset - gap * 6 - 40) / 5;
	Button kb[5][4];

	const String layout[5][4] = {
		{"EX", "CA", "+-", "SR"},
		{"7", "8", "9", "/"},
		{"4", "5", "6", "*"},
		{"1", "2", "3", "-"},
		{"0", ".", "=", "+"},
	};

	display->fillScreen(RGB565_BLACK);

	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 4; col++) {
			kb[row][col].init(gap * (col + 1) + btn_width * col, kb_offset + gap * (row + 1) + btn_height * row,
								btn_width, btn_height, layout[row][col], 3, display);
			kb[row][col].draw();
		}
	}

	String result = "0";
	double rememberedNum = 0;
	String action = "";
	auto *textCanvas = new Arduino_Canvas(LCD_WIDTH, kb_offset, display);
	textCanvas->begin(GFX_SKIP_OUTPUT_BEGIN);
	textCanvas->setTextSize(4);
	textCanvas->fillScreen(RGB565_BLACK);
	textCanvas->setCursor(40, 60);
	textCanvas->print(result);
	textCanvas->flush();
	while (true) {
		TouchPoints points = touch->getTouchPoints();
		String pressedBtn = "";
		for (auto &row: kb) {
			for (auto &col: row) {
				if (col.isPressed(points)) pressedBtn = col.text;
			}
		}
		if (pressedBtn == "EX") {
			delete textCanvas;
			return;
		}
		if (pressedBtn != "") {
			if (String("0123456789").indexOf(pressedBtn) != -1) {
				if (result != "0") result += pressedBtn;
				else result = pressedBtn;
			} else if (pressedBtn == "." && result.indexOf(".") == -1) {
				result += pressedBtn;
			} else if (pressedBtn == "CA") {
				result = "0";
				action = "";
				rememberedNum = 0;
			}

			double resultDouble = result.toDouble();
			bool update = false;
			if (String("/*-+").indexOf(pressedBtn) != -1 && action == "") {
				action = pressedBtn;
				rememberedNum = resultDouble;
				result = "";
			} else if (pressedBtn == "+-") {
				resultDouble = -resultDouble;
				update = true;
			} else if (pressedBtn == "SR") {
				resultDouble = sqrt(resultDouble);
				update = true;
			} else if (pressedBtn == "=") {
				if (action == "/") resultDouble = rememberedNum / resultDouble;
				if (action == "*") resultDouble *= rememberedNum;
				if (action == "-") resultDouble = rememberedNum - resultDouble;
				if (action == "+") resultDouble += rememberedNum;
				action = "";
				rememberedNum = 0;
				update = true;
			}

			if (update) {
				char buffer[32];
				snprintf(buffer, 32, "%.10g", resultDouble);
				result = String(buffer);
			}
			textCanvas->fillScreen(RGB565_BLACK);
			textCanvas->setCursor(40, 60);
			textCanvas->print(result);
			textCanvas->flush();
			while (points.getPointCount() > 0) points = touch->getTouchPoints();
		}
	}
}
