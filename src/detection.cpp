#include <stdint.h>
#include "detection.h"

float lastMeasure = 0.0F;

uint8_t phase = 0;
uint8_t detectPhase(float altitude) {
    if (altitude != altitude) {
        //* this returns true if altitude is NaN
        return 0;
    }

    if (lastMeasure == 0.0) {
        lastMeasure = altitude;
        return 0;
    }

    // TODO: improve and perfect after testing
    float diff = altitude - lastMeasure;
    if (diff >= 0.0F && diff < 3.0F) {
        // altitude hasn't changed much/at all, so we're on the ground
        // phase 1 is before launch
        // phase 4 is after landing
        phase = (phase != 3) ? 1 : 4;
    }
    else {
        // if altitude increases (current > last) then we're in the rocket going up (phase 2)
        // if altitude decreases (current < last) then we're falling from where the rocket launched us (phase 3)
        phase = (altitude > lastMeasure) ? 2 : 3;
    }

    lastMeasure = altitude;
    return phase;
}
