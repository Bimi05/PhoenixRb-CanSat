#include <stdint.h>
#include <Arduino.h>
#include <RH_RF95.h>

#include "RFM96W.h"

float frequency;
RH_RF95 RF = RH_RF95();

RFM96W::RFM96W(float freq = 0.0F) {
    frequency = freq;
}

void RFM96W::init(void) {
    if (!RF.init()) {
        Serial.println("[Debug]: Could not initialise the RFM96W Sensor.");
        while (true);
    }

    if (frequency != 0.0F) {
        RF.setFrequency(frequency);
    }
}

//encryption

//Sends the collected data to the ground station
void RFM96W::send(const char *data) {}
//send