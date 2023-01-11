#include <stdint.h>
#include <Arduino.h>
#include <RH_RF95.h>

class RFM96W {
    public:
        RFM96W(float);
        void init();
        void send(const char*);
};