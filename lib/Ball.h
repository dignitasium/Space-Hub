#ifndef BALL_H
#define BALL_H

#include "mbed.h"
#include "N5110.h"
#include "Utils.h"  // for Position


/** Ball Class
@author Dr Craig A. Evans, University of Leeds
@brief Controls the ball in the Pong game 
@date Febraury 2017
*/ 

class Ball
{

public:
    Ball();
    void init(int size,int speed);
    void draw(N5110 &lcd);
    void update();
    /// accessors and mutators
    void set_velocity(Position2D v);
    Position2D get_velocity();
    Position2D get_pos();
    int get_size();
    void set_pos(Position2D p);
    
private:

    Position2D _velocity;
    int _size;
    int _x;
    int _y;
};
#endif