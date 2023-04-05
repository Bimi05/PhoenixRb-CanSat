#include "phases.h"

uint8_t phase = 1;
bool landed = false;
bool ascending = false;
float measurings[5] = { 0.0F };

uint8_t detectPhase(Adafruit_BME680* BME, float pressure) {
    if (landed == true) {
        return 4;
    }

    for (int i=0; i<5; i++) {
        measurings[i] = BME->readAltitude(pressure);
    }

    uint8_t pos = 0;
    while (pos < 5) {
        float diff = measurings[pos] - measurings[pos+1];
        if (pos > 0) {
            float diff2 = measurings[pos] - measurings[pos-1];
        }

        if (diff < 0.0F) {
            phase = (ascending) ? 2 : 3;
        }

        pos++;
    }

    return phase;
}
