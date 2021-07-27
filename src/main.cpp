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
// TODO Recalibrate if necessary
#define LEFT_STEER 1330
#define CENTER_STEER 1450
#define RIGHT_STEER 1580

//#define LEFT_STEER 84
//#define CENTER_STEER 116
//#define RIGHT_STEER 148

// ---------------------------------------------------------
// WiFi Credentials
// ---------------------------------------------------------
const char *WIFI_SSID = "ESP32Test";
const char *WIFI_PASS = "jetsonucsd";

// Button Component
struct Button {
    uint8_t pin;
    bool    on;
    void update() {digitalWrite(pin, on ? HIGH : LOW);}
};

// Instances (Hardware items)
// TODO migrate steering servo from ledc to Servo
Servo pwmThrottle;
Servo pwmSteering;

// Instances (software structures)
Button eStop_button = {EMO_PIN, false };
Button ai_button = {NULL, false };
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");
TaskHandle_t server_Handle;
TaskHandle_t pwm_Handle;
QueueHandle_t server2PWM_QueueHandle;
QueueHandle_t server2Status_QueueHandle;

// FiLL In Buttons For New Users Using Current Values
String processor(const String &var) {
    String buttons = "";
    if(var == "E-STOP"){
        String eStop_state = eStop_button.on?"on":"off";
        buttons += "<button id='eStop' class="+eStop_state+"></button>";
    }
    return buttons;
}

// Locate HTML Page
void onRootRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

// WebSocket Events
void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,  
             void                 *arg,   
             uint8_t              *data,  
             size_t                len) { 

    // Incomings Message
    if(type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if(info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            // Deseralize Message
            const size_t size = JSON_OBJECT_SIZE(JSON_SIZE);
            StaticJsonDocument<size> json;
            DeserializationError err = deserializeJson(json, data);
            
            // Catch Deserialization Error
            if(err) { 
                return; 
            }
            // Test Calibration 
            else if(json["event"]=="test") {
                // Test Values
                json.clear();
                json["event"]="test";
            }
            // Save Calibration 
            else if(json["event"]=="save") {
                // Save Values
            }
            // Update Throttle 
            else if(json["event"]=="throttle") {
                // Save Value
            }
            // Erase Records 
            else if(json["event"]=="erase") {
                // Erase Records
                // Save Value
            }
            // Launch AI 
            else if(json["event"]=="ai") {
                ai_button.on = !ai_button.on;
                json["ai"] = ai_button.on? "on": "off";
                json["event"] = "ai_"+json["ai"].as<String>();                
            } 
            // Update PID 
            else if(json["event"]=="pid") {
                // Update PID
                // Save Values               
            } 
            // Update EMO Button
            else if(json["event"] == "eStop") {
                eStop_button.on = !eStop_button.on;
                json["eStop"] = eStop_button.on? "on": "off";
                json["event"] = "eStop_"+json["eStop"].as<String>();
            }
            
            // Sync Devices
            char buffer[BUFFER_SIZE];
            ws.textAll(buffer, serializeJson(json, buffer));
        }
    }
}

// Server Initialization
void serverSetup() {
    // Define Pin Behavior
    pinMode(eStop_button.pin, OUTPUT);
    
    // Activate Serial Monitor 
    delay(500);
  
    // Start SPIFFS
    SPIFFS.begin();
    
    // Create Soft Access Point
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    
    // Start Web Socket
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    
    // Launch Web Server
    server.on("/", onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
    Serial.println("\nServer Ready to go");
}

// Server Loop
void serverTasks(void* param) {
    Serial.print("serverTasks running on core ");
    Serial.println(xPortGetCoreID());
    
    while (true)
    {   // Update E-Stop Button
        eStop_button.update();
    
        // Close Lingering WebSockets
        if(millis()%1000 == 0){ ws.cleanupClients();}
        delay(400);
    }
}

// PWM Loop
void pwmTasks(void* param) {
    bool killed;
    Serial.print("pwmTasks running on core ");
    Serial.println(xPortGetCoreID());
    unsigned long begin = millis();
    unsigned long end = millis();
    bool i = false;
    while(true)
    {    
        Serial.print("red button: "+Serial.available());
        //Serial.println(red_Button.on);
        delay(100);
        // Serial Check
    
        if (!server2PWM_QueueHandle) {
            Serial.println("QUEUE NOT SET");
        }
        if (xQueueReceive(server2PWM_QueueHandle, &i, 0) == pdTRUE) {
            Serial.print("BUTTON == ");
            Serial.println(i);
            if (i) {
                killed = true;
            } else {
                killed = false;
            }
            Serial.print("killed == ");
            Serial.println(killed);
        }
        
        // Heartbeat Check
        while(!Serial.available()) {
            end = millis();

            // Enter Backup Routine
            if(end-begin >= 200) {
                Serial.println("Enterning backup routine");
                while(1) {
                    pwmThrottle.writeMicroseconds(IDLE_THROT);
                    pwmSteering.writeMicroseconds(CENTER_STEER);
                    delay(100);
                }
            }
        }
        
        // Get Message
        String payload;
        if(Serial.available()) {
            begin = millis();
            end = millis();
            esp_task_wdt_reset();
            payload = Serial.readStringUntil( '\n' );
        }
  
        // Deserialize Message
        const uint8_t size = JSON_OBJECT_SIZE(1000);
        StaticJsonDocument<size> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {    
        } else {
            // Calculate Steering and Throttle
            float normalized_throttle = doc["throttle"];
            float normalized_steering = doc["steering"];
            int steering_pwm = !i? CENTER_STEER+int(normalized_steering*int((RIGHT_STEER-LEFT_STEER)/2)): CENTER_STEER;
            int throttle_pwm = !i? IDLE_THROT+int(normalized_throttle*500): IDLE_THROT;
            pwmSteering.writeMicroseconds(steering_pwm);
            pwmThrottle.writeMicroseconds(throttle_pwm);
            Serial.print("steering_pwm=");
            Serial.print(steering_pwm);
            Serial.print(" throttle_pwm=");
            Serial.println(throttle_pwm);
        };
    }
}

// Set Up
void setup()
{   // Start Server
    
    // Start PWM
    
    // Create Server to PWM Queue
    Serial.begin(115200);
    Serial.print("setup running on core ");
    Serial.println(xPortGetCoreID());
    server2PWM_QueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(bool));
    server2Status_QueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(int));
    if (!server2PWM_QueueHandle) {
        Serial.println("QUEUE NOT SET");
    }

    // TODO initIO needs testing
    initIO();

    serverSetup();
    xTaskCreatePinnedToCore(serverTasks, "ServerTasks", 10000, NULL, 2, &server_Handle, 0);
    delay(500); 
    //initPWM();
    //xTaskCreatePinnedToCore(pwmTasks, "pwmTasks", 10000, NULL, 1, &pwm_Handle, 1);
    //delay(500);
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

void initPWM() {
    pinMode(PWM_THROTTLE, OUTPUT);
    pwmThrottle.attach(PWM_THROTTLE);
    pwmThrottle.writeMicroseconds(IDLE_THROT);

    // Initialize steering
    pinMode(PWM_STEERING, OUTPUT);
    pwmSteering.attach(PWM_STEERING);
    pwmSteering.writeMicroseconds(CENTER_STEER);

    // Delay to calibrate ESC
    delay(7000);
    Serial.println("PWM Ready to go");
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

