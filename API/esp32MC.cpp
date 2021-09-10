#include <string>
#include <vector>
#include <iostream>

using namespace std;

class esp32MC {
    private:        
        // Create and Send JSON Message
        void sendJSON(vector<string> elements) {
            string json = "{\"event\"";
            for(int i=0; i<elements.size(); i++) {
                json +=(i%2)? ",": ":";
                json += "\""+elements[i]+"\"";    
            }
            json += "}";

            cout <<json<<endl;
        }

    public:
        esp32MC() {
            cout <<"Searching For ESP32 Port"<<endl;
        }
        // ---------------------------------------------------------
        // Micro Controller Setters
        // ---------------------------------------------------------

        void setEStop(bool state) {
            sendJSON(vector<string>{
                "eStop", state?"true":"false"
            });
        }

        void setSteeringRange(int left, int center, int right) {
            sendJSON(vector<string>{
            "save", "ltSteer", to_string(left), "ctrSteer", to_string(center), "rtSteer", to_string(right)
            });
        } 

        void setThrottleRange(int min, int mid, int max) {
            sendJSON(vector<string>{
                "save", "minThrot", to_string(min), "midThrot", to_string(mid), "maxThrot", to_string(max)
            });
        }

        void setHeartbeatFrequency(int frequency) {
            sendJSON(vector<string>{
                "save", "frequency", to_string(frequency)
            });
        }

        void testCalibration() {
            sendJSON(vector<string>{
                "test"
            });
        }

        void sendPWM(int steering, int throttle) {
            sendJSON(vector<string>{
                "pwm", "steering", to_string(steering), "throttle", to_string(throttle)
            });
        }

        // ---------------------------------------------------------
        // Web App Sync Functions
        // ---------------------------------------------------------

        void syncThrottleScalar(double scalar) {
            sendJSON(vector<string>{
                "scale", "tScalar", to_string(scalar)
            });
        }

        void syncPID(int kp, int ki, int kd) {
            sendJSON(vector<string>{
                "pid", "kp", to_string(kp), "ki", to_string(ki), "kd", to_string(kd)
            });
        }

        void syncEraseRecords(int nRecords) {
            sendJSON(vector<string>{
                "erase", "nRecords", to_string(nRecords)
            });
        }

        void syncAI(bool state) {
            sendJSON(vector<string>{
                "ai", "ai", to_string(state)
            });
        }
};

// API Testing
int main()
{   esp32MC mc;

    mc.setSteeringRange(10, 13, 20);
    mc.setEStop(true);
    mc.setSteeringRange(100, 110, 120);
    mc.setThrottleRange(22, 23, 24);
    mc.setHeartbeatFrequency(300);
    mc.testCalibration();
    mc.sendPWM(5, 10);
    mc.syncThrottleScalar(0.5);
    mc.syncPID(50, 20, 75);
    mc.syncEraseRecords(300);
    mc.syncAI(false);

    return 0;
}
