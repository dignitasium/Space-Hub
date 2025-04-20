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

#define TILE_EMPTY    0  // No tile
#define TILE_WALL     1  // Solid line at bottom
#define TILE_HAB      2  // Small black rectangle (Habitat module)
#define TILE_ROVER    3  // Filled square (Rover)
#define TILE_CRATER   4  // Line in the middle (Crater)
#define TILE_TERMINAL 5  // Two pixels (Terminal)


// Function declarations for the two game modes:
void exploreMap(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton);
void spaceInvadeGame(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton);

#endif
