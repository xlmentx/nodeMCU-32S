// Dependancies
#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <math.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <ESP32Servo.h>

#include "defines.h"

// ---------------------------------------------------------
// RTOS Structures
// ---------------------------------------------------------
TaskHandle_t th_server;
TaskHandle_t th_pwm;

QueueHandle_t server2PWM_QueueHandle;
QueueHandle_t server2Status_QueueHandle;
QueueHandle_t qh_pwmCommand;;

// ---------------------------------------------------------
// Useful Functions
// ---------------------------------------------------------

static void initIO();
static void initRC();
static void initBLDC();
static void initI2C();
static void initSPI();
static void initWDT();

// Set Up
void setup()
{
    // Create Server to PWM Queue
    Serial.begin(115200);
    Serial.print("setup running on core ");
    Serial.println(xPortGetCoreID());

    // Old queues, deprecate
    server2PWM_QueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(bool));
    server2Status_QueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(int));

    qh_pwmCommand = xQueueCreate(6, sizeof(pwm_cmd_t));

    // Severe error, halt execution
    if (!server2PWM_QueueHandle) {
        // TODO better error handling
        Serial.println("QUEUE NOT SET, ENTERING SPINLOCK...");
        while(1);
    }

    initIO();
    initWebServer();

    xTaskCreatePinnedToCore(task_WebServer,    "Task_WebServer",    10240, NULL, 2, &th_server, 0);
    xTaskCreatePinnedToCore(task_PWM,          "Task_PWM",           2048, NULL, 5, &th_pwm,    1);
    xTaskCreatePinnedToCore(task_SerialParser, "Task_SerialParser", 40968, NULL, 3, &th_pwm,    1);
}

// Main Loop
void loop() {}

// ---------------------------------------------------------
// Init Functions
// ---------------------------------------------------------

static void initIO() {

  // LEDs
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(RB_R,  OUTPUT);
  pinMode(RB_B,  OUTPUT);

  // PWM Pins
  initPWM();

  // RC Pins
  initRC();

  // BLDC Sensor
  initBLDC();

  // Communication Busses
  initI2C();  // Generic I2C lines + OE pin
  initSPI();  // Generic SPI
}

static void initRC() {
  // TODO
}

static void initBLDC() {
  pinMode(BLDC1, INPUT);
  pinMode(BLDC2, INPUT);
  pinMode(BLDC3, INPUT);

  // Change to 0 to disable at compile-time (for debug)
  #if 0
  attachInterrupt(digitalPinToInterrupt(BLDC1), ISR_BLDC1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BLDC2), ISR_BLDC2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BLDC3), ISR_BLDC3, FALLING);
  #endif
}


// 2x Generic I2C Busses, plus OE pin
static void initI2C() {
  pinMode(PWM_OE, INPUT);

  // TODO
}

// Generic SPI Bus
static void initSPI() {
  // TODO
}

// Watchdog Timer
static void initWDT() {
    esp_task_wdt_add(NULL);

    // TODO set WDT reset period

    //enable WDT panic so ESP32 restarts
    esp_task_wdt_init(WDT_TIMEOUT, true);
}

// ---------------------------------------------------------
// Interrupt Service Routines (ISR) for BLDC Sensor
// ---------------------------------------------------------

void IRAM_ATTR ISR_BLDC1() {
}

void IRAM_ATTR ISR_BLDC2() {
}

void IRAM_ATTR ISR_BLDC3() {
}

