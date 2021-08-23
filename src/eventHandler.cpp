// Event Handler Dependancies
#include "defenitions.h"
#include <ArduinoJson.h>

// Event Handler Instances
StaticJsonDocument<JSON_SIZE> mcValues;

// Parse And Handle Incomming Events
void parseData(uint8_t *data) {
    Serial.print(xPortGetCoreID());
    Serial.println("]");
    
    // Deseralize JSON Data
    StaticJsonDocument<JSON_SIZE> jsonData;
    if(!DeserializationError(deserializeJson(jsonData, data))) {
        // Toggle Emergecy Stop Button
        if(jsonData["event"] == "eStop") {
            jsonData["eStop"] = jsonData["eStop"]=="true"? false: true;
            toggleEStop(jsonData["eStop"]);
        }
        // MC Configuration Testing 
        else if(jsonData["event"] == "test") {
            testCalibration(
                jsonData["ltSteer"],
                jsonData["ctrSteer"],
                jsonData["rtSteer"],
                jsonData["minThrot"],
                jsonData["midThrot"],
                jsonData["maxThrot"],
                jsonData["frequency"]
            );
            jsonData.clear(); 
            jsonData["event"]="test";
        } 
        // Launch Donkey AI 
        else if(jsonData["event"] == "ai") {
            jsonData["ai"] = jsonData["ai"]=="true"? false: true;
            toggleAI(jsonData["ai"]);
        } 
        // Erase N Donkey TUB Records 
        else if(jsonData["event"] == "erase") {
            eraseRecords(jsonData["nRecords"]);
        }
        // Update Throttle Scaling
        else if(jsonData["event"] == "throttle") {
            scaleThrottle(jsonData["tScalar"]);
        }
        // Update ROS PID 
        else if(jsonData["event"] == "pid") {
            updatePID(
                jsonData["kp"],
                jsonData["ki"],
                jsonData["kd"]
            );
        }
        
        // Send Sync Message To Devices
        char jsonBuffer[JSON_SIZE];
        serializeJson(jsonData, jsonBuffer);
        syncDevices(jsonBuffer);

        // Update Current MC Values
        for(auto item : jsonData.as<JsonObject>()) {
            String key = item.key().c_str();
            mcValues[key] = String(item.value().as<const char*>());
        }
        
        // Store Current MC Values
        File mcData = LITTLEFS.open("/mcData.json","w");
        serializeJson(mcValues, mcData);
        mcData.close();
    }
}

// Send Initial Values To New Users
void getInitialValues(char *jsonBuffer) {
    Serial.print(xPortGetCoreID());
    Serial.println("]");
    mcValues["event"] = "load";
    serializeJson(mcValues, jsonBuffer, JSON_SIZE);
}

void initEventHandler() {
    // Collect MCU Values From Flash
    File mcuData = LITTLEFS.open("/mcData.json","r");
    deserializeJson(mcValues, mcuData);
    mcuData.close();
    mcValues["ai"] = false;
    mcValues["eStop"] = false;
}