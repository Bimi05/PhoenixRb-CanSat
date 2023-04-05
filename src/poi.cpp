#include "poi.h"

char* findPOI(Pixy2I2C* Cam) {
    char *position = (char*) malloc(10 * sizeof(char));

    float x = 0.0F;
    float y = 0.0F;
    // TODO: find POI through Pixy2 camera

    snprintf(position, 10, "%f,%f", x, y); //! x and y are RELATIVE to Pixy2's (0, 0)
    return position;
}

void moveToPOI(Adafruit_BNO055* BNO, Servo* ServoMotor, const char *coordinates) {
    sensors_event_t data;
    BNO->getEvent(&data);

    float x = 0;
    float y = 0;

    float angle = atanf(x/y);
    ServoMotor->write(angle / 6);
}
