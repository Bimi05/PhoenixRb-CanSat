#include "poi.h"

uint8_t avg_red, avg_green, avg_blue;
uint16_t position[2];

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

bool scanned = false;

void findPOI(Pixy2I2C* Cam) {
    if (scanned) {
        return;
    }

    findAverageValues(Cam);

    uint8_t r, g, b;

    for (uint16_t i=0; i<Cam->frameHeight; i++) {
        for (uint16_t j=0; i<Cam->frameWidth; i++) {
            Cam->video.getRGB(j, i, &r, &g, &b);
        }
    }

    scanned = true;
}

void move(Servo* ServoMotor) {
    float x = static_cast<float>(position[0]);
    float y = static_cast<float>(position[1]);
    float angle = atanf(y/x); //* arctan(θ) gives us the angle to turn

    if (angle != 0) {
        int8_t min_value = (angle > 0) ? 5 : -5;
        if (abs(angle) < abs(min_value)) {
            min_value = angle;
        }

        int8_t degrees = (abs(angle) > abs(min_value)) ? (angle/5) : min_value;
        ServoMotor->write(((angle > 0) ? degrees : -(degrees)));
    }
    else {
        //! we're gonna spin in circles until we land
        ServoMotor->write(180);
    }
}
