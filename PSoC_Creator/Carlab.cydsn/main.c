/* ========================================
 *
 *   Michael Danielczuk and Andrew Kim
 *   main.c
 *   ELE 302
 *
 * ========================================
*/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "position.h"

// Defining constants
#define maxTime 65535.0
#define LCD_NUM_COLUMNS 16
#define pi 3.14159265359
#define clockSpeed 50000.0
#define alpha 0.2

static const float normSpeed = 6.0;

// Define boolean type
typedef enum { false, true } bool; 

// Global variables changed across interrupts
volatile uint16 times[5] = {maxTime/5, maxTime/5, maxTime/5, maxTime/5, maxTime/5};
volatile float prevEMA = 0.0;
volatile uint16 prevTime = maxTime;
volatile float prevError[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
volatile float integral = 0.0;
volatile uint8 UAcount = 0;
volatile uint8 UApos = 0;
volatile uint16 straightGuess = 9000;
volatile float sKp = 2.0;
volatile float sKd = 1.0;
volatile uint16 maxRight = 12000;
volatile uint16 maxLeft = 6000;
volatile uint16 optWidth = 90;
volatile uint16 widthError = 20;
volatile float sErrorTimes[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
volatile int magCount = 0;
volatile float stCorr = 0.000004;

// Struct of changeable variables that we want to save across PSOC off
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float maxInt;
    int openGuess;
    int maxOut;
    int minOut;
    float wDiameter;
    float setSpeed;
    float setDistance;
} config;

// Declaration of struct of saveable variables
config c = {5000.0, 450.0, 500.0, 15.0, 8000, 25000, 0, 0.216*12.0/12.7, 6.0, 1550};


// PID control function. Takes the current speed as input, calculates the
// error and returns the new comparator value for the PWM module.
float errorCal(float currentSpeed) {
    float output, derivative, error;
    int i;
    char buf[16];
    
    // Error calculated as difference between set speed and current speed
    error = c.setSpeed-currentSpeed;
   
    // Integral term accumulates error unless it is greater than a maximum integral
    // value, in which case it stays at that maximum value. This helps to minimize
    // integrator windup and overshoot.
    integral += error;
    if (integral > c.maxInt) {
        integral = c.maxInt;
    }
    if (integral < -c.maxInt) {
        integral = -c.maxInt;
    }
    
    // Derivative is calculated as the slope of the error with respect to 
    // the previous error from this magnet (e.g. last rotation) to minimize noise
    // effects on the slope.
    derivative = error-prevError[4];
    
    // Output is calculated using the weights for each term and the set open loop guess.
    output = c.openGuess + c.Kp*error + c.Ki*integral + c.Kd*derivative;
     
    // Integral output on LCD for testing
    //sprintf(buf, "%2.4f", integral);
    //LCD_Position(0,9);
    //LCD_PrintString(buf);
    
    // If output is too high or too low, output is set to the appropriate minimum or maximum value
    // so that the car does not accelerate or decelerate too fast.
    /*if (output > c.maxOut) {
        output = c.maxOut;
    }*/
    if (output < c.minOut) {
        output = c.minOut;
    }
    
    // Shifts the previous error values and enters the new error value
    for (i=4; i>0; i--) {
        prevError[i] = prevError[i-1];
    }
    prevError[0] = error;
    
    return output;
}

// Receives interrupt as input. Reads the time of the interrupt from the timer, calculates the time
// of the previous wheel rotation, then uses it to find the speed of the car. Then, it finds the new
// duty cycle to adjust the speed to the correct speed and sets the PWM module accordingly.
CY_ISR(interHE) {

    char buf[16];
    // Reads the time of the interrupt from the Timer module
    uint16 intTime = HETimer_ReadCapture();
    float currentSpeed, EMA, newDuty;
    uint16 diffTime, circTime;
    int i;
    
    // Clears the interrupt from the Timer now that the time of interrupt has been read
    HETimer_ReadStatusRegister();
    
    // Calculates the time since the previous interrupt
    if (intTime < prevTime) {
        diffTime = prevTime-intTime;
    }
    else {
        diffTime = maxTime-intTime+prevTime;
    }
    
    // Sets the previous time to the new time, shifts the previous times back one
    // and enters the newly recorded time. Also calculates the time that the past 
    // rotation has taken (for this one magnet to go around the wheel).
    prevTime = intTime;
    circTime = diffTime;
    for (i=4; i>0; i--) {
        times[i] = times[i-1];
        circTime += times[i];
    }
    times[0] = diffTime;
    
    // Finds the current speed based on the clock speed, time of rotation, and
    // wheel circumference, correcting for turning (when one wheel moves faster)
    currentSpeed = (clockSpeed*c.wDiameter*pi)/((float)circTime);
    currentSpeed *= (1.0 + stCorr*(straightGuess-PWMServo_ReadCompare()));
    /* Exponential moving average (if speed is noisy)
    EMA = alpha*currentSpeed + (1.0-alpha)*prevEMA;
    prevEMA = EMA; */
    
    // Calls errorCal to find the new duty cycle and writes it to the PWM module
    newDuty = errorCal(currentSpeed);
    PWMMotor_WriteCompare(newDuty);

    // Outputs the current speed and the comparator value from the PWM module
    // to the LCD sceen of the PSoC
    //sprintf(buf, "%2.4f", currentSpeed);
    //LCD_Position(0,9);
    //LCD_PrintString(buf);
    //LCD_Position(1,0);
    //LCD_PrintNumber(PWM_Motor_ReadCompare());
    
    // Distance Tracker
    magCount++;
    //LCD_Position(1,0);
    //LCD_PrintNumber(magCount);
    
    // Speed up on straightaways (and decrease sKp to cut down on oscillations)
    if (((magCount < c.setDistance) && (magCount > c.setDistance-170)) || ((magCount < c.setDistance/2 - 15) && (magCount > (c.setDistance/2)-195))) {
        c.setSpeed = 10.0;
        sKp = 0.8;
        stCorr = 0.0;
    }
    
    // Stop after going the distance
    else if (magCount > c.setDistance)
        c.setSpeed = 0.0;
    
    // Otherwise, proceed as normal
    else {
       c.setSpeed = normSpeed;
       sKp = 2.0;
       stCorr = 0.000004;
   }
}

// Steering interrupt routine
CY_ISR(interLine) {

     char buf[8];
    // Reads the time of the interrupt from the Timer module
    uint16 startTime, startLine, endLine, endTime, lineWidth, newSteer;
    float leftTime, rightTime, errorTime, sumErrors;
    int i;
    
    // Read times from Timer FIFO and clear the interrupt
    startTime = SteerTimer_ReadCapture();
    startLine = SteerTimer_ReadCapture();
    endLine = SteerTimer_ReadCapture();
    endTime = SteerTimer_ReadCapture();
    SteerTimer_ReadStatusRegister();
    
    // Calculate times (to line, width of line, and after line)
    if (startLine > startTime) { 
        leftTime = maxTime - startLine + startTime;
    }
    else {
        leftTime = startTime - startLine;
    }
    if (endTime > endLine) {
        rightTime = maxTime - endTime + endLine;
    }
    else {
        rightTime = endLine - endTime;
    }
    
    if (endLine > startLine) {
        lineWidth = maxTime - endLine + startLine;
    }
    else {
        lineWidth = startLine - endLine;
    }
    
    // If abnormal line width, disregard interrupt (steer in same as last one)
    if (abs(optWidth-lineWidth) > widthError)
        return;
    errorTime = leftTime - rightTime;
    
    // Keep array of previous errors
    sumErrors = errorTime;
    for (i=0; i<4; i++) {
        sErrorTimes[i] = sErrorTimes[i+1];
        sumErrors += sErrorTimes[i];
    }
    sErrorTimes[i] = errorTime;
    
    // Set new PWM for steering servo without going over maximum pulse width
    newSteer = straightGuess + sKp*errorTime + sKd*(errorTime-(float)sumErrors/5.0);
    if (newSteer > maxRight)
        newSteer = maxRight;
    if (newSteer < maxLeft)
        newSteer = maxLeft;
    PWMServo_WriteCompare(newSteer);
}



void main() {

    int i = 0;
    
    // Enable Global interrupts and start the interrupts, Timers, Counter and PWMs
    CyGlobalIntEnable;
    HE_inter_Start();
    HETimer_Start();
    PWMMotor_Start();
    PWMServo_Start();
    line_inter_Start();
    LineCounter_Start();
    SteerTimer_Start();

    position_init();
    
    // Sets the vector for the HE_inter interrupt and sends an interrupt to
    // start the car moving
    HE_inter_SetVector(interHE);
    line_inter_SetVector(interLine);
    PWMMotor_WriteCompare(errorCal(0.0));
    
    // Starts the UART and EEPROM modules
    UART_Start();
    EEPROM_Start();
   
    // Starts the LCD and prints Speed on the first line
    LCD_Start();
    //LCD_Position(0,0);
    //LCD_PrintString("Speed");
    
    
    
    
    for(;;) {
    //Uncomment for Reading from serial port
     /*   
        char8 readChar;
        char8 readData[20];
        char8 str[20];
        float x;
        
        int j = 0;
        char8 newVal[10];
        
        
        
        if ((readChar = UART_GetChar()) > 0) {
            // Data received from the Serial port 
            UART_PutChar(readChar);
            if (readChar != '\r') {
                readData[i++] = readChar;
            }
            else {
                UART_PutChar('\n');
                readData[i] = '\0';
                if (!strcmp(readData, "change Kp")) {
                    sprintf(str, "Kp = %f\n\r", c.Kp);
                    UART_PutString(str);
                    UART_PutString("New value = ");
                    while ((readChar = UART_GetChar()) != '\r'){
                        newVal[j++] = readChar;
                        UART_PutChar(readChar);
                    }
                    newVal[j] = '\0';
                    UART_PutChar(readChar);
                    UART_PutChar('\n');
                    UART_PutString(newVal);
                    c.Kp = atof(newVal);
                    sprintf(str, "Kp has been changed to %f\n\r", c.Kp);
                    UART_PutString(str);
                    j = 0;
                }
                i = 0;
            }
                
            LCD_Position(1,0);
            LCD_PrintString(readData);
        }*/
    }    
}

/* [] END OF FILE */
