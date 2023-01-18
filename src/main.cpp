#include <Arduino.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>
#include <Pixy2I2C.h>
#include <SPI.h>
#include <SD.h>

#include "detection.h"

#define FREQUENCY 434.2F //* frequency used for the RFM96W Sensor, change only if necessary
#define PRESSURE 1013.25F //* sea-level pressure, used to read altitude from the BME680 Sensor
#define DEBUG //* debug mode, used for printing out raw sensor data

Adafruit_BME680 BME680 = Adafruit_BME680();
Adafruit_BNO055 BNO055 = Adafruit_BNO055();
Adafruit_GPS GPS = Adafruit_GPS();
RH_RF95 RFM = RH_RF95();
Pixy2I2C Cam = Pixy2I2C();
File dataFile;

uint32_t timer = millis();
char *data = NULL;

void setup(void) {
    Serial.begin(115200);
    while (!Serial) {
        delay(100); //* Wait for serial port to connect, then start up
    }

    #ifdef DEBUG
        if (!BME680.begin()) {
            while (true) Serial.println("[Debug]: Could not initialise the BME680 Sensor.");
        }

        if (!BNO055.begin()) {
            while (true) Serial.println("[Debug]: Could not initialise the BNO055 Sensor.");
        }

        if (!GPS.begin(9600)) {
            while (true) Serial.println("[Debug]: Could not initialise the GPS Sensor.");
        }

        if (Cam.init() != 0) {
            while (true) Serial.println("[Debug]: Could not initialise Pixy2.");
        }

        if (!RFM.init()) {
            while (true) Serial.println("[Debug]: Could not initialise the RFM96W Sensor.");
        }

        if (!SD.begin()) {
            while (true) Serial.println("[Debug]: Could not initialise the SD Card.");
        }
    #else
        BME680.begin();
        BNO055.begin();
        GPS.begin(9600);
        Cam.init();
        RFM.init();
        SD.begin();
    #endif

    data = (char*) malloc(100 * sizeof(char));
    if (!data) {
        Serial.println("[Debug]: Could not allocate memory for the mission data.");
    }

    /*
    * if we've made it here, all sensors have been successfully initialised
    * now configure some settings and we're ready
    */

    if (SD.exists("data.txt")) {
        SD.remove("data.txt");
    }

    dataFile = SD.open("data.txt", FILE_WRITE);

    RFM.setFrequency(FREQUENCY);
    RFM.setModeTx(); //* will only send packets, not receive

    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_5HZ); //* 5 Hz GPS read rate (subject to change)
    Cam.changeProg("video"); //* RGB detection program
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
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.println("°C");
        }

        if (pressure) {
            Serial.print("Pressure: ");
            Serial.print(pressure);
            Serial.println(" hPa");
        }

        if (humidity) {
            Serial.print("Humidity: ");
            Serial.print(humidity);
            Serial.println(" %");
        }

        if (altitude) {
            Serial.print("Approx. Altitude: ");
            Serial.print(altitude);
            Serial.println("m");
        }
    #endif

    if (GPS.newNMEAreceived()) {
        #ifdef DEBUG
            Serial.print(GPS.lastNMEA());
        #endif
        GPS.parse(GPS.lastNMEA());
    }

    snprintf(data, 100 * sizeof(char), "%.02f %.02f %.02f %.02f %.04f %.04f", temperature, pressure, humidity, altitude, GPS.longitude, GPS.latitude);
    if (dataFile) {
        dataFile.println(data);
        dataFile.flush();
    }

    RFM.waitPacketSent();
    RFM.send((uint8_t*) data, strlen(data));

    uint8_t phase = detectPhase(altitude);
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
