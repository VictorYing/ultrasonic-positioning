/* ========================================
 * speed.c
 * Monica Lu and Victor Ying
 *
 * Speed measurement and control.
 * ========================================
 */

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "speed.h"
#include "steer.h"
#include "drive.h"
#include "usb_uart.h"

#define DISTANCE_PER_TICK 0.1285  // in feet
#define DERIV_CONTROL_AVERAGING 3
#define TIMER_COUNTS_PER_SECOND 1000000.0  // timers are fed by 1 MHz clock
#define PID_INTERVALS_PER_SECOND 100.0  // PID control is reevaluated every 10 ms

enum state {
    COAST = 0u,
    FORWARD = 1u,
    BACKWARD = 2u,
    BRAKE = 3u,
};

// Because the speed sensor is on the right side of the car, it underestimates
// speed during right turns and overestimates during left turns. This is for a
// compensating correction term.
#define SPEED_ADJUSTMENT_COEFFICIENT (-0.12)


static CY_ISR_PROTO(hall_handler) ;
static CY_ISR_PROTO(speed_pid_handler) ;
static void speed_pid_control(void) ;

static enum state current_state;

float speed = 0.0;
float distance_traveled = 0.0;
float power_output = 0.0;

static uint16 count_since_magnet = 0u;
static uint8 speed_pid_enabled = 0u;  // Boolean value indicating whether or not to do PID speed control.
static float speed_setpoint = 0.0;  // in feet per second
static float kp = 0.1;  // in normalized control output adjustment per (feet per second) error.
static float ki = 0.2;  // in normalized control output adjustment per (feet per second) error per second.
static float kd = 0.0;  // in normalized control output adjustment per (change in feet per second per second).


static void set_state(enum state desired) {
    current_state = desired;
    Drive_Control_Reg_Wakeup();
    Drive_Control_Reg_Write(desired);
}


/*
 * speed_init:
 * Initializes timers, pwm, etc. for controlling speed. Initializes
 * to coasting state.
 */
void speed_init(void) {
    uint8 status = CyEnterCriticalSection();
    
    speed_coast();

    Hall_Timer_Start();
    Hall_IRQ_Start();
    Hall_IRQ_SetVector(hall_handler);

    Speed_PID_Timer_Start();
    Speed_PID_IRQ_Start();
    Speed_PID_IRQ_SetVector(speed_pid_handler);
    
    Drive_PWM_Start();
    
    CyExitCriticalSection(status);
}

/*
 * speed_display_info:
 * Print current speed and normalized controller output to the LCD.
 */
void speed_display_info(void) {
    char strbuf[17];
    
    uint8 status = CyEnterCriticalSection();
    
    // Print out info
    sprintf(strbuf, "Speed:  %.6f", speed);
    LCD_Position(0,0);
    LCD_PrintString(strbuf);
    if (speed_pid_enabled) {
        sprintf(strbuf, "Control:%.6f", power_output);
        LCD_Position(1,0);
        LCD_PrintString(strbuf);
    }
    
    CyExitCriticalSection(status);
    
    CyDelay(50u);
}

/*
 * speed_pid_start:
 * Sets the speed to the floating point value represented by desired_speed,
 * and enables PID speed control
 */
void speed_pid_start(const char* desired_speed) {
    uint8 status = CyEnterCriticalSection();
    
    if (*desired_speed != '\0')
        speed_setpoint = atof(desired_speed);
    speed_pid_enabled = 1;

    CyExitCriticalSection(status);
}

/*
 * speed_set:
 * Sets the speed_setpoint.
 */
void speed_set(float desired) {
    uint8 status = CyEnterCriticalSection();
    
    speed_setpoint = desired;
    
    CyExitCriticalSection(status);
}

/*
 * speed_coast:
 * Disables PID speed control, and sets main drive motor PWM compare value
 * to 0, and sets state to coasting.
 */
void speed_coast(void) {
    uint8 status = CyEnterCriticalSection();
    
    speed_pid_enabled = 0;
    Drive_PWM_WriteCompare(0);
    set_state(COAST);
    
    CyExitCriticalSection(status);
}

/*
 * speed_brake:
 * Disables PID speed control, and sets main drive motor PWM compare value
 * to 0, and sets state to braking.
 */
void speed_brake(void) {
    uint8 status = CyEnterCriticalSection();
    
    speed_pid_enabled = 0;
    Drive_PWM_WriteCompare(0);
    if (current_state == FORWARD || current_state == BACKWARD) {
        set_state(COAST);
        CyDelay(1);
    }
    set_state(BRAKE);
    
    CyExitCriticalSection(status);
}

/*
 * speed_forward:
 * Changes the state to forward motion.
 */
void speed_forward(void) {
    uint8 status = CyEnterCriticalSection();
    
    if (current_state == BRAKE || current_state == BACKWARD) {
        set_state(COAST);
        CyDelay(1);
    }
    set_state(FORWARD);
    
    CyExitCriticalSection(status);
}

/*
 * speed_backward:
 * Changes the state to forward motion.
 */
