#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>
#include <Pixy2.h>
#include <SD.h>

#include "detection.h"
#include "RF.h"

#define FREQUENCY 434.2
#define PRESSURE 1013.25 // to change if needed

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS();
RH_RF95 RF = RH_RF95();
Pixy2 Cam = Pixy2();

File dataFile;
void setup() {
    Serial.begin(115200);

    Cam.init();
    Cam.changeProg("video");

    BME680.begin();
    BNO055.begin();

    RF.init();
    RF.setFrequency(FREQUENCY);

    if (SD.begin()) {
        SD.remove("data.txt"); // if it doesn't exist, nothing wrong happens :D
        dataFile = SD.open("data.txt", FILE_WRITE);
    }
}



void loop() {
    uint8_t phase = detectPhase(BME680.readAltitude(PRESSURE));

    float temperature = BME680.readTemperature();
    float pressure = BME680.readPressure();
    float humidity = BME680.readHumidity();

    //? find a way around gps
    char position = GPS.read();

    // TODO: add gps-related data
    if (dataFile) {
        // allocate memory worth 100 string characters for data
        char* data = (char*) malloc(100 * sizeof(char));
        if (data != NULL) {
            memset(data, 0, 100 * sizeof(char));
            snprintf(data, 100 * sizeof(char), "%.02f %.02f %.02f", temperature, pressure, humidity);
            dataFile.println(data);
            dataFile.flush();
            RFsend(&RF);
            free(data);
        }

        // if allocation failed, data is NULL
        Serial.println("[Debug]: Failed to allocate enough memory for the mission data.");
    }
}
