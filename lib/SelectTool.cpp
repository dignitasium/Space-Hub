#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "SelectTool.h"

SelectTool::SelectTool() : _base_x(0), _base_y(0), _x(0), _y(0),
                           _width(0), _height(0),
                           _amplitude(0), _phase(0), _phaseDelta(0) { }

void SelectTool::init(int baseX, int baseY, int width, int height, float amplitude, float phaseDelta) {
    _base_x = baseX;
    _base_y = baseY;
    _width = width;
    _height = height;
    _amplitude = amplitude;
    _phaseDelta = phaseDelta;
    _phase = 0;
    _x = _base_x;
    _y = _base_y;
}

void SelectTool::update() {
    // Increase phase for oscillation
    _phase += _phaseDelta;
    if (_phase > 2 * M_PI) {
        _phase -= 2 * M_PI;
    }
    // Calculate horizontal offset using sine function
    int offset = (int)(_amplitude * sin(_phase));
    _x = _base_x + offset;
    // _y remains as the base y
}

void SelectTool::draw(N5110 &lcd) {
    // Draw the select tool as a filled rectangle.
    lcd.drawRect(_x, _base_y, _width, _height, FILL_BLACK);
}

void SelectTool::setPosition(int baseX, int baseY) {
    _base_x = baseX;
    _base_y = baseY;
    _x = _base_x;
    _y = _base_y;
}

Position2D SelectTool::get_pos() {
    Position2D pos = { _x, _base_y };
    return pos;
}
