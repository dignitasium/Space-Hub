#include "LifeSupport.h"

LifeSupport::LifeSupport() : oxygen(100), food(100), water(100), health(100) { }

void LifeSupport::init() {
    oxygen = 100;
    food = 100;
    water = 100;
    health = 100;
    _timer.start();
}

void LifeSupport::update() {
    // Every 1000 ms (1 second) reduce indicators to simulate consumption/deterioration.
    if (_timer.read_ms() >= 1000) {
        oxygen -= 1;
        food   -= 1;
        water  -= 1;

        if (oxygen < 50 || food < 50 || water < 50) {
            health -= 2;
        } else {
            health -= 1;
        }
        if (oxygen < 0) oxygen = 0;
        if (food < 0) food = 0;
        if (water < 0) water = 0;
        if (health < 0) health = 0;

        _timer.reset();
    }
}

// Updated draw() so that the indicators are drawn in the bottom-right corner.
void LifeSupport::draw(N5110 &lcd) {
    char buf[20];
    // Print oxygen on bank row 4, starting at pixel column 50.
    sprintf(buf, "O2:%d%%", oxygen);
    lcd.printString(buf, 50, 4);
    // Print overall health on bank row 5, starting at pixel column 50.
    sprintf(buf, "H:%d%%", health);
    lcd.printString(buf, 50, 5);
}
