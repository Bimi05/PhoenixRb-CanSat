#include <Arduino.h>
#include <stdint.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <Pixy2.h>
#include "detection.h"

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS();
Pixy2 Cam = Pixy2();

void setup() {
    Serial.begin(115200);

    Cam.init();
    Cam.changeProg("video");

    bool BME_BOOT = BME680.begin();
    bool BNO_BOOT = BNO055.begin();

    const char* BME_INIT = "[Debug] BME680: Initialised!";
    const char* BNO_INIT = "[Debug] BNO055: Initialised!";

    if (!BME_BOOT) {
        BME_INIT = "[Debug] BME680: Failed to initialise.";
    }

    if (!BNO_BOOT) {
        BNO_INIT = "[Debug] BNO055: Failed to initialise.";
    }

    Serial.println(BME_INIT);
    Serial.println(BNO_INIT);
}

void loop() {
    uint8_t phase = detectPhase(BME680);

    float temperature = BME680.readTemperature();
    float pressure = BME680.readPressure();
    char position = GPS.read();

}
