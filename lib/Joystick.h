#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "mbed.h"
#include "Utils.h"

// this value can be tuned to alter tolerance of joystick movement
#define TOL 0.1f
#define RAD2DEG 57.2957795131f



/** Joystick Class
@author Dr Craig A. Evans, University of Leeds
@brief  Library for interfacing with analogue joystick

Example:

@code

#include "mbed.h"
#include "Joystick.h"

//                  y     x     button
Joystick joystick(PTB11,PTB10);

int main() {
    
    joystick.init();
    
    while(1) {
    
        Vector2D coord = joystick.get_coord();
        printf("Coord = %f,%f\n",coord.x,coord.y);
        
        Vector2D mapped_coord = joystick.get_mapped_coord(); 
        printf("Mapped coord = %f,%f\n",mapped_coord.x,mapped_coord.y); 
        
        float mag = joystick.get_mag();
        float angle = joystick.get_angle();
        printf("Mag = %f Angle = %f\n",mag,angle);
        
        Direction d = joystick.get_direction();
        printf("Direction = %i\n",d);
    
          
        wait(0.5);
    }
    
    
}

* @endcode
*/
class Joystick
{
public:
    //                y-pot         x-pot         button
    Joystick(PinName vertPin, PinName horizPin, PinName buttonPin = NC);  // add optional button pin

    void init();  // needs to be called at start with joystick centred

    float get_mag();              // polar
    float get_angle();            // polar
    Vector2D get_coord();         // cartesian co-ordinates x,y
    Vector2D get_mapped_coord();  // x,y mapped to circle
    Direction get_direction();    // N,NE,E,SE etc.
    Polar get_polar();            // mag and angle in struct form

    bool button_pressed();        // <- NEW: check button press

private:
    AnalogIn *vert;
    AnalogIn *horiz;
    DigitalIn *_button = nullptr;  // <- NEW: button input (pointer, optional)

    float _x0;
    float _y0;
};

#endif