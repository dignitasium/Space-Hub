#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "games.h"

// Global variables for map exploration.
// This creates a 2D array to represent the game map.
int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

// Global player state (position in the map).
int playerX = 2;
int playerY = 6;

// Global viewport state (defines which part of the map is visible on the LCD).
int viewportX = 0;
int viewportY = 0;

// Helper function to draw a single pixel on the LCD.
// It uses the drawRect function to effectively draw a 1x1 rectangle (a pixel).
static void drawPixel(N5110 &lcd, int x, int y, FillType color) {
    lcd.drawRect(x, y, 1, 1, color);
}

// Custom drawBitmap function that uses the helper drawPixel function.
// It draws a monochrome bitmap by iterating through each bit of the bitmap array.
void drawBitmap(N5110 &lcd, int x, int y, const unsigned char *bitmap, int width, int height) {
    // Calculate the number of bytes per row in the bitmap data.
    int bytesPerRow = (width + 7) / 8;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Calculate the index into the bitmap array.
            int byteIndex = row * bytesPerRow + (col / 8);
            // Determine the specific bit in the byte we want to check.
            int bitIndex = 7 - (col % 8);
            // If the bit is set, draw a pixel at the corresponding location.
            if (bitmap[byteIndex] & (1 << bitIndex)) {
                drawPixel(lcd, x + col, y + row, FILL_BLACK);
            }
        }
    }
}

// Updates the viewport's coordinates to center around the player's position.
// It also ensures the viewport doesn't extend outside the boundaries of the map.
void updateViewport() {
    viewportX = playerX - VIEWPORT_WIDTH / 2;
    viewportY = playerY - VIEWPORT_HEIGHT / 2;
    // Clamp the viewport to the left and top edges.
    if (viewportX < 0) viewportX = 0;
    if (viewportY < 0) viewportY = 0;
    // Clamp the viewport to the right and bottom edges.
    if (viewportX > MAP_WIDTH - VIEWPORT_WIDTH) viewportX = MAP_WIDTH - VIEWPORT_WIDTH;
    if (viewportY > MAP_HEIGHT - VIEWPORT_HEIGHT) viewportY = MAP_HEIGHT - VIEWPORT_HEIGHT;
}

