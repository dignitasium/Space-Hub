#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "games.h"

// Global variables for map exploration.
int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };
int playerX = 2;
int playerY = 6;
int playerYOffset = 0;
int viewportX = 0;
int viewportY = 0;

static void drawPixel(N5110 &lcd, int x, int y, FillType color) {
    lcd.drawRect(x, y, 1, 1, color);
}

void drawBitmap(N5110 &lcd, int x, int y, const unsigned char *bitmap, int width, int height) {
    int bytesPerRow = (width + 7) / 8;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int byteIndex = row * bytesPerRow + (col / 8);
            int bitIndex = 7 - (col % 8);
            if (bitmap[byteIndex] & (1 << bitIndex)) {
                drawPixel(lcd, x + col, y + row, FILL_BLACK);
            }
        }
    }
}

void updateViewport() {
    viewportX = playerX - VIEWPORT_WIDTH / 2;
    viewportY = playerY - VIEWPORT_HEIGHT / 2;
    if (viewportX < 0) viewportX = 0;
    if (viewportY < 0) viewportY = 0;
    if (viewportX > MAP_WIDTH - VIEWPORT_WIDTH) viewportX = MAP_WIDTH - VIEWPORT_WIDTH;
    if (viewportY > MAP_HEIGHT - VIEWPORT_HEIGHT) viewportY = MAP_HEIGHT - VIEWPORT_HEIGHT;
}

