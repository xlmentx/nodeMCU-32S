#include "defenitions.h"

static str_pool_ent_t* allocSerialBufferFromPool();

str_pool_ent_t serBufPool[QUEUE_LEN_SERIAL];

void jetsonServer(void* param) {

    str_pool_ent_t* bufferPoolEntry;
    char* payloadBuffer;

    uint32_t payloadPtr = 0;
    unsigned char c;

    //char initMsg[64] = "jetsonServer running on core ?";
    //initMsg[strlen(initMsg)-1] = (char) (xPortGetCoreID() + '0');
    log_i("jetsonServer running on core %d", xPortGetCoreID());

    while(true)
    {
        // First, "allocate" a serial parse buffer from a pool of
        // statically allocated string buffers. If none are available,
        // yield and try again on re-entry
        bufferPoolEntry = allocSerialBufferFromPool();
        if (!bufferPoolEntry) {
            log_d("Found zero available string buffers from static pool, yielding");
            portYIELD();
            continue;
        }

        payloadBuffer = bufferPoolEntry->buf;
        payloadPtr = 0;

        c = '\0';
        while (c != '\n') {

            if (!Serial.available()) {
                portYIELD();
            }
            else {
                c = (unsigned char) Serial.read();
                payloadBuffer[payloadPtr++] = c;

                if (payloadPtr >= SERIAL_BUFSIZE) {
                    log_e("Serial rx buffer overflow, expected <%d chars; JSON Parsing will be corrupted", SERIAL_BUFSIZE);
                    payloadPtr = 0;
                    continue;
                }
            }
        }

        payloadBuffer[payloadPtr-1] = '\0';
        log_d("Received serial message into parse buffer, sending to JSONParser");
        log_d("%s", payloadBuffer);

        if (xQueueSend(qh_jsonToParse, &bufferPoolEntry, 50) != pdTRUE) {
            log_w("JSONParser queue full, timed out; Dropping serial message");
            xSemaphoreGive(bufferPoolEntry->sem);
        }
    }
}

/*
 * Find an available Serial Buffer Pool Entry from our static 
 * memory pool, and return it to the user.
 *
 * return: Pointer to the first available buffer pool entry
 *         if any available. NULL if all are taken.
 *
 */
static str_pool_ent_t* allocSerialBufferFromPool() {
    for (unsigned int i = 0; i < QUEUE_LEN_SERIAL; i++) {
        if (xSemaphoreTake(serBufPool[i].sem, 1) == pdTRUE) {
            log_v("Found available string buffer (%d) from static pool", i);
            return &serBufPool[i];
        }
    }

    return NULL;
}

// ---------------------------------------------------------
// Event Functions
// ---------------------------------------------------------

void toggleAI(bool const &ai) {
    if(ai) {
        //xQueueSend(...);
    }
    else {
        //xQueueSend(...);
    }
}

void eraseRecords(int const &nRecords) {
    //xQueueSend(...);
}

void scaleThrottle(int const &tScalar) {
    //xQueueSend(...);
}

void updatePID( int const &kp,
                int const &ki,
                int const &kd) {
    //xQueueSend(...);
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
        String payloadBuffer;
        if(Serial.available()) {
            begin = millis();
            end = millis();
            esp_task_wdt_reset();
            payloadBuffer = Serial.readStringUntil('\n');
        }
  
        // Deserialize Message
        const uint8_t size = JSON_OBJECT_SIZE(1000);
        StaticJsonDocument<size> doc;
        DeserializationError error = deserializeJson(doc, payloadBuffer);
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