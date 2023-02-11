#include <Arduino.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>
#include <Pixy2I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>

#include "poi.h"
#include "phases.h"

#define SEALEVEL_PRESSURE 1013.25F

#define RFM_CS 0
#define RFM_INT 0
#define RFM_RST 0
#define RFM_FREQUENCY 434.2F

#define BUZZER 0

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS();
RH_RF95 RFM = RH_RF95(RFM_CS, RFM_INT);
Pixy2I2C Cam = Pixy2I2C();
Servo Servo1 = Servo();
Servo Servo2 = Servo();
File dataFile;

char *data = (char*) malloc(100 * sizeof(char));
void setup(void) {
    while (!Serial) {
        Serial.begin(9600);
    }
    Serial.println("[Debug] Beginning initialisation...");

    if (!BME680.begin()) {
        Serial.println("[Debug] Could not initialise the BME680 Sensor.");
    }
    else {
        Serial.println("[Debug] Sensor BME680 initialised!");
    }

    if (!BNO055.begin()) {
        Serial.println("[Debug] Could not initialise the BNO055 Sensor.");
    }
    else {
        Serial.println("[Debug] Sensor BNO055 initialised!");
    }

    if (!GPS.begin(9600)) {
        Serial.println("[Debug] Could not initialise the Ultimate GPS Sensor.");
    }
    else {
        GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
        Serial.println("[Debug] Sensor Ultimate GPS initialised!");
    }

    if (!SD.begin(0)) {
        Serial.println("[Debug] Could not initialise the SD Card.");
    }
    else {
        dataFile = SD.open("data.txt", O_RDWR);
        if (dataFile) {
            dataFile.seek(0);
            Serial.println("[Debug] SD card initialised!");
        }
    }

    if (!RFM.init()) {
        Serial.println("[Debug] Could not initialise the RFM9x LoRa Sensor.");
    }
    else {
        pinMode(RFM_RST, OUTPUT);
        digitalWrite(RFM_RST, LOW);
        delay(10);
        digitalWrite(RFM_RST, HIGH);
        delay(10);

        RFM.setFrequency(RFM_FREQUENCY);
        RFM.setModeTx();
        RFM.setTxPower(20);
        Serial.println("[Debug] Sensor RFM9x LoRa initialised!");
    }

    if (Cam.init() != 0) {
        Serial.println("[Debug] Could not initialise Pixy2.");
    }
    else {
        Cam.changeProg("video");
        Serial.println("[Debug] Camera Pixy2 initialised!");
    }

    Serial.println("[Debug] Initialisation complete!");
    Serial.println("----------------------------------------------------------------------");
}

#define DEBUG_MODE

uint32_t timer = millis();
void loop(void) {
    float temperature = BME680.readTemperature();
    float pressure = BME680.readPressure();
    float humidity = BME680.readHumidity();
    float altitude = BME680.readAltitude(SEALEVEL_PRESSURE);

    #ifdef DEBUG_MODE
        if ((millis() - timer) >= 2000) {
            Serial.print("[Debug] Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");

            Serial.print("[Debug] Pressure: ");
            Serial.print(pressure);
            Serial.println(" hPa");

            Serial.print("[Debug] Humidity: ");
            Serial.print(humidity);
            Serial.println("%");

            Serial.print("[Debug] Approx. Altitude: ");
            Serial.print(altitude);
            Serial.println("m");

            Serial.println();
        }
    #endif

    GPS.read();
    if (GPS.newNMEAreceived()) {
        #ifdef DEBUG_MODE
            Serial.print("[Debug] Latest NMEA sentence: ");
            Serial.println(GPS.lastNMEA());
        #endif
        GPS.parse(GPS.lastNMEA());
    }

    // TODO: add encryption prefix/suffix, ID & time for data
    uint8_t len = snprintf(data, 100 * sizeof(char), "%.02f %.02f %.02f %.02f %.04f %.04f", temperature, pressure, humidity, altitude, GPS.longitude, GPS.latitude);

    dataFile.println(data);
    dataFile.flush();

    uint32_t now = millis();
    if ((now - timer) >= 500) {
        if (RFM.send((uint8_t*) data, len)) {
            Serial.println("[Debug] Packet sent successfully!");
        }
        else {
            Serial.println("[Debug] Packet could not be sent.");
        }

        RFM.waitPacketSent();
        timer = now;
    }

    uint8_t phase = detectPhase(BME680, SEALEVEL_PRESSURE);
    if (phase == 3) {
        char* pos = findPOI(Cam);
        moveToPOI(BNO055, Servo1, Servo2, pos);
    }
    else if (phase == 4) {
        tone(BUZZER, 0, 100);
    }
}
