/*
 * TODO more web server description
 *
 */

#include "defines.h"

#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

// TODO ArduinoJson won't be neccesary, defer JSON parsing to
// task_JSONParser
#include <ArduinoJson.h>

// ---------------------------------------------------------
// WiFi Credentials
// ---------------------------------------------------------
const char *WIFI_SSID = "ESP32Test";
const char *WIFI_PASS = "jetsonucsd";

// ---------------------------------------------------------
// Web Server Structures
// ---------------------------------------------------------
bool eStop_button = false;
bool ai_button = false;

AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

void onWebServerEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void onRootRequest(AsyncWebServerRequest *request);

// ---------------------------------------------------------
// RTOS Tasks
// ---------------------------------------------------------

// Server Loop
void task_WebServer(void* param) {
    char initMsg[64] = "task_WebServer running on core ?";
    initMsg[strlen(initMsg)-1] = (char) (xPortGetCoreID() + '0');
    log_i("%s", initMsg);
    
    while (true)
    {
    
        // Close Lingering WebSockets
        if(millis()%1000 == 0){ ws.cleanupClients();}
        delay(400);
    }
}

// ---------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------

// Server Initialization
void initWebServer() {
    // Activate Serial Monitor 
    // TODO this delay may or may not be necessary
    //delay(500);
  
    // Start SPIFFS
    SPIFFS.begin();
    
    // Create Soft Access Point
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    
    // Start Web Socket
    ws.onEvent(onWebServerEvent);
    server.addHandler(&ws);
    
    // Launch Web Server
    server.on("/", onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
    log_v("Server Ready to go");
}

// FiLL In Buttons For New Users Using Current Values
String processor(const String &var) {
    String buttons = "";
    if(var == "E-STOP"){
        String eStop_state = eStop_button ? "on" : "off";
        buttons += "<button id='eStop' class="+eStop_state+"></button>";
    }
    return buttons;
}

// Locate HTML Page
void onRootRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

// WebSocket Events
void onWebServerEvent(AsyncWebSocket *server,
             AsyncWebSocketClient    *client,
             AwsEventType             type,  
             void                    *arg,   
             uint8_t                 *data,  
             size_t                   len) { 

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
                ai_button = !ai_button;
                json["ai"] = ai_button ? "on": "off";
                json["event"] = "ai_"+json["ai"].as<String>();                
            } 
            // Update PID 
            else if(json["event"]=="pid") {
                // Update PID
                // Save Values               
            } 
            // Update EMO Button
            else if(json["event"] == "eStop") {
                eStop_button = !eStop_button;
                json["eStop"] = eStop_button ? "on": "off";
                json["event"] = "eStop_"+json["eStop"].as<String>();
            }
            
            // Sync Devices
            char buffer[BUFFER_SIZE];
            ws.textAll(buffer, serializeJson(json, buffer));
        }
    }
}

