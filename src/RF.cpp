#include <stdint.h>
#include <Arduino.h>
#include <RH_RF95.h>
#include "RF.h"

class RFM96W {
    private:
        float frequency;
        RH_RF95 RF = RH_RF95();

    public:
        RFM96W(float freq = 0.0) {
            frequency = freq;
        }

        void init() {
            if (!RF.init()) {
                Serial.println("[Debug]: RFM96W: Could not initialise.");
            }

            if (frequency != 0.0) {
                RF.setFrequency(frequency);
            }
            // TODO: add whatever else is needed for initialisation
        }

        void send(const char *data) {
            // TODO: send the mission data here
        }
};