#ifndef UTILS_H
#define UTILS_H

struct Position2D {
  int x;
  int y;  
};

enum Direction {
    CENTRE,  // 0
    N,       // 1
    NE,      // 2
    E,       // 3
    SE,      // 4
    S,       // 5
    SW,      // 6
    W,       // 7
    NW      // 8
};

struct UserInput {
    Direction d;
    float mag;
};

struct Vector2D {
  float x;
  float y;  
};

struct Polar {
    float mag;
    float angle;
};

#endif