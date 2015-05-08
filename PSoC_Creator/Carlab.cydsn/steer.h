/* ========================================
 * steer.h
 * Monica Lu and Victor Ying
 *
 * Line detection and steering control.
 * ========================================
 */

#ifndef STEER_H
#define STEER_H


extern float steer_output;  // Ranges from -1.0 to 1.0.

    
/*
 * steer_init:
 * Initializes timers, interrupts, pwm, etc. for steering the car.
 */
void steer_init(void) ;

/*
 * steer_display_info:
 * Print current steering info to the LCD.
 */
void steer_display_info(void) ;

/*
 * steer_pid_start:
 * Enables PID steering control
 */
void steer_pid_start(void) ;

/*
 * steer_stop:
 * Disables PID steering control.
 */
void steer_stop(void) ;

/*
 * steer_set:
 * Sets the steering angle.
 */
void steer_set(const char *valstr) ;

/*
 * steer_set_steer_k*:
 * Set PID control constants.
 */
void steer_set_kp(const char *val) ;
void steer_set_ki(const char *val) ;
void steer_set_kd(const char *val) ;


#endif

//[] END OF FILE
