#ifndef LIFESUPPORT_H
#define LIFESUPPORT_H

#include "mbed.h"
#include "N5110.h"
#include "Utils.h"  // for Position2D
#include <cstdio>

class LifeSupport {
public:
    LifeSupport();

    /// Initialize all indicators and start the timer.
    void init();

    /// Update health indicators over time.
    void update();

    /// Draw the health status panel on the LCD.
    void draw(N5110 &lcd);

    // Public indicators (for demonstration, as percentages 0-100)
    int oxygen;
    int food;
    int water;
    int health;  // overall health indicator

private:
    Timer _timer;
};

#endif
