#ifndef WATCHS3_FIRMWARE_APPS_H
#define WATCHS3_FIRMWARE_APPS_H
#include <TouchDrvFT6X36.hpp>
#include <Arduino_GFX_Library.h>
#include <AudioTools.h>

void calc(Arduino_CO5300 *display, TouchDrvFT6X36 *touch);

void music(Arduino_CO5300 *display, TouchDrvFT6X36 *touch, AudioPlayer *player);

#endif //WATCHS3_FIRMWARE_APPS_H