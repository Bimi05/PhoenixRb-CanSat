#include <stdint.h>
#include <Arduino.h>
#include <RH_RF95.h>

class RFM96W {
    public:
        RFM96W(float freq = 0.0);
        void init();
        void send(const char *data);
};