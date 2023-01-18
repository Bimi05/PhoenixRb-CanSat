#include <stdint.h>
#include "detection.h"

float lastMeasure = 0.0F;

uint8_t phase = 0;
uint8_t detectPhase(float altitude) {
    /*
    * ----- PHASES ----- *
    * Phase 1: before launch
    * Phase 2: going up (inside the rocket)
    * Phase 3: going down (free fall)
    * Phase 4: after landing
    * ------------------ *
    */

    if (altitude != altitude) {
        return 0; //! altitude is NaN
    }

    if (lastMeasure == 0.0F) {
        lastMeasure = altitude;
        return 0;
    }

    float diff = altitude - lastMeasure;
    if (diff >= 0.0F && diff < 3.0F) {
        phase = (phase != 3) ? 1 : 4;
    }
    else {
        phase = (altitude > lastMeasure) ? 2 : 3;
    }

    lastMeasure = altitude;
    return phase;
}
