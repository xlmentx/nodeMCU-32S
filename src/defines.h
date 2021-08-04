#ifndef ESTOP_DEFINES_H
#define ESTOP_DEFINES_H

#include <Arduino.h>

// -------------------------------------------------------------------
// I/O Pins (Hardcoded, PCB V1.0)
// -------------------------------------------------------------------

// Generic PWM Outputs
#define PWM_CH1 16
#define PWM_CH2 2
#define PWM_CH3 12

// Generic RC Inputs
#define RC_CH1 36
#define RC_CH2 39
#define RC_CH3 34
#define RC_CH4 35

// RGB LED
#define RGB_R 14
#define RGB_G 27
#define RGB_B 26

// Red/Blu LED
#define RB_R  4
#define RB_B  0

// BLDC Hall Effect Sensor Pins
#define BLDC1 32
#define BLDC2 33
#define BLDC3 25

// Generic I2C Bus x2
#define I2C1_SDA  21
#define I2C1_SCL  22
#define I2C2_SDA  15
#define I2C2_SCL  5

// Generic SPI Bus
#define SPI_MISO  19
#define SPI_MOSI  23
#define SPI_CLK   18
#define SPI_CS    17

// Output Enable (input pin)
// For emulating Adafruit PCA9685 PWM Driver, use with I2C1 bus
#define PWM_OE    13

// -------------------------------------------------------------------
// I/O Pin Aliases (for team-specific wiring; modify as necessary)
// -------------------------------------------------------------------

// PWM Pin Aliases
#define PWM_STEERING PWM_CH1
#define PWM_THROTTLE PWM_CH2

// RC Pin Aliases
// define as necessary, with Jack/Haoru's help

// ---------------------------------------------------------
// Macros (TODO Organize)
// ---------------------------------------------------------
#define HTTP_PORT 80
#define TIMEOUT 500
#define WDT_TIMEOUT 3
#define QUEUE_SIZE 10
#define JSON_SIZE 13
#define BUFFER_SIZE 310
#define STEERING_CHN 3
#define EMO_PIN 15

// ---------------------------------------------------------
// Calibration Parameters (TODO Store in Flash Memory)
// ---------------------------------------------------------
#define IDLE_THROT  1500
#define BRAKE_THROT 1000 // Full Reverse?
#define MID_THROT   1750
#define MAX_THROT   2000

// Currently calculated from mapping 0-255 values
// into 1000-2000 microsecond pulses
#define LEFT_STEER 1330
#define CENTER_STEER 1450
#define RIGHT_STEER 1580

// Original 0-255 values. Remove eventually
//#define LEFT_STEER 84
//#define CENTER_STEER 116
//#define RIGHT_STEER 148

// ---------------------------------------------------------
// Web Server Helper Structures
// ---------------------------------------------------------

// Button Component
struct Button {
    uint8_t pin;
    bool    on;
    void update() {digitalWrite(pin, on ? HIGH : LOW);}
};

// ---------------------------------------------------------
// Data Structs for IPC
// ---------------------------------------------------------

// Passed into qh_pwmCommand from Serial or WebServer;
// PWM Task receives PWM command and dispatches it
typedef struct {
  uint16_t  channel;      // PWM Channel [1-3]
  uint16_t  pulseWidth;   // Positive Pulse Width (millisec)
} pwm_cmd_t;

// ---------------------------------------------------------
// RTOS Task Functions and Handles
// ---------------------------------------------------------

extern TaskHandle_t th_server;
extern TaskHandle_t th_pwm;
extern TaskHandle_t th_serial;
extern TaskHandle_t th_json;

extern QueueHandle_t server2PWM_QueueHandle;
extern QueueHandle_t server2Status_QueueHandle;

extern QueueHandle_t qh_pwmCommand;;

void task_WebServer(void* param);
void task_PWM(void* param);
void task_SerialParser(void* param);
void task_JSONParser(void* param);


// ---------------------------------------------------------
// Global Function Prototypes
// ---------------------------------------------------------
void initWebServer();
void initPWM();

#endif
