#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "games.h"
#include <cstdlib>
#include <ctime>

// --- Game Constants ---
static int score = 0;
static int level = 1;
static int game_speed = 0;
static int enemy_phase = 0;
static int enemy_0_pos = 2, enemy_1_pos = 2;
static bool enemy_dead = true;
static int playerLane = 2;
static bool control = true;
static int combo = 0;
static bool invincible = false;
static int invincible_frames = 0;

struct Projectile { int x, y; bool active; };
static Projectile bullet = {0, 0, false};

// --- Sprites ---
const unsigned char ship[] = {
    0x00,0x00,0x01,0x00,0x03,0x80,0x02,0x80,0x02,0xC0,
    0x07,0xC0,0x0D,0xE0,0x1F,0xF0,0x3F,0xF8,0x7F,0xFC,
    0x7F,0xFC,0x7F,0xFC,0x1F,0xF0,0x07,0xE0,0x00,0x00
};

const unsigned char enemy[] = {
    0x05,0xE0,0x0B,0xF0,0x03,0xF0,0x33,0xF8,0x7F,0xFC,
    0xBF,0xFA,0x77,0xDC,0x7E,0xFC,0x3F,0xFC,0xEF,0xEE,
    0xC1,0x86,0x81,0x82,0x80,0x82,0x00,0x00
};

// --- Utility Drawing ---
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

static void drawHUD(N5110 &lcd) {
    lcd.drawLine(0, 0, 0, 47, FILL_BLACK);
    lcd.drawLine(50, 0, 50, 47, FILL_BLACK);
    lcd.drawLine(0, 47, 50, 47, FILL_BLACK);
    
    char buffer[16];
    sprintf(buffer, "Lv:%d", level); lcd.printString(buffer, 52, 0);
    sprintf(buffer, "Sp:%d", game_speed); lcd.printString(buffer, 52, 1);
    sprintf(buffer, "Sc:%d", score); lcd.printString(buffer, 52, 2);
}

static void enemyShip(N5110 &lcd, int lane, int phase) {
    int x = (lane - 1) * 16 + 2;
    drawBitmap(lcd, x, phase, enemy, 15, 15);
}

static void playerShip(N5110 &lcd, int lane) {
    int x = (lane - 1) * 16 + 2;
    drawBitmap(lcd, x, 32, ship, 15, 15);
}

// --- Transitions ---
static void lightShow(N5110 &lcd, int lvl) {
    for (int i = 0; i < 6; i++) {
        lcd.clear();
        if (i % 2 == 0) lcd.drawRect(0, 0, 84, 48, FILL_BLACK);
        lcd.refresh();
        ThisThread::sleep_for(100ms);
    }
    lcd.clear();
    char buffer[16];
    sprintf(buffer, "LEVEL %d", lvl);
    lcd.printString(buffer, 18, 2);
    lcd.refresh();
    ThisThread::sleep_for(1200ms);
}

// --- Level & Difficulty ---
static void levelControl(N5110 &lcd) {
    static bool leveled_up = false;
    static int previous_level = 0;

    if (score >= 10 && !leveled_up) {
        level++; score = 0;
        lightShow(lcd, level);
        leveled_up = true;
    } else if (score < 10) {
        leveled_up = false;
    }

    game_speed = (level <= 5) ? level - 1 : 5;
    previous_level = level;
}

// --- Game Over ---
static void gameOver(N5110 &lcd) {
    while (true) {
        lcd.clear();
        lcd.printString("GAME OVER", 10, 3);
        lcd.refresh();
        ThisThread::sleep_for(400ms);
    }
}

// --- Main Game ---
void spaceInvadeGame(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    // Reset game state
    score = 0; level = 1; game_speed = 0;
    enemy_phase = 0; enemy_dead = true;
    playerLane = 2; control = true;
    combo = 0; invincible = false; invincible_frames = 0;
    bullet.active = false;

    srand(time(NULL));

    // Welcome screen
    lcd.clear();
    lcd.printString("Space Invaders", 0, 1);
    lcd.printString("Press select", 0, 2);
    lcd.refresh();

    while (selectButton.read() == 1) ThisThread::sleep_for(100ms);
    while (selectButton.read() == 0) ThisThread::sleep_for(50ms);

    // Main loop
    while (true) {
        lcd.clear();

        // Joystick input
        Direction d = joystick.get_direction();
        if (d == W && playerLane > 1 && control) { playerLane--; control = false; }
        else if (d == E && playerLane < 3 && control) { playerLane++; control = false; }
        else if (d == CENTRE) { control = true; }

        // Fire bullet
        if (joystick.button_pressed() && !bullet.active) {
            bullet.x = (playerLane - 1) * 16 + 9;
            bullet.y = 32;
            bullet.active = true;
        }

        // Update bullet
        if (bullet.active) {
            bullet.y -= 2;
            if (bullet.y < 0) bullet.active = false;
            else lcd.drawRect(bullet.x, bullet.y, 2, 4, FILL_BLACK);
        }

        // Draw player ship
        if (!invincible || (invincible_frames % 4 < 2)) {
            playerShip(lcd, playerLane);
        }

        // Spawn enemies
        if (enemy_dead) {
            enemy_0_pos = playerLane;
            enemy_1_pos = (rand() % 3) + 1;
            enemy_phase = 0;
            enemy_dead = false;
        }

        // Draw enemies
        enemyShip(lcd, enemy_0_pos, enemy_phase);
        enemyShip(lcd, enemy_1_pos, enemy_phase);
        enemy_phase++;

        // Bullet collision with enemy
        if (bullet.active && enemy_phase <= bullet.y + 4) {
            int bullet_lane = bullet.x < 15 ? 1 : bullet.x < 31 ? 2 : 3;
            if (bullet_lane == enemy_0_pos || bullet_lane == enemy_1_pos) {
                score++; combo++;
                if (combo >= 3) {
                    invincible = true;
                    invincible_frames = 100;
                }
                enemy_dead = true;
                bullet.active = false;
            }
        }

        // Invincibility decay
        if (invincible) {
            invincible_frames--;
            if (invincible_frames <= 0) {
                invincible = false;
                combo = 0;
            }
        }

        // Collision with enemy
        if (!invincible && enemy_phase > 22 && (enemy_0_pos == playerLane || enemy_1_pos == playerLane)) {
            gameOver(lcd);
        }

        // Missed enemy
        if (enemy_phase > 40) {
            enemy_dead = true;
            score++;
        }

        // Update level and UI
        levelControl(lcd);
        drawHUD(lcd);
        lcd.refresh();

        // Adjust game speed
        switch (game_speed) {
            case 0: ThisThread::sleep_for(80ms); break;
            case 1: ThisThread::sleep_for(70ms); break;
            case 2: ThisThread::sleep_for(60ms); break;
            case 3: ThisThread::sleep_for(50ms); break;
            case 4: ThisThread::sleep_for(40ms); break;
            default: ThisThread::sleep_for(30ms); break;
        }

        // Exit game
        if (selectButton.read() == 0) {
            while (selectButton.read() == 0) ThisThread::sleep_for(50ms);
            break;
        }
    }
}
