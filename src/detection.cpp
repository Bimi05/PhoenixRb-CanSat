#include <stdint.h>
#include "detection.h"

float lastMeasure = 0.0F;

uint8_t phase = 0U;
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
        return 0U; //! altitude is NaN
    }

    if (lastMeasure == 0.0F) {
        lastMeasure = altitude;
        return 0U;
    }

    float diff = altitude - lastMeasure;
    if (diff >= 0.0F && diff < 3.0F) {
        phase = (phase != 3U) ? 1U : 4U;
    }
    else {
        phase = (altitude > lastMeasure) ? 2U : 3U;
    }

    lastMeasure = altitude;
    return phase;
}