void exploreMap(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    for (int x = 0; x < MAP_WIDTH; x++) {
        map[0][x] = TILE_WALL;
        map[MAP_HEIGHT - 1][x] = TILE_WALL;
    }
    for (int y = 0; y < MAP_HEIGHT; y++) {
        map[y][0] = TILE_WALL;
        map[y][MAP_WIDTH - 1] = TILE_WALL;
    }

    for (int x = 5; x < 10; x++) map[6][x] = TILE_HAB;
    for (int x = 15; x < 20; x++) map[6][x] = TILE_ROVER;

    for (int y = 5; y <= 7; y++) {
        for (int x = 25; x <= 29; x++) map[y][x] = TILE_EMPTY;
    }
    map[5][26] = TILE_CRATER;
    map[5][27] = TILE_CRATER;
    map[5][28] = TILE_CRATER;
    map[6][25] = TILE_CRATER;
    map[6][29] = TILE_CRATER;
    for (int x = 25; x <= 29; x++) map[7][x] = TILE_CRATER;

    for (int x = 35; x < 38; x++) map[6][x] = TILE_TERMINAL;

    lcd.clear();
    lcd.printString("Explore Mars", 0, 1);
    lcd.printString("Use joystick", 0, 2);
    lcd.printString("to move", 0, 3);
    lcd.printString("Press select", 0, 4);
    lcd.refresh();

    while (selectButton.read() == 1) ThisThread::sleep_for(100ms);
    while (selectButton.read() == 0) ThisThread::sleep_for(50ms);

    float y_velocity = 0.0f;
    bool jumping = false;
    int jump_timer = 0;
    bool on_ground = false;
    int coyote_timer = 0;
    const int MAX_JUMP_FRAMES = 10;
    const int COYOTE_FRAMES = 6;
    const float GRAVITY = 0.25f;
    const float JUMP_FORCE = -0.8f;
    const float MAX_FALL_SPEED = 2.0f;

    while (true) {
        lcd.clear();

        Direction d = joystick.get_direction();
        int newX = playerX;
        if (d == E) newX++;
        else if (d == W) newX--;

        on_ground = (playerY + 1 < MAP_HEIGHT && map[playerY + 1][playerX] != TILE_EMPTY);

        if (on_ground) {
            coyote_timer = COYOTE_FRAMES;
        } else if (coyote_timer > 0) {
            coyote_timer--;
        }

        if (!jumping && coyote_timer > 0 && joystick.button_pressed()) {
            jumping = true;
            jump_timer = MAX_JUMP_FRAMES;
            y_velocity = JUMP_FORCE;
        }

        if (jumping) {
            if (jump_timer > 0 && joystick.button_pressed()) {
                y_velocity = JUMP_FORCE;
                jump_timer--;
            } else {
                jumping = false;
            }
        }

        if (!joystick.button_pressed()) {
            jumping = false;
            jump_timer = 0;
        }

        if (!on_ground || y_velocity < 0.0f) {
            y_velocity += GRAVITY;
            if (y_velocity > MAX_FALL_SPEED) y_velocity = MAX_FALL_SPEED;
        }

        float newYf = (float)playerY + y_velocity;
        int newY = (int)(newYf + 0.5f);
        if (newY < 0) newY = 0;
        if (newY >= MAP_HEIGHT) {
            newY = MAP_HEIGHT - 1;
            y_velocity = 0;
            jumping = false;
        }

        if (map[newY][newX] != TILE_WALL) {
            playerX = newX;
            playerY = newY;
        } else {
            y_velocity = 0;
            jumping = false;
        }

        updateViewport();

        const unsigned char hab[] = {
            0xf0, 0x6f, 0xff, 0xff, 0x57, 0xff, 0xff, 0x3b, 0xff, 0xff, 0x7d, 0xff, 0xfe, 0xfe, 0xff, 0xfd,
            0xff, 0x7f, 0xfb, 0xff, 0xbf, 0xf7, 0xff, 0xdf, 0xef, 0xff, 0xef, 0xdf, 0xff, 0xf7, 0xbf, 0xff,
            0xfb, 0x00, 0x00, 0x01, 0xdf, 0xff, 0xff, 0xdf, 0xf0, 0x1f, 0xd0, 0x37, 0xdf, 0xd6, 0xb7, 0xdf,
            0xd6, 0xb7, 0xdf, 0xd6, 0xb7, 0xdf, 0xd6, 0xb7, 0xdf, 0xd0, 0x37, 0xdf, 0xdf, 0xf7, 0xdf, 0xdf,
            0xf7, 0xdf, 0xdf, 0xf7, 0xdf, 0xc0, 0x00, 0x07
        };

        for (int row = 0; row < VIEWPORT_HEIGHT; row++) {
            for (int col = 0; col < VIEWPORT_WIDTH; col++) {
                int mapX = viewportX + col;
                int mapY = viewportY + row;
                int tile = map[mapY][mapX];
                int x_pixel = col * TILE_SIZE;
                int y_pixel = row * TILE_SIZE + 8;

                if (tile == TILE_HAB && mapY == 6 && mapX >= 5 && mapX < 10) {
                    if (mapX == 7) {
                        int habBitmapX = (mapX - viewportX) * TILE_SIZE;
                        int habBitmapY = (mapY - viewportY) * TILE_SIZE + 8 + TILE_SIZE - 24;
                        drawBitmap(lcd, habBitmapX, habBitmapY, hab, 24, 24);
                    }
                } else {
                    switch (tile) {
                        case TILE_WALL:
                            lcd.drawRect(x_pixel, y_pixel, TILE_SIZE, TILE_SIZE, FILL_BLACK);
                            break;
                        case TILE_ROVER:
                            lcd.drawLine(x_pixel, y_pixel + 4, x_pixel + 7, y_pixel + 4, FILL_BLACK);
                            break;
                        case TILE_CRATER: {
                            for (int dx = 0; dx < TILE_SIZE; dx++) {
                                for (int dy = 0; dy < TILE_SIZE; dy++) {
                                    if ((dy == 0 && (dx > 2 && dx < 5)) ||
                                        (dy == 1 && (dx == 2 || dx == 5)) ||
                                        (dy == 2 && (dx == 1 || dx == 6)) ||
                                        (dy == 3 && dx >= 2 && dx <= 5)) {
                                        lcd.setPixel(x_pixel + dx, y_pixel + dy);
                                    }
                                }
                            }
                            break;
                        }
                        case TILE_TERMINAL:
                            lcd.drawLine(x_pixel + 1, y_pixel + 1, x_pixel + 6, y_pixel + 6, FILL_BLACK);
                            lcd.drawLine(x_pixel + 6, y_pixel + 1, x_pixel + 1, y_pixel + 6, FILL_BLACK);
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        int px = (playerX - viewportX) * TILE_SIZE + 1;
        int py = (playerY - viewportY) * TILE_SIZE + 1 + 8 - playerYOffset;
        lcd.drawRect(px, py, TILE_SIZE, TILE_SIZE, FILL_BLACK);
        lcd.refresh();

        if (selectButton.read() == 0) {
            while (selectButton.read() == 0) ThisThread::sleep_for(50ms);
            break;
        }
        ThisThread::sleep_for(100ms);
    }
}