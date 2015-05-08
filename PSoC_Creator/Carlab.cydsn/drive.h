/* ========================================
 * drive.h
 * Monica Lu and Victor Ying
 *
 * High-level movement program.
 * ========================================
 */

#ifndef DRIVE_H
#define DRIVE_H


/*
 * drive_init:
 * Begins driving the car.
 */
void drive_init(void) ;

/*
 * magnet_callback:
 * Called on every hall sensor tick with a running total distance traveled.
 */
void magnet_callback(void) ;

#endif

//[] END OF FILE
