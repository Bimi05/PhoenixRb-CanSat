#include "poi.h"

char* findPOI(Pixy2I2C* Cam) {
    char *position = (char*) malloc(10 * sizeof(char));

    uint8_t x = 120;
    uint8_t y = 60;
    // TODO: find POI through Pixy2 camera

    snprintf(position, 10, "%i,%i", x, y);
    return position;
}

void moveToPOI(Adafruit_BNO055* BNO, Servo* ServoMotor, const char *coordinates) {
    // TODO: use BNO055 and both servos to move to the POI
}
