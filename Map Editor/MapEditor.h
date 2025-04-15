#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"

#define MAP_WIDTH 60
#define MAP_HEIGHT 10
#define TILE_SIZE 8
#define TILE_TYPE_COUNT 6  // Update if you add more tiles

class MapEditor {
public:
    MapEditor(N5110 &lcd, Joystick &joystick, DigitalIn &select);
    void run();

private:
    void drawMap();
    void drawCursor();
    void drawTileSelector();
    void update();
    void exportMap();

    N5110 &lcd;
    Joystick &joystick;
    DigitalIn &selectButton;

    int map[MAP_HEIGHT][MAP_WIDTH];
    int cursorX, cursorY;
    int selectedTile;
    int viewportX, viewportY;
};

#endif