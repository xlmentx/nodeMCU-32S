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
// WiFi Credentials
// ---------------------------------------------------------
const char *WIFI_SSID = "ESP32Test";
const char *WIFI_PASS = "jetsonucsd";

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

    xTaskCreatePinnedToCore(task_WebServer,  "Task_WebServer",  10240, NULL, 2, &th_server, 0);
    xTaskCreatePinnedToCore(task_PWM,        "Task_PWM",        2048,  NULL, 5, &th_pwm,    1);
}

// Main Loop
void loop() {}

// ---------------------------------------------------------
// Init Functions
// ---------------------------------------------------------

void initIOPins() {

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

void initRC() {
  // TODO
}

void initBLDC() {
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
void initI2C() {
  pinMode(PWM_OE, INPUT);

  // TODO
}

// Generic SPI Bus
void initSPI() {
  // TODO
}

// Watchdog Timer
void initWDT() {
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