// Main function that implements the map exploration game mode.
void exploreMap(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    // Draw the map's borders by setting the outer edges to walls.
    for (int x = 0; x < MAP_WIDTH; x++) {
        map[0][x] = TILE_WALL;
        map[MAP_HEIGHT - 1][x] = TILE_WALL;
    }
    for (int y = 0; y < MAP_HEIGHT; y++) {
        map[y][0] = TILE_WALL;
        map[y][MAP_WIDTH - 1] = TILE_WALL;
    }
    
    // Place horizontal features on the map (at row 6) for different objects.
    // HAB (Habitat area) is drawn from x = 5 to 9.
    for (int x = 5; x < 10; x++) map[6][x] = TILE_HAB;
    // Rover area is drawn from x = 15 to 19.
    for (int x = 15; x < 20; x++) map[6][x] = TILE_ROVER;
    // Crater area from x = 25 to 29.
    for (int x = 25; x < 30; x++) map[6][x] = TILE_CRATER;
    // Terminal area from x = 35 to 37.
    for (int x = 35; x < 38; x++) map[6][x] = TILE_TERMINAL;

    // Display introductory instructions on the LCD.
    lcd.clear();
    lcd.printString("Explore Mars", 0, 1);
    lcd.printString("Use joystick", 0, 2);
    lcd.printString("to move", 0, 3);
    lcd.printString("Press select", 0, 4);
    // Clear old crater
    for (int y = 5; y <= 7; y++) {
        for (int x = 25; x <= 29; x++) {
            map[y][x] = TILE_EMPTY;
        }
    }

    // Shape the crater edges and floor
    map[5][26] = TILE_CRATER;
    map[5][27] = TILE_CRATER;
    map[5][28] = TILE_CRATER;

    map[6][25] = TILE_CRATER;
    map[6][29] = TILE_CRATER;

    map[7][25] = TILE_CRATER;
    map[7][26] = TILE_CRATER;
    map[7][27] = TILE_CRATER;
    map[7][28] = TILE_CRATER;
    map[7][29] = TILE_CRATER;
    // center is left as TILE_EMPTY to appear hollow    // Wait until the select button is pressed to start the game.
    // First wait until the button is released (if previously pressed),
    // then wait for a new press.
    while (selectButton.read() == 1) ThisThread::sleep_for(100ms);
    while (selectButton.read() == 0) ThisThread::sleep_for(50ms);

    // Main exploration loop of the game.
    while (true) {
        lcd.clear(); // Clear the LCD for drawing the new frame.

        // Read joystick input and determine the new intended position.
        Direction d = joystick.get_direction();
        int newX = playerX;
        int newY = playerY;
        if (d == N)      newY--; // Move up.
        else if (d == S) newY++; // Move down.
        else if (d == E) newX++; // Move right.
        else if (d == W) newX--; // Move left.

        // Check if movement is valid:
        // 1. The player moved.
        // 2. The new position is within the map boundaries.
        // 3. The target tile is not a wall.
        if ((newX != playerX || newY != playerY) &&
            newX >= 0 && newX < MAP_WIDTH &&
            newY >= 0 && newY < MAP_HEIGHT &&
            map[newY][newX] != TILE_WALL) {
            playerX = newX;
            playerY = newY;
1            ThisThread::sleep_for(150ms); // Debounce delay after movement.
        }

        // Update the viewport to center around the updated player position.
        updateViewport();

        // Detailed bitmap for HAB (Habitat) feature.
        // This bitmap is drawn in more detail in one of the HAB tiles.
        const unsigned char hab[] = {
            0xf0, 0x6f, 0xff, 0xff, 0x57, 0xff, 0xff, 0x3b, 0xff, 0xff, 0x7d, 0xff, 0xfe, 0xfe, 0xff, 0xfd, 
            0xff, 0x7f, 0xfb, 0xff, 0xbf, 0xf7, 0xff, 0xdf, 0xef, 0xff, 0xef, 0xdf, 0xff, 0xf7, 0xbf, 0xff, 
            0xfb, 0x00, 0x00, 0x01, 0xdf, 0xff, 0xff, 0xdf, 0xf0, 0x1f, 0xd0, 0x37, 0xdf, 0xd6, 0xb7, 0xdf, 
            0xd6, 0xb7, 0xdf, 0xd6, 0xb7, 0xdf, 0xd6, 0xb7, 0xdf, 0xd0, 0x37, 0xdf, 0xdf, 0xf7, 0xdf, 0xdf, 
            0xf7, 0xdf, 0xdf, 0xf7, 0xdf, 0xc0, 0x00, 0x07
        };

        // Iterate through each tile inside the current viewport.
        for (int row = 0; row < VIEWPORT_HEIGHT; row++) {
            for (int col = 0; col < VIEWPORT_WIDTH; col++) {
                // Calculate the corresponding map coordinates.
                int mapX = viewportX + col;
                int mapY = viewportY + row;
                int tile = map[mapY][mapX];
                // Calculate the pixel coordinates on the LCD for the top-left of this tile.
                int x_pixel = col * TILE_SIZE;
                int y_pixel = row * TILE_SIZE + 8;  // Offset of 8 pixels for display margin or header
                
                // Special handling for the HAB area: draw a detailed bitmap in the central HAB tile.
                if (tile == TILE_HAB && mapY == 6 && mapX >= 5 && mapX < 10) {
                    if (mapX == 7) { // Central tile for detailed HAB image.
                        int habBitmapX = (mapX - viewportX) * TILE_SIZE;
                        // Adjust the vertical position so the bitmap appears centered in the tile.
                        int habBitmapY = (mapY - viewportY) * TILE_SIZE + 8 + TILE_SIZE - 24;
                        drawBitmap(lcd, habBitmapX, habBitmapY, hab, 24, 24);
                    }
                } else {
                    // For other tile types, draw a representative graphic.
                    switch (tile) {
                        case TILE_WALL:
                            // Draw wall as a filled rectangle.
                            lcd.drawRect(x_pixel, y_pixel, TILE_SIZE, TILE_SIZE, FILL_BLACK);
                            break;
                        case TILE_ROVER:
                            // Draw the rover with a simple horizontal line.
                            lcd.drawLine(x_pixel, y_pixel + 4, x_pixel + 7, y_pixel + 4, FILL_BLACK);
                            break;
                        case TILE_CRATER:
                            // Draw a crater as a smaller filled rectangle in the tile.
                            lcd.drawRect(x_pixel + 2, y_pixel + 2, 4, 4, FILL_BLACK);
                            break;
                        case TILE_TERMINAL:
                            // Draw the terminal as two crossing diagonal lines.
                            lcd.drawLine(x_pixel + 1, y_pixel + 1, x_pixel + 6, y_pixel + 6, FILL_BLACK);
                            lcd.drawLine(x_pixel + 6, y_pixel + 1, x_pixel + 1, y_pixel + 6, FILL_BLACK);
                            break;
                        default:
                            // If there is no special tile, nothing is drawn.
                            break;
                    }
                }
            }
        }

        // Draw the player indicator as a rectangle on the screen.
        // Calculate player's position relative to the viewport.
        int px = (playerX - viewportX) * TILE_SIZE + 1;
        int py = (playerY - viewportY) * TILE_SIZE + 1 + 8;
        lcd.drawRect(px, py, TILE_SIZE, TILE_SIZE, FILL_BLACK);
        lcd.refresh();  // Update the display to show the new frame.

        // Check if the select button has been pressed to exit exploration mode.
        if (selectButton.read() == 0) {
            while (selectButton.read() == 0) ThisThread::sleep_for(50ms); // Debounce the button press.
            break; // Exit the loop and thus the exploration mode.
        }
        ThisThread::sleep_for(100ms); // Delay for a short period to control frame rate.
    }
}
nclude "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "games.h"

