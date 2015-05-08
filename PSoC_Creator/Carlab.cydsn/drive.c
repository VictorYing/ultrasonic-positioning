/* ========================================
 * drive.c
 * Monica Lu and Victor Ying
 *
 * High-level movement program.
 * ========================================
 */
 
#include <project.h>
 
#include "drive.h"
#include "speed.h"
#include "steer.h"


/*
 * drive_init:
 * Begins driving the car.
 */
void drive_init(void) {
    steer_init();
    speed_init();
    
    steer_pid_start();
    speed_forward();
    speed_pid_start("0.5"); 
}

/*
 * magnet_callback:
 * Called on every hall sensor tick with a running total distance traveled.
 */
void magnet_callback(void) {
    // Do about half a lap
    if (distance_traveled >= 50.0)
        speed_brake();
    
    /*
    // Do two laps and maximum speed
    if (distance_traveled < 1.0)
        ;
    else if ((distance_traveled > 3.0 && distance_traveled < 20.0) ||
             (distance_traveled > 100.0 && distance_traveled < 118.0))
        speed_set(9.0);
    else if (distance_traveled < 194.0)
        speed_set(5.7);
    else
        speed_brake(); */
}


//[] END OF FILE
