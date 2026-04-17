#ifndef WATCHS3_FIRMWARE_PIN_CONFIG_H
#define WATCHS3_FIRMWARE_PIN_CONFIG_H
#pragma once

#define XPOWERS_CHIP_AXP2101

#define LCD_SDIO0 4
#define LCD_SDIO1 5
#define LCD_SDIO2 6
#define LCD_SDIO3 7
#define LCD_SCLK 11
#define LCD_CS 12
#define LCD_RESET 8
#define LCD_WIDTH 410
#define LCD_HEIGHT 502

// TOUCH
#define IIC_SDA 15
#define IIC_SCL 14
#define TP_INT 38
#define TP_RESET 9

// SD
#define SD_MOSI 1
#define SD_SCK 2
#define SD_MISO 3
#define SD_CS 17

constexpr int SDMMC_CLK = 2;
constexpr int SDMMC_CMD = 1;
constexpr int SDMMC_DATA = 3;
constexpr int SDMMC_CS = 17;
// ES8311
#define I2S_MCLK 16
#define I2S_SCLK 41


#define BTN_TOP 0
#define BTN_DOWN 10

#endif //WATCHS3_FIRMWARE_PIN_CONFIG_H