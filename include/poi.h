#include <Adafruit_BNO055.h>
#include <Adafruit_GPS.h>
#include <RH_RF95.h>
#include <Pixy2I2C.h>
#include <Servo.h>
#include <stdint.h>

void findPOI(Pixy2I2C* Cam);
void setDesiredPOI(uint8_t num);

void sendPosition(Adafruit_GPS* GPS, RH_RF95* RFM);
void move(Adafruit_BNO055* BNO, Servo* ServoMotor, bool force=false);