void speed_backward(void) {
    uint8 status = CyEnterCriticalSection();
    
    if (current_state == BRAKE || current_state == FORWARD) {
        set_state(COAST);
        CyDelay(1);
    }
    set_state(BACKWARD);
    
    CyExitCriticalSection(status);
}

/*
 * speed_set_power:
 * Sets the main drive motor PWM compare value.
 */
void speed_set_power(const char *valstr) {
    uint8 status = CyEnterCriticalSection();

    if (speed_pid_enabled) {
        usb_uart_putline(
            "Cannot manually set power while PID control is running!");
    }
    else {
        uint16 cmp;
    
        power_output = atof(valstr);
        if (power_output > 1.0)
            power_output = 1.0;
        else if (power_output < 0.0)
            power_output = 0.0;
        cmp = (uint16)(power_output * USHRT_MAX);
        Drive_PWM_WriteCompare(cmp);
    }

    CyExitCriticalSection(status);
}

/*
 * speed_set_k*:
 * Set PID control constants.
 */
void speed_set_kp(const char *val) {
    uint8 status = CyEnterCriticalSection();
    kp = atof(val);
    CyExitCriticalSection(status);
}
void speed_set_ki(const char *val) {
    uint8 status = CyEnterCriticalSection();
    ki = atof(val);
    CyExitCriticalSection(status);
}
void speed_set_kd(const char *val) {
    uint8 status = CyEnterCriticalSection();
    kd = atof(val);
    CyExitCriticalSection(status);
}


/*
 * hall_handler:
 * Should run every time a magnet is seen.
 * Recalculates the current speed estimate.
 */
static CY_ISR(hall_handler) {
    static uint32 prev;
    uint32 val = Hall_Timer_ReadCapture();
    uint32 diff = prev - val;
    float time;
    uint8 status;
    
    prev = val;
    
    time = (float)diff / TIMER_COUNTS_PER_SECOND;
    speed = DISTANCE_PER_TICK / time;  // in units of feet/second
    speed /= 1.0 + steer_output * SPEED_ADJUSTMENT_COEFFICIENT;
    
    count_since_magnet = 0;
    
    distance_traveled += DISTANCE_PER_TICK /
            (1.0 + steer_output * SPEED_ADJUSTMENT_COEFFICIENT);
            
    status = CyEnterCriticalSection();
    magnet_callback();
    CyExitCriticalSection(status);
    
    // Clear interrupt
    Hall_Timer_ReadStatusRegister();
}

/*
 * speed_pid_handler:
 * Should run once every 10 ms interval. Does speed control.
 */
static CY_ISR(speed_pid_handler) {
    uint8 saved_interrupt_status;
    
    // If a magnet is not seen for a long time, decrease the speed estimate
    if (count_since_magnet > 0) {
        float time_since_magnet = count_since_magnet / PID_INTERVALS_PER_SECOND;
        float speed_estimate = DISTANCE_PER_TICK / time_since_magnet;
        speed_estimate /= 1.0 + steer_output * SPEED_ADJUSTMENT_COEFFICIENT;
        if (speed_estimate < speed)
            speed = speed_estimate;
    }
    count_since_magnet++;
    
    saved_interrupt_status = CyEnterCriticalSection();
    if (speed_pid_enabled && 
            (current_state == FORWARD || current_state == BACKWARD))
        speed_pid_control();
    CyExitCriticalSection(saved_interrupt_status);
}

static void speed_pid_control(void) {
    float error, deriv, next_riemann_sum,
          prop_control, integ_control, deriv_control;
    static float riemann_sum = 0.0;
    static float prev_errors[DERIV_CONTROL_AVERAGING];
    static uint8 prev_errors_index = 0;
    float change;
    uint16 pwm_cmp;
    uint8 status;

    // Begin control calculations
    error = speed_setpoint - speed;
    
    // Proportional control
    prop_control = kp * error;
    
    // Integral control
    next_riemann_sum = riemann_sum + error / PID_INTERVALS_PER_SECOND; 
    integ_control = ki * next_riemann_sum;
    
    // Derivative control
    change = error - prev_errors[prev_errors_index];
    deriv = change * PID_INTERVALS_PER_SECOND / (float)DERIV_CONTROL_AVERAGING;
    prev_errors[prev_errors_index] = error;
    prev_errors_index = (prev_errors_index + 1) % DERIV_CONTROL_AVERAGING;
    deriv_control = kd * deriv;
    
    // Add it all together, with limiting to valid duty cycle values
    power_output = 0.15 + prop_control + integ_control + deriv_control;
    if (power_output < 0.0) {
        power_output = 0.0;
    }
    else if (power_output > 1.0) {
        power_output = 1.0;
    }
    else {
        // Anti-windup: only allow integrator to build up if not saturating
        riemann_sum = next_riemann_sum;  
    }
    
    // Scale control value to a PWM compare value
    pwm_cmp = (uint16)(USHRT_MAX * power_output);
    status = CyEnterCriticalSection();
    Drive_PWM_WriteCompare(pwm_cmp);
    CyExitCriticalSection(status);
}


//[] END OF FILE
