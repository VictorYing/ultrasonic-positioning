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

/*
 * position_init:
 * Start positioning.
 */
void position_init(void) ;

/*
 * position_?:
 * Getter functions for position in units of feet, with the origin at the
 * center of the rectangle formed by the transmitters.
 */
float position_x(void) ;
float position_y(void) ;

#endif

/* [] END OF FILE */
