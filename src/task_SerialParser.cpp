#include "defines.h"

void task_SerialParser(void* param) {

    String payload;

    char initMsg[64] = "task_SerialParser running on core ?";
    initMsg[strlen(initMsg)-1] = (char) (xPortGetCoreID() + '0');
    log_i("%s", initMsg);

    while(true)
    {
        while(!Serial.available()) {
            portYIELD();
        }

        // TODO does this block?? Find way to yield to other tasks
        payload = Serial.readStringUntil('\n');

        // TODO pass payload to JSONParser
    }
}

/* OLD task_PWM loop
void task_PWM(void* param) {
    Serial.print("task_PWM running on core ");
    Serial.println(xPortGetCoreID());

    bool killed;
    unsigned long begin = millis();
    unsigned long end = millis();
    bool buttonValue = false;

    while(true)
    {    
        //Serial.print("red button: "+Serial.available());
        //Serial.println(red_Button.on);
        
        // TODO REMOVE THE DELAY, want immediate response to commands
        delay(100);

        // Serial Check
        if (xQueueReceive(server2PWM_QueueHandle, &buttonValue, 0) == pdTRUE) {
            Serial.print("BUTTON == ");
            Serial.println(buttonValue);
            if (buttonValue) {
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
            payload = Serial.readStringUntil('\n');
        }
  
        // Deserialize Message
        const uint8_t size = JSON_OBJECT_SIZE(1000);
        StaticJsonDocument<size> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
          // TODO error handling
        } else {
            // Calculate Steering and Throttle
            float normalized_throttle = doc["throttle"];
            float normalized_steering = doc["steering"];
            int steering_pwm = !buttonValue ? CENTER_STEER+int(normalized_steering*int((RIGHT_STEER-LEFT_STEER)/2)): CENTER_STEER;
            int throttle_pwm = !buttonValue ? IDLE_THROT+int(normalized_throttle*500): IDLE_THROT;
            pwmSteering.writeMicroseconds(steering_pwm);
            pwmThrottle.writeMicroseconds(throttle_pwm);
            Serial.print("steering_pwm=");
            Serial.print(steering_pwm);
            Serial.print(" throttle_pwm=");
            Serial.println(throttle_pwm);
        };
    }
}
*/
