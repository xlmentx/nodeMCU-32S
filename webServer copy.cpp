// Webserver Dependencies
#include <Arduino.h>
#include <LITTLEFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Webserver Constants
#define HTTP_PORT 80
#define JSON_SIZE 510
const char *WIFI_SSID = "ESP32Test";
const char *WIFI_PASS = "jetsonucsd";

// Webserver Instances
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");
StaticJsonDocument<JSON_SIZE> mcValues;
char messageBuffer[JSON_SIZE];

// WebSocket Event Handler
void webSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {     
    // New User Connection
    if(type == WS_EVT_CONNECT) {
        // Send Current Values To User
        mcValues["event"] = "load";
        client->text(messageBuffer, serializeJson(mcValues, messageBuffer));
    }
    // Incoming User Message
    else if(type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if(info->final && info->index==0 && info->len==len && info->opcode==WS_TEXT) {
            // Deseralize JSON Data
            StaticJsonDocument<JSON_SIZE> json;
            if(!DeserializationError(deserializeJson(json, data))) {
                // Handle JSON Event 
                if(json["event"] == "test") {
                    // calibrationTest();
                    json.clear(); 
                    json["event"]="test";
                }
                // Update Throttle 
                else if(json["event"] == "throttle") {
                    // updateThrottle();
                }
                // Erase Records 
                else if(json["event"] == "erase") {
                    // eraseRecords();
                }
                // Launch AI 
                else if(json["event"] == "ai") {
                    // LaunchAI();
                    json["ai"] = json["ai"]=="off"? "on": "off";
                } 
                // Update PID 
                else if(json["event"] == "pid") {
                    // updatePID();
                } 
                // Toggle EMO Button
                else if(json["event"] == "eStop") {
                    json["eStop"] = json["eStop"]=="off"? "on": "off";
                }
                
                // Update Current MC Values
                for(auto item : json.as<JsonObject>()) {
                    String key = item.key().c_str();
                    mcValues[key] = String(item.value().as<const char*>());
                }

                // Store Current MC Values
                File mcData = LITTLEFS.open("/mcData.json","w");
                serializeJson(mcValues, mcData);
                mcData.close();

                // Send Sync Message To Devices
                ws.textAll(messageBuffer, serializeJson(json, messageBuffer));   
            }
        }
    }
}

// Server Initialization
void initWebServer() {
    // Load Flash File System
    if(LITTLEFS.begin()) {
        // Collect MCU Values From Flash
        File mcData = LITTLEFS.open("/mcData.json","r");
        deserializeJson(mcValues, mcData);
        mcData.close();
        mcValues["ai"] = "off";
        mcValues["eStop"] = "off";

        // Initialize Web Socket Handlers
        ws.onEvent(webSocketEvent);
        server.addHandler(&ws);
        
        // Initialize Web Server Handlers
        server.serveStatic("/", LITTLEFS, "/");
        server.on("/", [](AsyncWebServerRequest *request){
            request->send(LITTLEFS, "/index.html", "text/html");
        });
        
        // Launch Soft Access Point and Server
        WiFi.softAP(WIFI_SSID, WIFI_PASS);
        server.begin();
    }
}

// Web Server Loop
void task_WebServer(void* param) {
    while (true)
    {   // Close Lingering WebSockets
        if(millis()%1000 == 0) { 
            ws.cleanupClients();
        }

        // Adjust/Remove Thread Delay 
        delay(400);
    }
}