// Global variables for map exploration.
// This creates a 2D array to represent the game map.
int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };

// Global player state (position in the map).
int playerX = 2;
int playerY = 6;
const float GRAVITY = 0.4f;
const float JUMP_FORCE = -1.8f;
const float MAX_FALL_SPEED = 2.0f;

// Global viewport state (defines which part of the map is visible on the LCD).
int viewportX = 0;
int viewportY = 0;

// Helper function to draw a single pixel on the LCD.
// It uses the drawRect function to effectively draw a 1x1 rectangle (a pixel).
static void drawPixel(N5110 &lcd, int x, int y, FillType color) {
    lcd.drawRect(x, y, 1, 1, color);
}

// Custom drawBitmap function that uses the helper drawPixel function.
// It draws a monochrome bitmap by iterating through each bit of the bitmap array.
void drawBitmap(N5110 &lcd, int x, int y, const unsigned char *bitmap, int width, int height) {
    // Calculate the number of bytes per row in the bitmap data.
    int bytesPerRow = (width + 7) / 8;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            // Calculate the index into the bitmap array.
            int byteIndex = row * bytesPerRow + (col / 8);
            // Determine the specific bit in the byte we want to check.
            int bitIndex = 7 - (col % 8);
            // If the bit is set, draw a pixel at the corresponding location.
            if (bitmap[byteIndex] & (1 << bitIndex)) {
                drawPixel(lcd, x + col, y + row, FILL_BLACK);
            }
        }
    }
}

// Updates the viewport's coordinates to center around the player's position.
// It also ensures the viewport doesn't extend outside the boundaries of the map.
void updateViewport() {
    viewportX = playerX - VIEWPORT_WIDTH / 2;
    viewportY = playerY - VIEWPORT_HEIGHT / 2;
    // Clamp the viewport to the left and top edges.
                        case TILE_CRATER: {
                            // Draw a curved crater tile shape
                            for (int dx = 0; dx < TILE_SIZE; dx++) {
                                for (int dy = 0; dy < TILE_SIZE; dy++) {
                                    // Make edge pixels or curved pixel art
                                    if ((dy == 0 && (dx > 2 && dx < 5)) ||          // top arch
                                        (dy == 1 && (dx == 2 || dx == 5)) ||        // side edges
                                        (dy == 2 && (dx == 1 || dx == 6)) ||        // wider base
                                        (dy == 3 && dx >= 2 && dx <= 5)) {          // flat floor
                                        lcd.setPixel(x_pixel + dx, y_pixel + dy);
                                    }
                                }
                            }
                            break;
                        }EWPORT_WIDTH) viewportX = MAP_WIDTH - VIEWPORT_WIDTH;
    if (viewportY > MAP_HEIGHT - VIEWPORT_HEIGHT) viewportY = MAP_HEIGHT - VIEWPORT_HEIGHT;
}

