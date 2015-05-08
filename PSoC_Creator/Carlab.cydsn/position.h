/* ===========================================================
 *
 * position.h
 * Michael Danielczuk, Andrew Kim, Monica Lu, and Victor Ying
 *
 * Provides positioning using time difference of arrival
 * multilateration with four transmitters arranged in a
 * rectangle.
 *
 * ===========================================================
 */

#ifndef POSITION_H
#define POSITION_H

#include <project.h>

/*
 * position_init:
 * Start positioning.
 */
void position_init(void) ;

/*
 * position_data_available:
 * returns nonzero if new data since the last time this function was called.
 */
uint8 position_data_available(void) ;

/*
 * position_?:
 * Getter functions for position in units of feet, with the origin at the
 * center of the rectangle formed by the transmitters.
 */
float position_x(void) ;
float position_y(void) ;

/*
 * error:
 * Getter function for error in units of feet squared.
 */
float error(void) ;

#endif

/* [] END OF FILE */
