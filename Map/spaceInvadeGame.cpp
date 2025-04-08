#include "mbed.h"
#include "N5110.h"
#include "Joystick.h"
#include "games.h"  // Contains game constants and function declaration

#include <cstdlib>   // For random number generation
#include <ctime>     // For seeding the random number generator

// Bitmaps for the player ship and enemy ship (15x15 pixels)
const unsigned char ship[] = {
    0x00,0x00,
    0x01,0x00,
    0x03,0x80,
    0x02,0x80,
    0x02,0xC0,
    0x07,0xC0,
    0x0D,0xE0,
    0x1F,0xF0,
    0x3F,0xF8,
    0x7F,0xFC,
    0x7F,0xFC,
    0x7F,0xFC,
    0x1F,0xF0,
    0x07,0xE0,
    0x00,0x00
};

const unsigned char enemy[] = {
    0x05,0xE0,
    0x0B,0xF0,
    0x03,0xF0,
    0x33,0xF8,
    0x7F,0xFC,
    0xBF,0xFA,
    0x77,0xDC,
    0x7E,0xFC,
    0x3F,0xFC,
    0xEF,0xEE,
    0xC1,0x86,
    0x81,0x82,
    0x80,0x82,
    0x00,0x00
};

// Global game variables
static int score = 0;            // Player's score
static int game_speed = 0;       // Current game speed level (affects delay)
static int enemy_phase = 0;      // Vertical position offset for enemy ships
static int enemy_0_pos = 2;      // Lane for the first enemy ship
static int enemy_1_pos = 2;      // Lane for the second enemy ship
static bool enemy_dead = true;   // Flag to trigger enemy re-spawn
static int playerLane = 2;       // The lane (1, 2, or 3) where the player's ship resides
static bool control = true;      // Input control flag to avoid multiple lane changes per joystick move

// Local version of drawBitmap: Draws a monochrome bitmap on the LCD at (x, y)
// using individual pixels based on the provided bitmap data.
static void drawBitmap(N5110 &lcd, int x, int y, const unsigned char *bitmap, int width, int height) {
    int bytesPerRow = (width + 7) / 8;  // Calculate the number of bytes per row
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int byteIndex = row * bytesPerRow + (col / 8);  // Locate the correct byte in the array
            int bitIndex = 7 - (col % 8);                   // Find the bit index within this byte
            if (bitmap[byteIndex] & (1 << bitIndex)) {       // Check if the bit is set
                lcd.drawRect(x + col, y + row, 1, 1, FILL_BLACK);  // Draw pixel if bit is 1
            }
        }
    }
}

// Draws the game border and displays the speed and score information.
static void gamescreen(N5110 &lcd) {
    // Draw vertical and horizontal borders
    lcd.drawLine(0, 0, 0, 47, FILL_BLACK);
    lcd.drawLine(50, 0, 50, 47, FILL_BLACK);
    lcd.drawLine(0, 47, 50, 47, FILL_BLACK);
    
    // Display game speed and score using formatted strings.
    char buffer[16];
    sprintf(buffer, "Speed:%d", game_speed);
    lcd.printString(buffer, 42, 0);
    sprintf(buffer, "Score:%d", score);
    lcd.printString(buffer, 42, 2);
}

// Draws an enemy ship in a given lane (1-3) at the vertical position specified by 'phase'.
static void enemy_ship(N5110 &lcd, int lane, int phase) {
    int x = 0;
    // Determine x-coordinate based on enemy lane position
    if (lane == 1)      x = 2;
    else if (lane == 2) x = 18;
    else if (lane == 3) x = 34;
    // Draw the enemy bitmap using the calculated position.
    drawBitmap(lcd, x, phase, enemy, 15, 15);
}

// Draws the player's ship in the specified lane at a fixed vertical position.
static void player_car(N5110 &lcd, int lane) {
    int x = 0, y = 32; // Fixed vertical position near the bottom of the screen
    // Determine x-coordinate based on player's lane
    if (lane == 1)      x = 2;
    else if (lane == 2) x = 18;
    else if (lane == 3) x = 34;
    // Draw the player's ship bitmap.
    drawBitmap(lcd, x, y, ship, 15, 15);
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

// Main game function for the Space Invaders style game.
void spaceInvadeGame(N5110 &lcd, Joystick &joystick, DigitalIn &selectButton) {
    // Initialize game variables and seed the random number generator.
    score = 0;
    game_speed = 0;
    enemy_phase = 0;
    enemy_dead = true;       // Start with no enemy on screen
    playerLane = 2;          // Start the player in the center lane
    control = true;          // Enable control input
    srand(time(NULL));       // Seed random number generation for enemy lane randomization

    // Display an instruction screen and wait for the select button to start the game.
    lcd.clear();
    lcd.printString("Space Invaders", 0, 1);
    lcd.printString("Press select", 0, 2);
    lcd.refresh();
    while (selectButton.read() == 1) {
        ThisThread::sleep_for(100ms);
    }
    while (selectButton.read() == 0) {
        ThisThread::sleep_for(50ms);
    }
    
    // Main game loop
    while (true) {
        lcd.clear();
        gamescreen(lcd); // Draw borders, speed, and score

        // Read joystick input to change lanes.
        // Use a left (W) or right (E) movement to adjust the player's lane.
        Direction d = joystick.get_direction();
        if (d == W && playerLane > 1 && control) {
            playerLane--;    // Move to the left lane
            control = false; // Prevent rapid multiple lane changes
        } else if (d == E && playerLane < 3 && control) {
            playerLane++;    // Move to the right lane
            control = false;
        } else {
            control = true;  // Re-enable control when joystick is neutral
        }
        
        // Draw the player ship in the updated lane.
        player_car(lcd, playerLane);
        
        // Enemy ship management:
        // If there are no enemies on the screen, spawn new enemy ships.
        if (enemy_dead) {
            enemy_0_pos = playerLane;             // First enemy appears in the same lane as the player
            enemy_1_pos = (rand() % 3) + 1;         // Second enemy in a random lane (1 to 3)
            enemy_phase = 0;                      // Reset enemy vertical position
            enemy_dead = false;                   // Mark that enemies are now active
        }
        
        // Draw enemy ships based on their current phase (vertical position)
        enemy_ship(lcd, enemy_0_pos, enemy_phase);
        enemy_ship(lcd, enemy_1_pos, enemy_phase);
        enemy_phase++;  // Move the enemy ships downward for the next frame
        
        // Check for collision:
        // When enemy ships are close to the player (enemy_phase > 22) and an enemy is in the same lane as the player,
        // trigger the game over sequence.
        if (enemy_phase > 22 && (enemy_0_pos == playerLane || enemy_1_pos == playerLane)) {
            game_over(lcd);
        }
        // If enemy ships pass completely off the screen (enemy_phase > 40), mark them as dead and increment the score.
        if (enemy_phase > 40) {
            enemy_dead = true;
            score++;
        }
        
        // Adjust the game speed based on the current score.
        Level_Controller();
        lcd.refresh();  // Update the display with all drawings

        // Exit the game if the select button is pressed.
        if (selectButton.read() == 0) {
            // Debounce the select button press before breaking out of the loop.
            while (selectButton.read() == 0) {
                ThisThread::sleep_for(50ms);
            }
            break;
        }
    }
}
