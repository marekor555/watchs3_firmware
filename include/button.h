#ifndef WATCHS3_FIRMWARE_BUTTON_H
#define WATCHS3_FIRMWARE_BUTTON_H
#include <Arduino_GFX.h>

class Button {
public:
	int x, y, width, height, color = RGB565_GRAY;
	String text;
	Arduino_Canvas *btnCanvas;

	Button(int x_ = 0, int y_ = 0, int width_ = 0, int height_ = 0, String text_ = "", int textSize_ = 1, Arduino_CO5300 *display = nullptr) {
		init(x_, y_, width_, height_, text_, textSize_, display);
	}

	void init(int x_ = 0, int y_ = 0, int width_ = 0, int height_ = 0, String text_ = "", int textSize_ = 1, Arduino_CO5300 *display = nullptr) {
		x = x_, y = y_, width = width_, height=height_, text=text_;

		if (display != nullptr && width_ > 0 && height_ > 0) {
			btnCanvas = new Arduino_Canvas(width_, height_, display, x_, y_);
			btnCanvas->begin(GFX_SKIP_OUTPUT_BEGIN);
			btnCanvas->setTextSize(textSize_);
		}
	}

	void obliterate() const {
		delete btnCanvas;
	}

	bool isPressed(TouchPoints &points) const {
		for (int i = 0; i < points.getPointCount(); i++) {
			TouchPoint point = points.getPoint(i);
			if ((point.x > x && point.x < x + width) && (point.y > y && point.y < y + height)) {
				return true;
			}
		}
		return false;
	}

	void draw(const String &text_ = "") {
		String drawString = text;
		if (!text_.isEmpty()) drawString = text_;
		btnCanvas->fillScreen(RGB565_BLACK);
		btnCanvas->fillRoundRect(0, 0, width, height, min(width, height) / 2, color);
		int16_t x1, y1;
		uint16_t w, h;
		btnCanvas->getTextBounds(drawString.c_str(), 0, 0, &x1, &y1, &w, &h);

		const short drawX = (width / 2) - (w / 2) - x1;
		const short drawY = (height / 2) - (h / 2) - y1;

		btnCanvas->setCursor(drawX, drawY);
		btnCanvas->print(drawString);
		btnCanvas->flush();
	}
};


#endif //WATCHS3_FIRMWARE_BUTTON_H