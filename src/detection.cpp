#include <stdint.h>
#include <Adafruit_BME680.h>
#include "detection.h"

float lastValue = NULL;
float currentValue = NULL;

uint8_t lastRecordedPhase = 0;
uint8_t phase = 0;

uint8_t detectPhase(Adafruit_BME680 BME680) {
    if (lastValue == NULL) {
        lastValue = BME680.readAltitude(SENSORS_PRESSURE_SEALEVELHPA);
        return 0;
    }

    currentValue = BME680.readAltitude(SENSORS_PRESSURE_SEALEVELHPA);
    if (0.0 <= abs(currentValue - lastValue) < 5.0) {
        phase = (lastRecordedPhase == 0) ? 1 : 4;
    }

    if (currentValue > lastValue) {
        phase = 2;
    }
    else {
        phase = 3;
    }

    lastRecordedPhase = phase;
    lastValue = currentValue;

    return phase;
}
