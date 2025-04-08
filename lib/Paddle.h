#ifndef PADDLE_H
#define PADDLE_H

#include "mbed.h"
#include "N5110.h"
#include "Utils.h"

class Paddle
{
public:

    Paddle();
    void init(int x,int height,int width);
    void draw(N5110 &lcd);
    void update(UserInput input);
    void add_score();
    int get_score();
    Position2D get_pos();
    int get_height();
    int get_width();

private:

    int _height;
    int _width;
    int _x;
    int _y;
    int _speed;
    int _score;

};
#endif