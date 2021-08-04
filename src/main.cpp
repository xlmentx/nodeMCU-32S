#include "defines.h"

#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

// ---------------------------------------------------------
// RTOS Structures
// ---------------------------------------------------------

#define STACK_SIZE_SERVER   (1024*10)   // 10 KWords
#define STACK_SIZE_PWM      (1024*2)    //  2 KWords
#define STACK_SIZE_SERIAL   (1024*4)    //  4 KWords
#define STACK_SIZE_JSON     (1024*4)    //  4 KWords

// Prefer static tasks to determine memory usage at link-time instead of run-time
#define USE_STATIC_TASKS

#ifdef USE_STATIC_TASKS

// Static Stack Buffers
StackType_t sb_WebServer     [STACK_SIZE_SERVER];
StackType_t sb_PWM           [STACK_SIZE_PWM];
StackType_t sb_SerialParser  [STACK_SIZE_SERIAL];
StackType_t sb_JSONParser    [STACK_SIZE_JSON];

// Static Task Buffers
StaticTask_t tb_WebServer;
StaticTask_t tb_PWM;
StaticTask_t tb_SerialParser;
StaticTask_t tb_JSONParser;
#endif

TaskHandle_t th_server;
TaskHandle_t th_pwm;
TaskHandle_t th_serial;
TaskHandle_t th_json;

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

    #ifdef USE_STATIC_TASKS
        th_server = xTaskCreateStaticPinnedToCore(task_WebServer,    "Task_WebServer",    STACK_SIZE_SERVER,  NULL, 2, sb_WebServer,    &tb_WebServer,    0);
        th_pwm    = xTaskCreateStaticPinnedToCore(task_PWM,          "Task_PWM",          STACK_SIZE_PWM,     NULL, 5, sb_PWM,          &tb_PWM,          1);
        th_serial = xTaskCreateStaticPinnedToCore(task_SerialParser, "Task_SerialParser", STACK_SIZE_SERIAL,  NULL, 3, sb_SerialParser, &tb_SerialParser, 1);
        th_json   = xTaskCreateStaticPinnedToCore(task_JSONParser,   "Task_JSONParser",   STACK_SIZE_JSON,    NULL, 3, sb_JSONParser,   &tb_JSONParser,   1);
    #else
        xTaskCreatePinnedToCore(task_WebServer,    "Task_WebServer",    STACK_SIZE_SERVER,  NULL, 2, &th_server, 0);
        xTaskCreatePinnedToCore(task_PWM,          "Task_PWM",          STACK_SIZE_PWM,     NULL, 5, &th_pwm,    1);
        xTaskCreatePinnedToCore(task_SerialParser, "Task_SerialParser", STACK_SIZE_SERIAL,  NULL, 3, &th_serial, 1);
        xTaskCreatePinnedToCore(task_JSONParser,   "Task_JSONParser",   STACK_SIZE_JSON,    NULL, 3, &th_json,   1);
    #endif
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

