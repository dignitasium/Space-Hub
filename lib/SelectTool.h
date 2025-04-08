#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "mbed.h"
#include "N5110.h"
#include "Utils.h"    // for Position2D
#include <math.h>

// If M_PI is not defined, define it.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief A hovering select tool to indicate the current menu item.
 *
 * This class represents an indicator that “hovers” (oscillates horizontally)
 * next to a menu option. Its base (anchor) position is set by the menu code,
 * and update() applies a sine‑based horizontal offset.
 */
class SelectTool {
public:
    SelectTool();

    /**
     * @brief Initialize the select tool.
     * @param baseX The base (anchor) X coordinate.
     * @param baseY The base (anchor) Y coordinate.
     * @param width The width of the tool.
     * @param height The height of the tool.
     * @param amplitude The maximum horizontal offset (hover amplitude).
     * @param phaseDelta The amount to change the phase on each update.
     */
    void init(int baseX, int baseY, int width, int height, float amplitude, float phaseDelta);

    /// Update the tool (applies the hover effect).
    void update();

    /// Draw the tool on the LCD.
    void draw(N5110 &lcd);

    /// Set the base position (anchor) for the tool.
    void setPosition(int baseX, int baseY);

    /// Get the current position (including hover offset).
    Position2D get_pos();

private:
    int _base_x;    // Anchor x-position (set by menu)
    int _base_y;    // Anchor y-position (set by menu)
    int _x;         // Current x-position (base + hover offset)
    int _y;         // Current y-position (same as _base_y)
    int _width;     
    int _height;
    float _amplitude;   // Maximum horizontal displacement for hover effect
    float _phase;       // Current phase for sine oscillation
    float _phaseDelta;  // How much the phase changes per update
};

#endif