#include <Adafruit_BNO055.h>
#include <Pixy2I2C.h>
#include <Servo.h>

#include <stdint.h>

char* findPOI(Pixy2I2C* Cam);
void moveToPOI(Adafruit_BNO055* BNO, Servo* ServoMotor, const char *coordinates);
