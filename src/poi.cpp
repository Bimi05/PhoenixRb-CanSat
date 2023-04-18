#include "poi.h"

uint8_t avg_red, avg_green, avg_blue;

uint8_t poi_index = 0;
uint16_t desired_POI[2] = { 0, 0 };
uint16_t POIs[5][2] = {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 }
};

bool scanned = false;
bool sendPOI = false;
char* pos = (char*) malloc(25 * sizeof(char));


void findAverageValues(Pixy2I2C* Cam) {
    uint8_t r, g, b;

    for (uint16_t i=0; i<Cam->frameHeight; i++) {
        for (uint16_t j=0; i<Cam->frameWidth; i++) {
            Cam->video.getRGB(j, i, &r, &g, &b);

            avg_red += r;
            avg_green += g;
            avg_blue += b;
        }
    }

    avg_red /= (Cam->frameHeight * Cam->frameWidth);
    avg_green /= (Cam->frameHeight * Cam->frameWidth);
    avg_blue /= (Cam->frameHeight * Cam->frameWidth);
}


void findPOI(Pixy2I2C* Cam) {
    if (scanned) {
        return;
    }

    findAverageValues(Cam);

    uint8_t num = 0;
    uint8_t r, g, b;
    for (uint16_t i=0; i<Cam->frameHeight; i++) {
        for (uint16_t j=0; i<Cam->frameWidth; i++) {
            Cam->video.getRGB(j, i, &r, &g, &b);

            if (abs(r-avg_red) > 50 && abs(g-avg_green) > 50 && abs(b-avg_blue) > 50) {
                POIs[num][0] = (j-Cam->frameWidth);
                POIs[num][1] = (i-Cam->frameHeight);
                num++;
            }
        }
    }

    scanned = true;
}


void setDesiredPOI(uint8_t num) {
    desired_POI[0] = POIs[num][0];
    desired_POI[1] = POIs[num][1];
}


void sendPosition(Adafruit_GPS* GPS, RH_RF95* RFM) {
    if (!sendPOI) {
        return;
    }

    memset(pos, 0, 50 * sizeof(char));
    do {
        GPS->read();
    } while (!GPS->newNMEAreceived());
    GPS->parse(GPS->lastNMEA());

    float lat = (GPS->lat == 'S') ? -(GPS->latitude) : GPS->latitude;
    float lon = (GPS->lon == 'W') ? -(GPS->longitude) : GPS->longitude;

    uint8_t len = snprintf(pos, 25 * sizeof(char), "PRb_PoI: (%i) %.04f,%.04f", poi_index + 1, lat, lon);
    RFM->send((uint8_t*) pos, len);
    RFM->waitPacketSent();
    sendPOI = false;
}


void move(Adafruit_BNO055* BNO, Servo* ServoMotor, bool force) {
    //* force here is used for the PoI received by the ground station
    //* the PoI locations will be sent to ground station one by one
    //* the ground station will then pick one, send us the info
    //* and we'll "force" land there

    sensors_event_t BNO_data;
    float x, y;

    if (force) {
        x = static_cast<float>(desired_POI[0]);
        y = static_cast<float>(desired_POI[1]);
    }
    else {
        x = static_cast<float>(POIs[poi_index][0]);
        y = static_cast<float>(POIs[poi_index][1]);
    }

    float angle = atanf(y/x); //* arctan(θ) gives us the angle to turn
    float turn = 180.0F;

    BNO->getEvent(&BNO_data);
    if ((angle > -1.0F) && (angle < 1.0F)) {
        if ((x > -1.0F) && (x < 1.0F)) {
            if (!force) {
                poi_index++;
                sendPOI = true;
            }
        }
        return;
    }
    else {
        turn = (BNO_data.orientation.z + angle);
    }

    ServoMotor->write((angle > 0) ? 180 : 0);
    while (abs(turn-BNO_data.orientation.z) > 1.0F) {
        BNO->getEvent(&BNO_data);
    }
    ServoMotor->write(90);
}
