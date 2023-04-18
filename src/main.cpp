#include <cstring>
#include <Arduino.h>

#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>
#include <Pixy2I2C.h>
#include <Servo.h>

#include <SD.h>
#include <SPI.h>
#include <TeensyThreads.h>

#include "poi.h"
#include "phases.h"

#define RFM_CS 10
#define RFM_INT 8
#define RFM_RST 7
#define RFM_FREQUENCY 434.2F

#define BUZ_PIN 22

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS(&Serial1);
RH_RF95 RFM = RH_RF95(RFM_CS, RFM_INT);
Pixy2I2C Cam = Pixy2I2C();
Servo ServoMotor = Servo();
File dataFile;

//? this is really just a shortcut to prevent repetitions
void play(uint8_t pin, uint16_t freq, uint32_t duration) {
    tone(pin, freq);
    delay(duration);
    noTone(pin);
}

uint32_t initTime = 0;
char *data = (char*) malloc(255 * sizeof(char)); //* a message of maximum 255 characters (RFM limit)
float ground_pressure = static_cast<float>(NAN); //! we will calculate this ourselves

void mainMission(void) {
    static uint32_t ID = 1; //* packet ID; will be used by data analysis
    while (true) {
        BME680.beginReading();

        float temp = static_cast<float>(NAN);
        float pres = static_cast<float>(NAN);
        float hum = static_cast<float>(NAN);
        float alt = static_cast<float>(NAN);
        float lat = static_cast<float>(NAN);
        float lon = static_cast<float>(NAN);


        bool parsedNMEAs = false;
        for (uint8_t i=0; i<5; i++) {
            do {
                GPS.read();
            } while (!GPS.newNMEAreceived());

            if (i > 2) {
                //* read last 2 NMEA sentences, to avoid possible corruption
                //* if at least one is parsed, the boolean will be set to true
                parsedNMEAs = GPS.parse(GPS.lastNMEA());
            }
        }

        if (parsedNMEAs) {
            lat = (GPS.lat == 'S') ? -(GPS.latitude) : GPS.latitude;
            lon = (GPS.lon == 'W') ? -(GPS.longitude) : GPS.longitude;
        }

        float time = static_cast<float>((millis() - initTime) / 1000.0F);

        if (BME680.endReading()) {
            temp = BME680.temperature;
            pres = BME680.pressure / 100.0F;
            hum = BME680.humidity;
            alt = BME680.readAltitude(ground_pressure);
        }

        uint8_t len = snprintf(data, 255 * sizeof(char), "PRb_DATA:%li %.01f %.02f %.02f %.02f %.02f %.04f %.04f", ID, time, temp, pres, hum, alt, lat, lon);
        RFM.send((uint8_t*) data, len);

        if (dataFile) {
            dataFile.println(data);
            dataFile.flush();
        }

        ID++;
    }
}

void setup(void) {
    if (!BME680.begin()) {
        play(BUZ_PIN, 3000, 500);
        delay(1000);
    }
    else {
        for (int i=0; i<5; i++) {
            ground_pressure += (BME680.readPressure() / 100.0F);
        }
        ground_pressure /= 5;

        play(BUZ_PIN, 4000, 500);
        delay(1000);
    }


    if (!BNO055.begin()) {
        play(BUZ_PIN, 3000, 500);
        delay(1000);
    }
    else {
        play(BUZ_PIN, 4000, 500);
        delay(1000);
    }


    if (!GPS.begin(9600)) {
        play(BUZ_PIN, 3000, 500);
        delay(1000);
    }
    else {
        GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
        GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
        play(BUZ_PIN, 4000, 500);
        delay(1000);
    }


    if (!SD.begin(23)) {
        play(BUZ_PIN, 3000, 500);
        delay(1000);
    }
    else {
        if (SD.exists("data.txt")) {
            SD.remove("data.txt");
        }

        dataFile = SD.open("data.txt", FILE_WRITE);
        if (dataFile) {
            play(BUZ_PIN, 4000, 500);
            delay(1000);
        }
    }


    pinMode(RFM_RST, OUTPUT);
    digitalWrite(RFM_RST, LOW);
    delay(100);
    digitalWrite(RFM_RST, HIGH);
    delay(100);

    if (!RFM.init()) {
        play(BUZ_PIN, 3000, 500);
        delay(1000);
    }
    else {
        RFM.setFrequency(RFM_FREQUENCY);
        RFM.setTxPower(20); //* max power for possibly better transmissions
        play(BUZ_PIN, 4000, 500);
        delay(1000);
    }


    if (Cam.init() != 0) {
        play(BUZ_PIN, 3000, 500);
        delay(1000);
    }
    else {
        play(BUZ_PIN, 4000, 500);
        delay(1000);
    }

    ServoMotor.attach(3);
    ServoMotor.write(90);

    initTime = millis();
    threads.addThread(mainMission);


    play(BUZ_PIN, 3322, 500);
    delay(1000);
    play(BUZ_PIN, 2489, 500);
    delay(1000);
    play(BUZ_PIN, 1661, 500);
    delay(1000);
    play(BUZ_PIN, 1864, 500);
    delay(1000);
}

void loop(void) {
    uint8_t phase = detectPhase(&BME680, ground_pressure);
    if (phase == 3 && (BME680.readAltitude(ground_pressure) < 800.0F)) {
        findPOI(&Cam);
        move(&BNO055, &ServoMotor, false);
        sendPosition(&GPS, &RFM);

        //* receive orders from ground station, and move where necessary
        uint8_t buf[50];
        uint8_t len = sizeof(buf);

        if (RFM.recv(buf, &len)) {
            if (strstr((char*) buf, "PRb_PoI")) {
                setDesiredPOI(buf[len-1]);

                uint8_t response[] = "PRb_INFO: PoI information received. Moving.";
                RFM.send(response, sizeof(response));

                move(&BNO055, &ServoMotor, true);
            }
        }
    }
    else if (phase == 4) {
        static uint32_t last_beat = millis();
        if ((millis() - last_beat) > 2000) {
            last_beat = millis();
            play(BUZ_PIN, 4000, 500);
        }
    }
}
