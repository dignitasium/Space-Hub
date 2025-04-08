#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "menu.h"
#include "games.h"

N5110 lcd(PC_7, PA_9, PB_10, PB_5, PB_3, PA_10);
Joystick joystick(PC_1, PC_0, PB_4);
DigitalIn selectButton(BUTTON1);

int main() {
    lcd.init(LPH7366_1);
    lcd.setContrast(0.5);
    joystick.init();

    while (true) {
        // Show main menu to choose game mode or exit
        showMainMenu(lcd, joystick, selectButton);
        if (selected == 0) {
            // Mars Exploration game
            exploreMap(lcd, joystick, selectButton);
        } else if (selected == 1) {
            // Space Invade game
            spaceInvadeGame(lcd, joystick, selectButton);
        } else {
            // Exit option
            lcd.clear();
            lcd.printString("Exiting...", 0, 3);
            lcd.refresh();
            ThisThread::sleep_for(2000ms);
            while (true) { 
                ThisThread::sleep_for(1000ms); 
            }
        }
    }
}