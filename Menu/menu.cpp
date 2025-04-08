#include "menu.h"

const int NUM_OPTIONS = 3;
const char* menuOptionsStr[NUM_OPTIONS] = { "Mars Explorer", "Space Invader", "   Exit   " };
int selected = 0;

void showMainMenu(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    const int menuStartRow = 1;
    selected = 0;
    bool inMenu = true;
    while (inMenu) {
        lcd.clear();
        // Print menu options
        for (int i = 0; i < NUM_OPTIONS; i++) {
            lcd.printString(menuOptionsStr[i], 0, menuStartRow + i);
        }

        // Handle joystick navigation
        Direction d = joystick.get_direction();
        if (d == N) {
            selected--;
            if (selected < 0) selected = NUM_OPTIONS - 1;
            ThisThread::sleep_for(200ms);
        } else if (d == S) {
            selected++;
            if (selected >= NUM_OPTIONS) selected = 0;
            ThisThread::sleep_for(200ms);
        }

        // Highlight the current option using a transparent rectangle
        lcd.drawRect(0, (menuStartRow + selected) * 8, 84, 8, FILL_TRANSPARENT);
        lcd.refresh();

        // Check if the select button is pressed to confirm a choice
        if (selectButton.read() == 0) {
            while (selectButton.read() == 0) { 
                ThisThread::sleep_for(50ms); 
            }
            inMenu = false;
        }
        ThisThread::sleep_for(100ms);
    }
}
