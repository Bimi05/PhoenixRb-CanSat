#include <Arduino.h>
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
    bool BME_BOOT = BME680.begin();
    bool BNO_BOOT = BNO055.begin();

    const char* BME_INIT = "[Debug] BME680: Failed to initialise";
    const char* BNO_INIT = "[Debug] BNO055: Failed to initialise";

    if (BME_BOOT) {
        BME_INIT = "..."; // successfully initiated BME680
    }

    if (BNO_BOOT) {
        BNO_INIT = "..."; // successfully initialised BNO055
    }



    Cam.init();
    Cam.changeProg("video");
}

void loop() {
    // run repeatedly
}