// Main function that implements the map exploration game mode.
void exploreMap(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    if (joystick.button_pressed()) {
    printf("Jump button pressed!\n");
}
    float y_velocity = 0.0f;
    bool on_ground = false;
    bool jumping = false;
    int jump_timer = 0;
    const int MAX_JUMP_FRAMES = 8;  // number of frames player can rise
    // Draw the map's borders by setting the outer edges to walls.
    for (int x = 0; x < MAP_WIDTH; x++) {
        map[0][x] = TILE_WALL;
        map[MAP_HEIGHT - 1][x] = TILE_WALL;
    }
    for (int y = 0; y < MAP_HEIGHT; y++) {
        map[y][0] = TILE_WALL;
        map[y][MAP_WIDTH - 1] = TILE_WALL;
    }
    
    // Place horizontal features on the map (at row 6) for different objects.
    // HAB (Habitat area) is drawn from x = 5 to 9.
    for (int x = 5; x < 10; x++) map[6][x] = TILE_HAB;
    // Rover area is drawn from x = 15 to 19.
    for (int x = 15; x < 20; x++) map[6][x] = TILE_ROVER;
    // Crater area from x = 25 to 29.
    for (int x = 25; x < 30; x++) map[6][x] = TILE_CRATER;
    // Terminal area from x = 35 to 37.
    for (int x = 35; x < 38; x++) map[6][x] = TILE_TERMINAL;

    // Display introductory instructions on the LCD.
    lcd.clear();
    lcd.printString("Explore Mars", 0, 1);
    lcd.printString("Use joystick", 0, 2);
    lcd.printString("to move", 0, 3);
    lcd.printString("Press select", 0, 4);
    lcd.refresh();

    // Wait until the select button is pressed to start the game.
    // First wait until the button is released (if previously pressed),
    // then wait for a new press.
    while (selectButton.read() == 1) ThisThread::sleep_for(100ms);
    while (selectButton.read() == 0) ThisThread::sleep_for(50ms);

    // Main exploration loop of the game.
    while (true) {
        lcd.clear(); // Clear the LCD for drawing the new frame.

        // Horizontal movement
        Direction d = joystick.get_direction();
        int newX = playerX;
        if (d == E) newX++;
        else if (d == W) newX--;

        // Constants (can move these to top)
        const float GRAVITY = 0.25f;
        const float JUMP_FORCE = -1.4f;
        const float MAX_FALL_SPEED = 2.0f;
        const int MAX_JUMP_FRAMES = 10;
        const int COYOTE_FRAMES = 6;

        // State
        static float y_velocity = 0.0f;
        static bool jumping = false;
        static int jump_timer = 0;
        static bool on_ground = false;
        static int coyote_timer = 0;

        // Check if standing on solid tile
        on_ground = (playerY + 1 < MAP_HEIGHT && map[playerY + 1][playerX] != TILE_EMPTY);

        // Handle coyote time
        if (on_ground) {
            coyote_timer = COYOTE_FRAMES;
        } else if (coyote_timer > 0) {
            coyote_timer--;
        }

        // Start jump if button pressed and within coyote time
        if (!jumping && coyote_timer > 0 && joystick.button_pressed()) {
            jumping = true;
            jump_timer = MAX_JUMP_FRAMES;
            y_velocity = JUMP_FORCE;
        }

        // Jump sustain — keep moving up while jump timer active and button held
        if (jumping) {
            if (jump_timer > 0 && joystick.button_pressed()) {
                y_velocity = JUMP_FORCE;
                jump_timer--;
            } else {
                jumping = false;
            }
        }

        // Stop jump early if button released
        if (!joystick.button_pressed()) {
            jumping = false;
            jump_timer = 0;
        }

        // Apply gravity if not grounded or still rising
        if (!on_ground || y_velocity < 0.0f) {
            y_velocity += GRAVITY;
            if (y_velocity > MAX_FALL_SPEED) y_velocity = MAX_FALL_SPEED;
        }

        // Vertical position update
        float newYf = (float)playerY + y_velocity;
        int newY = (int)(newYf + 0.5f);

        // Clamp to map
        if (newY < 0) newY = 0;
        if (newY >= MAP_HEIGHT) {
            newY = MAP_HEIGHT - 1;
            y_velocity = 0;
            jumping = false;
        }

        // Wall collision
        if (map[newY][newX] != TILE_WALL) {
            playerX = newX;
            playerY = newY;
        } else {
            y_velocity = 0;
            jumping = false;
        }

        // Check if movement is valid:
        // 1. The player moved.
        // 2. The new position is within the map boundaries.
        // 3. The target tile is not a wall.
        if ((newX != playerX || newY != playerY) &&
            newX >= 0 && newX < MAP_WIDTH &&
            newY >= 0 && newY < MAP_HEIGHT &&
            map[newY][newX] != TILE_WALL) {
            playerX = newX;
            playerY = newY;
            ThisThread::sleep_for(150ms); // Debounce delay after movement.
        }

        // Update the viewport to center around the updated player position.
        updateViewport();

        // Detailed bitmap for HAB (Habitat) feature.
        // This bitmap is drawn in more detail in one of the HAB tiles.
        const unsigned char hab[] = {
            0xf0, 0x6f, 0xff, 0xff, 0x57, 0xff, 0xff, 0x3b, 0xff, 0xff, 0x7d, 0xff, 0xfe, 0xfe, 0xff, 0xfd, 
            0xff, 0x7f, 0xfb, 0xff, 0xbf, 0xf7, 0xff, 0xdf, 0xef, 0xff, 0xef, 0xdf, 0xff, 0xf7, 0xbf, 0xff, 
            0xfb, 0x00, 0x00, 0x01, 0xdf, 0xff, 0xff, 0xdf, 0xf0, 0x1f, 0xd0, 0x37, 0xdf, 0xd6, 0xb7, 0xdf, 
            0xd6, 0xb7, 0xdf, 0xd6, 0xb7, 0xdf, 0xd6, 0xb7, 0xdf, 0xd0, 0x37, 0xdf, 0xdf, 0xf7, 0xdf, 0xdf, 
            0xf7, 0xdf, 0xdf, 0xf7, 0xdf, 0xc0, 0x00, 0x07
        };

        // Iterate through each tile inside the current viewport.
        for (int row = 0; row < VIEWPORT_HEIGHT; row++) {
            for (int col = 0; col < VIEWPORT_WIDTH; col++) {
                // Calculate the corresponding map coordinates.
                int mapX = viewportX + col;
                int mapY = viewportY + row;
                int tile = map[mapY][mapX];
                // Calculate the pixel coordinates on the LCD for the top-left of this tile.
                int x_pixel = col * TILE_SIZE;
                int y_pixel = row * TILE_SIZE + 8;  // Offset of 8 pixels for display margin or header
                
                // Special handling for the HAB area: draw a detailed bitmap in the central HAB tile.
                if (tile == TILE_HAB && mapY == 6 && mapX >= 5 && mapX < 10) {
                    if (mapX == 7) { // Central tile for detailed HAB image.
                        int habBitmapX = (mapX - viewportX) * TILE_SIZE;
                        // Adjust the vertical position so the bitmap appears centered in the tile.
                        int habBitmapY = (mapY - viewportY) * TILE_SIZE + 8 + TILE_SIZE - 24;
                        drawBitmap(lcd, habBitmapX, habBitmapY, hab, 24, 24);
                    }
                } else {
                    // For other tile types, draw a representative graphic.
                    switch (tile) {
                        case TILE_WALL:
                            // Draw wall as a filled rectangle.
                            lcd.drawRect(x_pixel, y_pixel, TILE_SIZE, TILE_SIZE, FILL_BLACK);
                            break;
                        case TILE_ROVER:
                            // Draw the rover with a simple horizontal line.
                            lcd.drawLine(x_pixel, y_pixel + 4, x_pixel + 7, y_pixel + 4, FILL_BLACK);
                            break;
                        case TILE_CRATER:
                            // Draw a crater as a smaller filled rectangle in the tile.
                            lcd.drawRect(x_pixel + 2, y_pixel + 2, 4, 4, FILL_BLACK);
                            break;
                        case TILE_TERMINAL:
                            // Draw the terminal as two crossing diagonal lines.
                            lcd.drawLine(x_pixel + 1, y_pixel + 1, x_pixel + 6, y_pixel + 6, FILL_BLACK);
                            lcd.drawLine(x_pixel + 6, y_pixel + 1, x_pixel + 1, y_pixel + 6, FILL_BLACK);
                            break;
                        default:
                            // If there is no special tile, nothing is drawn.
                            break;
                    }
                }
            }
        }

        // Draw the player indicator as a rectangle on the screen.
        // Calculate player's position relative to the viewport.
        int px = (playerX - viewportX) * TILE_SIZE + 1;
        int py = (playerY - viewportY) * TILE_SIZE + 1 + 8;
        lcd.drawRect(px, py, TILE_SIZE, TILE_SIZE, FILL_BLACK);
        lcd.refresh();  // Update the display to show the new frame.

        // Check if the select button has been pressed to exit exploration mode.
        if (selectButton.read() == 0) {
            while (selectButton.read() == 0) ThisThread::sleep_for(50ms); // Debounce the button press.
            break; // Exit the loop and thus the exploration mode.
        }
        ThisThread::sleep_for(100ms); // Delay for a short period to control frame rate.
    }
}
