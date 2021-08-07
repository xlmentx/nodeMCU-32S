/*
 *
 */

#include "defines.h"
#include <ESP32Servo.h>

// ---------------------------------------------------------
// Library Hardware Object Instances
// ---------------------------------------------------------

// Only this file should access the PWM channels
// PWM Channel Numbers correspond to their PCB Markings
Servo pwm1;
Servo pwm2;
Servo pwm3;

// Note that pwmChannels[0] is PWM channel 1 etc
// This is handled correctly inside task_PWM() task loop
Servo* pwmChannels[3] = {
  &pwm1,
  &pwm2,
  &pwm3
};

// ---------------------------------------------------------
// RTOS Tasks
// ---------------------------------------------------------

void task_PWM(void* param) {
    pwm_cmd_t pwmCommand;

    char logBuf[128] = "task_PWM running on core ?";
    logBuf[strlen(logBuf)-1] = (char) (xPortGetCoreID() + '0');
    log_i("%s", logBuf);

    while(true)
    {    
        // Wait indefinitely until PWM Command arrives
        if (xQueueReceive(qh_pwmCommand, &pwmCommand, portMAX_DELAY) == pdFALSE) {
            // something bad happened, report error somehow
            continue;
        }
        
        if (pwmCommand.channel < 1 || pwmCommand.channel > 3) {
            sprintf(logBuf, "Invalid PWM Channel (%d)", pwmCommand.channel);
            log_e("%s", logBuf);
            continue;
        }
        if (pwmCommand.pulseWidth < 1000 || pwmCommand.pulseWidth > 2000) {
            sprintf(logBuf, "Invalid PWM Pulse Width (%d)", pwmCommand.pulseWidth);
            log_e("%s", logBuf);
            continue;
        }

        // pwmCommand.channel ranges in [1-3] to match PCB markings
        // Convert to 0-index
        pwmChannels[pwmCommand.channel-1]->writeMicroseconds(pwmCommand.pulseWidth);
    }
}

// ---------------------------------------------------------
// Init Functions
// ---------------------------------------------------------

void initPWM() {
    pinMode(PWM_THROTTLE, OUTPUT);
    pwm1.attach(PWM_THROTTLE);
    pwm1.writeMicroseconds(IDLE_THROT);

    pinMode(PWM_STEERING, OUTPUT);
    pwm2.attach(PWM_STEERING);
    pwm2.writeMicroseconds(CENTER_STEER);

    // Channel 3 currently reserved
    pinMode(PWM_CH3, OUTPUT);
    pwm3.attach(PWM_CH3);
    pwm3.writeMicroseconds(1500);

    // Delay to calibrate ESC
    // TODO remove this, want minimal delays (consider when watchdog
    // trips and entire system reboots; want quick boot to resume functionality)
    //delay(7000);
    log_v("PWM Ready to go");
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
