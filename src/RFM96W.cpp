#include <stdint.h>
#include <Arduino.h>
#include <RH_RF95.h>

#include "RFM96W.h"

float frequency;
RH_RF95 RFM = RH_RF95();

RFM96W::RFM96W(float freq = 0.0F) {
    frequency = freq;
}

void RFM96W::init() {
    if (!RFM.init()) {
        Serial.println("[Debug]: RFM96W: Could not initialise.");
    }

    if (frequency != 0.0F) {
        RFM.setFrequency(frequency);
    }

    // TODO: add whatever else is needed for initialisation
}

void RFM96W::send(const char *data) {
    RFM.send("Hello World!", 13);
}
