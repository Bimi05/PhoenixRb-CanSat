#include "phases.h"

uint8_t phase = 0;
bool landed = false;

uint8_t detectPhase(Adafruit_BME680* BME, float pressure) {
    if (landed == true) {
        return 4;
    }

    float measurings[2];
    measurings[0] = BME->readAltitude(pressure);
    measurings[1] = BME->readAltitude(pressure);

    //* diff > 0 means cansat is going up
    //* diff < 0 means cansat is going down
    //! a maximum difference of max ± 1.0 for phases 1 & 4
    //! must be applied, for there are extremely subtle differences when stationary
    float diff = measurings[1] - measurings[0];

    if ((diff > -1.0F && diff < 1.0F) && phase == 3) {
        landed = true;
        return 4;
    }
    else if ((diff > -1.0F && diff < 1.0F)) {
        return 1;
    }

    phase = (diff > 0.0F) ? 2 : 3;
    return phase;
}
