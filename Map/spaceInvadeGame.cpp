#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "games.h"

#include <cstdlib>
#include <ctime>

// Bitmaps
const unsigned char ship[] = {
    0x00,0x00, 0x01,0x00, 0x03,0x80, 0x02,0x80, 0x02,0xC0,
    0x07,0xC0, 0x0D,0xE0, 0x1F,0xF0, 0x3F,0xF8, 0x7F,0xFC,
    0x7F,0xFC, 0x7F,0xFC, 0x1F,0xF0, 0x07,0xE0, 0x00,0x00
};

const unsigned char enemy[] = {
    0x05,0xE0, 0x0B,0xF0, 0x03,0xF0, 0x33,0xF8, 0x7F,0xFC,
    0xBF,0xFA, 0x77,0xDC, 0x7E,0xFC, 0x3F,0xFC, 0xEF,0xEE,
    0xC1,0x86, 0x81,0x82, 0x80,0x82, 0x00,0x00
};

struct Projectile {
    int x, y;
    bool active;
};

static int score = 0, game_speed = 0, enemy_phase = 0;
static int enemy_0_pos = 2, enemy_1_pos = 2;
static bool enemy_dead = true;
static int playerLane = 2;
static bool control = true;
static Projectile bullet = {0, 0, false};
static int combo = 0;
static bool invincible = false;
static int invincible_frames = 0;

static void drawBitmap(N5110 &lcd, int x, int y, const unsigned char *bitmap, int width, int height) {
    int bytesPerRow = (width + 7) / 8;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int byteIndex = row * bytesPerRow + (col / 8);
            int bitIndex = 7 - (col % 8);
            if (bitmap[byteIndex] & (1 << bitIndex)) {
                lcd.drawRect(x + col, y + row, 1, 1, FILL_BLACK);
            }
        }
    }
}

static void gamescreen(N5110 &lcd) {
    lcd.drawLine(0, 0, 0, 47, FILL_BLACK);
    lcd.drawLine(50, 0, 50, 47, FILL_BLACK);
    lcd.drawLine(0, 47, 50, 47, FILL_BLACK);
    char buffer[16];
    sprintf(buffer, "Speed:%d", game_speed);
    lcd.printString(buffer, 42, 0);
    sprintf(buffer, "Score:%d", score);
    lcd.printString(buffer, 42, 2);
}

static void enemy_ship(N5110 &lcd, int lane, int phase) {
    int x = lane == 1 ? 2 : lane == 2 ? 18 : 34;
    drawBitmap(lcd, x, phase, enemy, 15, 15);
}

static void player_car(N5110 &lcd, int lane) {
    int x = lane == 1 ? 2 : lane == 2 ? 18 : 34;
    drawBitmap(lcd, x, 32, ship, 15, 15);
}

// Adjusts the game speed based on the current score by setting delay values.
// As the score increases, the delay decreases, and the game speeds up.
static void Level_Controller() {
    if (score >= 0 && score <= 10) {
        game_speed = 0;
        ThisThread::sleep_for(80ms);
    } else if (score > 10 && score <= 20) {
        game_speed = 1;
        ThisThread::sleep_for(70ms);
    } else if (score > 20 && score <= 30) {
        game_speed = 2;
        ThisThread::sleep_for(60ms);
    } else if (score > 30 && score <= 40) {
        game_speed = 3;
        ThisThread::sleep_for(50ms);
    } else {
        ThisThread::sleep_for(40ms);
    }
}

// Displays the game over screen indefinitely after a collision.
static void game_over(N5110 &lcd) {
    while (true) {
        ThisThread::sleep_for(100ms);
        lcd.clear();
        lcd.printString("GAME OVER", 10, 3);
        lcd.refresh();
        
    }
}

void spaceInvadeGame(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    score = game_speed = enemy_phase = combo = 0;
    enemy_dead = true;
    playerLane = 2;
    control = true;
    invincible = false;
    invincible_frames = 0;
    bullet.active = false;
    srand(time(NULL));

    lcd.clear();
    lcd.printString("Space Invaders", 0, 1);
    lcd.printString("Press select", 0, 2);
    lcd.refresh();
    while (selectButton.read() == 1) ThisThread::sleep_for(100ms);
    while (selectButton.read() == 0) ThisThread::sleep_for(50ms);

    while (true) {
        lcd.clear();
        gamescreen(lcd);

        Direction d = joystick.get_direction();
        if (d == W && playerLane > 1 && control) {
            playerLane--; control = false;
        } else if (d == E && playerLane < 3 && control) {
            playerLane++; control = false;
        } else if (d == CENTRE) {
            control = true;
        }

        if (joystick.button_pressed() && !bullet.active) {
            bullet.x = playerLane == 1 ? 9 : playerLane == 2 ? 25 : 41;
            bullet.y = 32;
            bullet.active = true;
        }

        if (bullet.active) {
            bullet.y -= 2;
            if (bullet.y < 0) bullet.active = false;
            else lcd.drawRect(bullet.x, bullet.y, 2, 4, FILL_BLACK);
        }

        if (!invincible || (invincible && (invincible_frames % 4 < 2))) {
            player_car(lcd, playerLane);
        }

        if (enemy_dead) {
            enemy_0_pos = playerLane;
            enemy_1_pos = (rand() % 3) + 1;
            enemy_phase = 0;
            enemy_dead = false;
        }

        enemy_ship(lcd, enemy_0_pos, enemy_phase);
        enemy_ship(lcd, enemy_1_pos, enemy_phase);
        enemy_phase++;

        if (bullet.active && enemy_phase <= bullet.y + 4) {
            int bullet_lane = bullet.x < 15 ? 1 : bullet.x < 31 ? 2 : 3;
            if (bullet_lane == enemy_0_pos || bullet_lane == enemy_1_pos) {
                score++;
                combo++;
                if (combo >= 3) {
                    invincible = true;
                    invincible_frames = 100;
                }
                enemy_dead = true;
                bullet.active = false;
            }
        }

        if (invincible) {
            invincible_frames--;
            if (invincible_frames <= 0) {
                invincible = false;
                combo = 0;
            }
        }

        if (!invincible && enemy_phase > 22 && (enemy_0_pos == playerLane || enemy_1_pos == playerLane)) {
            game_over(lcd);
        }

        if (enemy_phase > 40) {
            enemy_dead = true;
        }

        Level_Controller();
        lcd.refresh();

        if (selectButton.read() == 0) {
            while (selectButton.read() == 0) ThisThread::sleep_for(50ms);
            break;
        }
    }
}
