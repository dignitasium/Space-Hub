#include "MapEditor.h"

MapEditor::MapEditor(N5110 &lcd, Joystick &joystick, DigitalIn &select)
    : lcd(lcd), joystick(joystick), selectButton(select) {
    cursorX = 0;
    cursorY = 0;
    selectedTile = 1;
    viewportX = 0;
    viewportY = 0;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = 0;
        }
    }
}

void MapEditor::run() {
    while (true) {
        update();
        drawMap();
        drawCursor();
        drawTileSelector();
        lcd.refresh();
        ThisThread::sleep_for(100ms);
    }
}

void MapEditor::update() {
    Direction d = joystick.get_direction();

    if (d == N && cursorY > 0) cursorY--;
    else if (d == S && cursorY < MAP_HEIGHT - 1) cursorY++;
    else if (d == E && cursorX < MAP_WIDTH - 1) cursorX++;
    else if (d == W && cursorX > 0) cursorX--;

    if (joystick.button_pressed()) {
        map[cursorY][cursorX] = selectedTile;
    }

    if (selectButton.read() == 0) {
        selectedTile = (selectedTile + 1) % TILE_TYPE_COUNT;
        while (selectButton.read() == 0) ThisThread::sleep_for(50ms);
    }

    // Export if user long-presses select
    static int pressDuration = 0;
    if (selectButton.read() == 0) {
        pressDuration++;
        if (pressDuration > 30) {
            exportMap();
            pressDuration = 0;
        }
    } else {
        pressDuration = 0;
    }
}

void MapEditor::drawMap() {
    lcd.clear();
    viewportX = cursorX - 5;
    viewportY = cursorY - 2;
    if (viewportX < 0) viewportX = 0;
    if (viewportY < 0) viewportY = 0;
    if (viewportX > MAP_WIDTH - 10) viewportX = MAP_WIDTH - 10;
    if (viewportY > MAP_HEIGHT - 4) viewportY = MAP_HEIGHT - 4;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 10; col++) {
            int mapX = viewportX + col;
            int mapY = viewportY + row;
            int tile = map[mapY][mapX];
            int px = col * TILE_SIZE;
            int py = row * TILE_SIZE + 8;
            switch (tile) {
                case 1:
                    lcd.drawLine(px, py + 7, px + 7, py + 7, FILL_BLACK);
                    break;
                case 2:
                    lcd.drawRect(px + 1, py + 5, 6, 3, FILL_BLACK);
                    break;
                case 3:
                    lcd.drawRect(px, py, 8, 8, FILL_BLACK);
                    break;
                case 4:
                    lcd.drawLine(px, py + 4, px + 7, py + 4, FILL_BLACK);
                    break;
                case 5:
                    lcd.setPixel(px + 3, py + 2);
                    lcd.setPixel(px + 4, py + 3);
                    break;
                default:
                    break;
            }
        }
    }
}

void MapEditor::drawCursor() {
    int cx = (cursorX - viewportX) * TILE_SIZE;
    int cy = (cursorY - viewportY) * TILE_SIZE + 8;
    lcd.drawRect(cx, cy, TILE_SIZE, TILE_SIZE, FILL_TRANSPARENT);
}

void MapEditor::drawTileSelector() {
    char buf[16];
    sprintf(buf, "Tile: %d", selectedTile);
    lcd.printString(buf, 0, 0);
}

void MapEditor::exportMap() {
    printf("int map[%d][%d] = {\n", MAP_HEIGHT, MAP_WIDTH);
    for (int y = 0; y < MAP_HEIGHT; y++) {
        printf("  {");
        for (int x = 0; x < MAP_WIDTH; x++) {
            printf("%d", map[y][x]);
            if (x < MAP_WIDTH - 1) printf(",");
        }
        printf("}%s\n", y < MAP_HEIGHT - 1 ? "," : "");
    }
    printf("};\n");
}