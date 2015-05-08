/* ========================================
 * steer.c
 * Monica Lu and Victor Ying
 *
 * Line detection and steering control.
 * ========================================
 */

#include <project.h>
#include <stdio.h>
#include <stdlib.h>

#include "steer.h"
#include "usb_uart.h"


#define FASTEST_CLK_FREQ 48000000.0  // 48 MHz
#define EXPECTED_ROW_LEN (0.000045 * FASTEST_CLK_FREQ)  // in clock cycles
#define ACCEPTABLE_ROW_MARGIN 0.2
#define MIN_ACCEPTABLE_ROW_LEN ((uint16)(EXPECTED_ROW_LEN * (1.0 - ACCEPTABLE_ROW_MARGIN)))
#define MAX_ACCEPTABLE_ROW_LEN ((uint16)(EXPECTED_ROW_LEN * (1.0 + ACCEPTABLE_ROW_MARGIN)))
#define PID_INTERVALS_PER_SECOND 60.0 // Camera is 30 fps interlaced
#define DERIV_CONTROL_AVERAGING 4
#define STEERING_CENTER 1500 // 1.5 ms pulse = steer straight ahead


static CY_ISR_PROTO(camera_handler) ;
static void steer_pid_control(void) ;


float steer_output = 0.0;

static float measurement = 0.0;
static uint8 steer_pid_enabled = 0; // Boolean value indicating whether or not to do PID steering control.
static float kp = 0.75;
static float ki = 0.0;
static float kd = 0.03;


/*
 * steer_init:
 * Initializes timers, interrupts, pwm, etc. for steering the car.
 */
void steer_init(void) {
    steer_stop();
    
    Camera_Comp_Start();
    Camera_Counter_Start();
    Camera_Timer_Start();
    Camera_IRQ_Start();
    Camera_IRQ_SetVector(camera_handler);

    Steering_PWM_Start();
}

/*
 * steer_display_info:
 * Print current measurements and normalized controller output to the LCD.
 */
void steer_display_info(void) {
    char strbuf[17];
    
    // Print out info
    sprintf(strbuf, "Meas.: %f  ", measurement);
    LCD_Position(0,0);
    LCD_PrintString(strbuf);
    sprintf(strbuf, "Steer: %f ", steer_output);
    LCD_Position(1,0);
    LCD_PrintString(strbuf);
    CyDelay(50u);
}

/*
 * steer_pid_start:
 * Enables PID steering control
 */
void steer_pid_start(void) {
    steer_pid_enabled = 1u;
}

/*
 * steer_stop:
 * Disables PID steering control.
 */
void steer_stop(void) {
    steer_pid_enabled = 0u;
}

/*
 * steer_set:
 * Sets the steering angle.
 */
void steer_set(const char *valstr) {
    uint8 status = CyEnterCriticalSection();
    
    if (steer_pid_enabled) {
        usb_uart_putline(
            "Cannot manually set steering while PID control is running!");
    }
    else {
        uint16 cmp;
        steer_output = atof(valstr);
        if (steer_output > 1.0)
            steer_output = 1.0;
        else if (steer_output < -1.0)
            steer_output = -1.0;
        cmp = (int16)(500.0 * steer_output) + STEERING_CENTER;
        Steering_PWM_WriteCompare(cmp);
    }
    
    CyExitCriticalSection(status);
}

/*
 * steer_set_k*:
 * Set PID control constants.
 */
void steer_set_kp(const char *val) {
    kp = atof(val);
}
void steer_set_ki(const char *val) {
    ki = atof(val);
}
void steer_set_kd(const char *val) {
    kd = atof(val);
}

static CY_ISR(camera_handler) {
    uint16 row_start_time = Camera_Timer_ReadCapture();
    uint16 black_start_time = Camera_Timer_ReadCapture();
    uint16 black_end_time = Camera_Timer_ReadCapture();
    uint16 row_end_time = Camera_Timer_ReadCapture();
    uint16 black_mid_time, row_mid_time;
    uint8 saved_interrupt_status;
    
    // Only update our measurement of where the black strip is if the row looks
    // like a full row containing a single black strip. (This effectively tosses
    // out data at intersections.)
    uint16 row_length = row_start_time - row_end_time;
    if (row_length >= MIN_ACCEPTABLE_ROW_LEN &&
            row_length <= MAX_ACCEPTABLE_ROW_LEN) {
        black_mid_time = black_end_time + (black_start_time - black_end_time)/2u;
        row_mid_time = row_end_time + row_length/2u;
    
        measurement = (float)(int16)(black_mid_time - row_mid_time) * 2 / (float)row_length;
    }
    
    // Do PID steering control
    saved_interrupt_status = CyEnterCriticalSection();
    if (steer_pid_enabled)
        steer_pid_control();
    CyExitCriticalSection(saved_interrupt_status);
    
    // Clear interrupt
    Camera_Timer_ReadStatusRegister();
}

/*
 * steer_pid_control:
 * Should run once after every new video frame. Does steering control.
 */
static void steer_pid_control(void) {
    float error, deriv, next_riemann_sum,
          prop_control, integ_control, deriv_control;
    static float riemann_sum = 0.0;
    static float prev_errors[DERIV_CONTROL_AVERAGING] = {0};
    static uint8 prev_errors_index = 0;
    float change;
    uint16 pwm_cmp;
    uint8 status;

    // Begin control calculations
    error = - measurement;
    
    // Proportional control
    prop_control = kp * error;
    
    // Integral control
    next_riemann_sum = riemann_sum + error/PID_INTERVALS_PER_SECOND; 
    integ_control = ki * next_riemann_sum;
    
    // Derivative control
    change = error - prev_errors[prev_errors_index];
    deriv = change * PID_INTERVALS_PER_SECOND / (float)DERIV_CONTROL_AVERAGING;
    prev_errors[prev_errors_index] = error;
    prev_errors_index = (prev_errors_index + 1) % DERIV_CONTROL_AVERAGING;
    deriv_control = kd * deriv;
    
    // Add it all together, with limiting to valid duty cycle values
    steer_output = prop_control + integ_control + deriv_control;
    if (steer_output < -1.0) {
        steer_output = -1.0;
    }
    else if (steer_output > 1.0) {
        steer_output = 1.0;
    }
    else {
        // Anti-windup: only allow integrator to build up if not saturating
        riemann_sum = next_riemann_sum;  
    }
    
    // Scale control value to a PWM compare value
    pwm_cmp = (int16)(500.0 * steer_output) + STEERING_CENTER;
    status = CyEnterCriticalSection();
    Steering_PWM_WriteCompare(pwm_cmp);
    CyExitCriticalSection(status);
}

//[] END OF FILE
