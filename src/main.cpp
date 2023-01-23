#include <Arduino.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>
#include <Pixy2I2C.h>
#include <SPI.h>
#include <SD.h>

#include "detection.h"

#define FREQUENCY 433.2F //* frequency used for the RFM96W Sensor, change only if necessary
#define PRESSURE 1013.25F //* sea-level pressure, used to read altitude from the BME680 Sensor
#define DEBUG //* debug mode, used for printing out raw sensor data

//* RFM96W board pins (SPI communication)
#define RF_CS 0
#define RF_INT 0
#define RF_RST 0

//* Buzzer related (phase 4 // recovery)
#define BUZ_PIN 0
#define BUZ_FREQ 0

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS();
RH_RF95 RFM = RH_RF95(RF_CS, RF_INT);
Pixy2I2C Cam = Pixy2I2C();
File dataFile;

uint32_t timer = millis(); //* global timer
uint32_t packet = 1; //* packet ID (increments after every RFM transmission)
char *data = NULL; //* buffer that will hold our mission data

void setup(void) {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }

    bool BME_INIT = BME680.begin();
    bool BNO_INIT = BNO055.begin();
    bool GPS_INIT = GPS.begin(9600);
    bool SD_INIT = SD.begin();
    bool RFM_INIT = RFM.init();
    bool PIXY_INIT = Cam.init() == 0;

    data = (char*) malloc(100 * sizeof(char));
    dataFile = SD.open("data.txt", O_RDWR);
    dataFile.seek(0);

    #ifdef DEBUG
        if (!BME_INIT) {
            while (true) Serial.println("[Debug]: Could not initialise the BME680 Sensor.");
        }

        if (!BNO_INIT) {
            while (true) Serial.println("[Debug]: Could not initialise the BNO055 Sensor.");
        }

        if (!GPS_INIT) {
            while (true) Serial.println("[Debug]: Could not initialise the GPS Sensor.");
        }

        if (!SD_INIT) {
            while (true) Serial.println("[Debug]: Could not initialise the SD Card.");
        }

        if (!RFM_INIT) {
            while (true) Serial.println("[Debug]: Could not initialise the RFM96W Sensor.");
        }

        if (!PIXY_INIT) {
            while (true) Serial.println("[Debug]: Could not initialise Pixy2.");
        }

        if (!data) {
            while (true) Serial.println("[Debug]: Could not allocate memory for the mission data.");
        }

        if (!dataFile) {
            while (true) Serial.println("[Debug]: ");
        }
    #endif

    //* if we've made it here, everything has been successfully initialised

    pinMode(RF_RST, OUTPUT);
    digitalWrite(RF_RST, LOW);
    delay(100);
    digitalWrite(RF_RST, HIGH);
    delay(100);

    RFM.setFrequency(FREQUENCY);
    RFM.setModeTx();
    RFM.setTxPower(20);

    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
    Cam.changeProg("video");

    //* all settings configured!
}

void loop(void) {
    /*
    * --- BME680 ACCURACY --- *
    * Temperature: ± 1.0°C
    *  Pressure:   ± 1.0 hPa
    *  Humidity:   ± 3.0 %
    * ----------------------- *
    */

    float temperature = BME680.readTemperature();
    float pressure = BME680.readPressure();
    float humidity = BME680.readHumidity();
    float altitude = BME680.readAltitude(PRESSURE);

    GPS.read();
    #ifdef DEBUG
        if (temperature) {
            Serial.print("[Debug]: Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");
        }

        if (pressure) {
            Serial.print("[Debug]: Pressure: ");
            Serial.print(pressure);
            Serial.println(" hPa");
        }

        if (humidity) {
            Serial.print("[Debug]: Humidity: ");
            Serial.print(humidity);
            Serial.println(" %");
        }

        if (altitude) {
            Serial.print("[Debug]: Approx. Altitude: ");
            Serial.print(altitude);
            Serial.println("m");
        }
    #endif

    if (GPS.newNMEAreceived()) {
        #ifdef DEBUG
            Serial.print("[Debug]: ");
            Serial.println(GPS.lastNMEA());
        #endif
        GPS.parse(GPS.lastNMEA());
    }

    snprintf(data, 100 * sizeof(char), "%.02f %.02f %.02f %.02f %.04f %.04f", temperature, pressure, humidity, altitude, GPS.longitude, GPS.latitude);
    dataFile.println(data);
    dataFile.flush();

    //* send a packet every at least 100 milliseconds (10 Hz send rate)
    uint32_t now = millis();
    if (timer - now >= 100) {
        RFM.send((uint8_t*) data, strlen(data));
        RFM.waitPacketSent();
        packet++;
        timer = now;
    }

    uint8_t phase = detectPhase(altitude);
    if (phase == 3) {
        uint8_t red, green, blue;
        if (Cam.video.getRGB(Cam.frameWidth / 2, Cam.frameHeight / 2, &red, &green, &blue) == 0) {
            #ifdef DEBUG
                Serial.print("Red: ");
                Serial.println(red);
                Serial.print("Green: ");
                Serial.println(green);
                Serial.print("Blue: ");
                Serial.println(blue);
            #endif
        }
    }
    else if (phase == 4) {
        //* call multiple times at different frequencies to play a melody
        tone(BUZ_PIN, BUZ_FREQ, 100);
    }
}
