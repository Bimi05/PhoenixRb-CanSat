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

#define SEALEVEL_PRESSURE 1013.25F //* used for altitude calculation (BME680) - change if necessary

#define RFM_CS 10
#define RFM_INT 8
#define RFM_RST 7
#define RFM_FREQUENCY 434.2F

#define BUZ_PIN 22

#define GPSSerial Serial1

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS(&GPSSerial);
RH_RF95 RFM = RH_RF95(RFM_CS, RFM_INT);
Pixy2I2C Cam = Pixy2I2C();
Servo ServoMotor = Servo(); //* servo motor for our controlled movement
File dataFile;

uint32_t ID = 1; //* packet ID; will be used by data analysis
char *data = (char*) malloc(255 * sizeof(char)); //* a message of maximum 255 characters (RFM limit)

void setup(void) {
    while (!Serial);
    Serial.begin(115200);
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
        GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
        GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ);
        Serial.println("[Debug] Sensor Ultimate GPS initialised!");
    }

    if (!SD.begin(23)) {
        Serial.println("[Debug] Could not initialise the SD Card.");
    }
    else {
        dataFile = SD.open("data.txt", O_RDWR);
        if (dataFile) {
            dataFile.seek(0); //* start writing at first file byte, to overwrite everything
            Serial.println("[Debug] SD card initialised!");
        }
    }

    if (!RFM.init()) {
        Serial.println("[Debug] Could not initialise the RFM9x LoRa Sensor.");
    }
    else {
        RFM.setFrequency(RFM_FREQUENCY);
        RFM.setModeTx();
        RFM.setTxPower(20); //* max power for possibly better transmissions
        Serial.println("[Debug] Sensor RFM9x LoRa initialised!");
    }

    if (Cam.init() != 0) {
        Serial.println("[Debug] Could not initialise Pixy2.");
    }
    else {
        Cam.changeProg("video");
        Serial.println("[Debug] Camera Pixy2 initialised!");
    }

    ServoMotor.attach(3);

    Serial.println("[Debug] Initialisation complete!");
    Serial.println("----------------------------------------------------------------------");
}

#define DEBUG_MODE //! remove/comment out when done testing

void loop(void) {
    uint32_t start = millis();
    memset(data, '-', 255 * sizeof(char));

    BME680.performReading();

    float temp = BME680.temperature;
    float pres = BME680.pressure / 100.0F;
    float hum = BME680.humidity;
    float alt = BME680.readAltitude(SEALEVEL_PRESSURE);

    for (uint8_t i=0; i<2; i++) {
        while (!GPS.newNMEAreceived()) {
            GPS.read();
        }
    }
    GPS.parse(GPS.lastNMEA());

    float lat = (GPS.lat == 'S') ? -(GPS.latitude) : GPS.latitude;
    float lon = (GPS.lon == 'W') ? -(GPS.longitude) : GPS.longitude;

    float time = static_cast<float>(millis() / 1000.0F);
    uint8_t len = snprintf(data, 255 * sizeof(char), "PRb:%li %.01f %.02f %.02f %.02f %.02f %.04f %.04f", ID, time, temp, pres, hum, alt, lat, lon);


    RFM.send((uint8_t*) data, len);

    dataFile.println(data);
    dataFile.flush();

    #ifdef DEBUG_MODE
        Serial.println(data);
    #endif

    // uint8_t phase = detectPhase(&BME680, SEALEVEL_PRESSURE);
    uint8_t phase = 1;
    if (phase == 3) {
        char* pos = findPOI(&Cam);
        moveToPOI(&BNO055, &ServoMotor, pos);
    }
    else if (phase == 4) {
        static uint32_t last_beat = millis();
        if ((millis() - last_beat) > 2000) {
            last_beat = millis();

            //! melody to be determined :)
            tone(BUZ_PIN, 3322.438F, 500);
            tone(BUZ_PIN, 2489.016F, 500);
            tone(BUZ_PIN, 1661.219F, 500);
            tone(BUZ_PIN, 1864.655F, 500);
        }
    }


    ID++;
    if ((millis() - start) < 100) {
        delay(100 - (millis() - start));
    }
}
