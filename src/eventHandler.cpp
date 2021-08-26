// Event Handler Dependancies
#include "defenitions.h"
#include <ArduinoJson.h>

// Event Handler Instances
StaticJsonDocument<JSON_SIZE> mcValues;

// Parse And Handle Incomming Events
void parseData(uint8_t *data) {    
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
            jsonData.clear(); 
            jsonData["event"]="test";
        } 
        // Launch Donkey AI 
        else if(jsonData["event"] == "ai") {
            jsonData["ai"] = jsonData["ai"]=="true"? false: true;
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