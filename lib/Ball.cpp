#include "Ball.h"

Ball::Ball() {}

void Ball::init(int size, int speed) {
    //printf("Ball: Init\n");
    _size = size;
    _x = WIDTH/2 -  _size/2;
    _y = HEIGHT/2 - _size/2;

    _velocity.x = speed;
    _velocity.y = speed;
}

void Ball::draw(N5110 &lcd) {
    //printf("Ball: Draw\n");
    lcd.drawRect(_x,_y,_size,_size,FILL_BLACK);
}

void Ball::update(){
    //printf("Ball: Update\n");     
    _x += _velocity.x;
    _y += _velocity.y;
}

void Ball::set_velocity(Position2D v){
    printf("Ball: Velocity\n");
    _velocity.x = v.x;
    _velocity.y = v.y;
}

void Ball::set_pos(Position2D p) {
    printf("Ball: Set Position\n");
    _x = p.x;
    _y = p.y;
}

Position2D Ball::get_velocity(){ return {_velocity.x,_velocity.y}; }

Position2D Ball::get_pos() { return {_x,_y}; }

int Ball::get_size() { return _size; }

