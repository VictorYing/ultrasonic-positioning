/* ========================================
 * speed.h
 * Monica Lu and Victor Ying
 *
 * Speed measurement and control.
 * ========================================
 */

#ifndef SPEED_H
#define SPEED_H


extern float speed;
extern float distance_traveled;
extern float power_output;


/*
 * speed_init:
 * Initializes timers, interrupts, pwm, etc. for speed control. Initializes
 * to coasting state.
 */
void speed_init(void) ;

/*
 * speed_display_info:
 * Print current speed and normalized controller output to the LCD.
 */
void speed_display_info(void) ;

/*
 * speed_pid_start:
 * Sets the speed to the floating point value represented by desired_speed,
 * and enables PID speed control
 */
void speed_pid_start(const char* desired_speed) ;

/*
 * speed_set:
 * Sets the speed_setpoint.
 */
void speed_set(float speed_setpoint) ;

/*
 * speed_brake:
 * Disables PID speed control, and sets main drive motor PWM compare value
 * to 0, and sets state to braking.
 */
void speed_brake(void) ;

/*
 * speed_coast:
 * Disables PID speed control, and sets main drive motor PWM compare value
 * to 0, and sets state to coasting.
 */
void speed_coast(void) ;

/*
 * speed_forward:
 * Changes the state to forward motion.
 */
void speed_forward(void) ;

/*
 * speed_backward:
 * Changes the state to forward motion.
 */
void speed_backward(void) ;

/*
 * speed_set_power:
 * Sets the main drive motor PWM compare value.
 */
void speed_set_power(const char *cmpstr) ;

/*
 * speed_set_k*:
 * Set PID control constants.
 */
void speed_set_kp(const char *val) ;
void speed_set_ki(const char *val) ;
void speed_set_kd(const char *val) ;


#endif

//[] END OF FILE
