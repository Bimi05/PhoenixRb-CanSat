#include <stdint.h>
#include <Arduino.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <Pixy2I2C.h>
#include <SPI.h>
#include <SD.h>

#include "detection.h"
#include "RF.h"

#define FREQUENCY 434.2F
#define PRESSURE 1013.25F //* modify value if and when needed

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS();
RFM96W RF = RFM96W(FREQUENCY);
Pixy2I2C Cam = Pixy2I2C();

File dataFile;
void setup() {
    Serial.begin(115200);
    BME680.begin();
    BNO055.begin();
    GPS.begin(9600);
    Cam.init();
    RF.init();

    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);
    Cam.changeProg("video");

    if (SD.begin()) {
        SD.remove("data.txt"); // if it doesn't exist, nothing wrong happens :D
        dataFile = SD.open("data.txt", FILE_WRITE);
    }
}

void loop() {
    float temperature = BME680.readTemperature();
    float pressure = BME680.readPressure();
    float humidity = BME680.readHumidity();

    //? find a way around gps
    GPS.read();
    if (GPS.newNMEAreceived()) {
        GPS.parse(GPS.lastNMEA());
    }

    // allocate memory worth 100 string characters for data
    char *data = (char*) malloc(100 * sizeof(char));
    if (!data) {
        // memory allocation failed
        Serial.println("[Debug]: Failed to allocate enough memory for the mission data.");
    }

    snprintf(data, 100 * sizeof(char), "%.02f %.02f %.02f %.04f %.04f", temperature, pressure, humidity, GPS.longitude, GPS.latitude);
    if (dataFile) {
        dataFile.println(data);
        dataFile.flush();
    }
    RF.send(data);
    free(data);

    uint8_t phase = detectPhase(BME680.readAltitude(PRESSURE));
    if (phase == 3) {
        uint8_t red, green, blue;
        if (Cam.video.getRGB(Cam.frameWidth / 2, Cam.frameHeight / 2, &red, &green, &blue) == 0) {
            Serial.print("Red: ");
            Serial.println(red);
            Serial.print("Green: ");
            Serial.println(green);
            Serial.print("Blue: ");
            Serial.println(blue);
        }
    }
    else if (phase == 4) {
        // play buzzer for recovery
    }
}
