#include "PongEngine.h"

PongEngine::PongEngine(){ _lives = 4; }    

void PongEngine::init(int paddle_position, int paddle_height, int paddle_width, int ball_size, int speed){
    //printf("Pong Engine: Init\n");
    _ball.init(ball_size,speed);
    _paddle.init(paddle_position, paddle_height, paddle_width);
}

int PongEngine::update(UserInput input) {   
    //printf("Pong Engine: Update\n");
    check_goal();  // checking for a goal is a priority 
    _ball.update();
    _paddle.update(input);
    // important to update paddles and ball before checking collisions so can
    // correct for it before updating the display
    check_wall_collision();
    check_paddle_collision();
    
    return _lives;
}

void PongEngine::draw(N5110 &lcd) {
    //printf("Pong Engine: Draw\n");
    // draw the elements in the LCD buffer
    // pitch
    lcd.drawLine(0,0,WIDTH-1,0,1);  // top
    lcd.drawLine(WIDTH-1,0,WIDTH-1,HEIGHT-1,1);  // back wall
    lcd.drawLine(0,HEIGHT-1,WIDTH-1,HEIGHT-1,1); // bottom
    _ball.draw(lcd);
    _paddle.draw(lcd);
}

void PongEngine::check_wall_collision() {
    //printf("Pong Engine: Check Wall Collision\n");
    // read current ball attributes
    Position2D ball_pos = _ball.get_pos();
    Position2D ball_velocity = _ball.get_velocity();
    int size = _ball.get_size();

    // check if hit top wall
    if (ball_pos.y <= 1) {  //  1 due to 1 pixel boundary
        ball_pos.y = 1;  // bounce off ceiling without going off screen
        ball_velocity.y = -ball_velocity.y;  // flip velocity
    } else if (ball_pos.y + size >= (HEIGHT-1) ) {
        // hit bottom
        ball_pos.y = (HEIGHT-1) - size;  // stops ball going off screen
        ball_velocity.y = -ball_velocity.y;    // flip velcoity 
    } else if (ball_pos.x + size >= (WIDTH-1) ) {
        // hit right wall
        ball_pos.x = (WIDTH-1) - size;  // stops ball going off screen
        ball_velocity.x = -ball_velocity.x;    // flip velcoity 
    } 

    // update ball parameters
    _ball.set_velocity(ball_velocity);
    _ball.set_pos(ball_pos);
}

void PongEngine::check_paddle_collision() {
    //printf("Pong Engine: Check Paddle Collision\n");
    // read current ball and paddle attributes
    Position2D ball_pos = _ball.get_pos();
    Position2D ball_velocity = _ball.get_velocity();
    Position2D paddle_pos = _paddle.get_pos();  // paddle

    // see if ball has hit the paddle by checking for overlaps
    if (
        (ball_pos.y >= paddle_pos.y) && //top
        (ball_pos.y <= paddle_pos.y + _paddle.get_height() ) && //bottom
        (ball_pos.x >= paddle_pos.x) && //left
        (ball_pos.x <= paddle_pos.x + _paddle.get_width() )  //right
    ) {
        // if it has, fix position and reflect x velocity
        ball_pos.x = paddle_pos.x + _paddle.get_width();
        ball_velocity.x = -ball_velocity.x;
    }

    // write new attributes
    _ball.set_velocity(ball_velocity);
    _ball.set_pos(ball_pos);
}

void PongEngine::check_goal() {
    //printf("Pong Engine: Check Goal\n");
    Position2D ball_pos = _ball.get_pos();
    int size = _ball.get_size();
    int speed = abs(_ball.get_velocity().x);  // speed is magnitude of velocity
    // check if ball position has gone off the left
    if (ball_pos.x + size < 0) {
        // reset the ball
        _ball.init(size,speed);
        _lives--;  // lose a life
    }   
}