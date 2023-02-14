#include "phases.h"

uint8_t phase = 1;
bool landed = false;
float measurings[5] = { 0.0F };

uint8_t detectPhase(Adafruit_BME680* BME, float pressure) {
    if (landed == true) {
        return 4;
    }

    for (int i=0; i<5; i++) {
        measurings[i] = BME->readAltitude(pressure);
    }

    //* check measures
    //! ascending order => phase 2
    //! descending order => phase 3
    //! ascending then descending or just descending => phase 3
    //! descending then stabilising => phase 4 --- set landed to true

    return phase;
}
