#ifndef MENU_H
#define MENU_H

#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"

extern int selected;
extern const int NUM_OPTIONS;
extern const char* menuOptionsStr[];

void showMainMenu(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton);

#endif
