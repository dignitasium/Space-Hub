#ifndef MAP_H
#define MAP_H

#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"

// World size definitions
#define MAP_WIDTH 60
#define MAP_HEIGHT 8

// Tile size and viewport
#define TILE_SIZE 8
#define VIEWPORT_WIDTH 10
#define VIEWPORT_HEIGHT 4

// Tile types
#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_HAB 2
#define TILE_ROVER 3
#define TILE_CRATER 4
#define TILE_TERMINAL 5

// Function declarations for the two game modes:
void exploreMap(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton);
void spaceInvadeGame(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton);

#endif
