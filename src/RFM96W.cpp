#include <stdint.h>
#include <Arduino.h>
#include <RH_RF95.h>

#include "RFM96W.h"

float frequency;
RH_RF95 RF = RH_RF95();

RFM96W::RFM96W(float freq = 0.0F) {
    frequency = freq;
}

void RFM96W::init(void) {
    if (!RF.init()) {
        Serial.println("[Debug]: Could not initialise the RFM96W Sensor.");
        while (true);
    }

    if (frequency != 0.0F) {
        RF.setFrequency(frequency);
    }
}

//encryption
//String key = "";                 //?Fix pending
//RF.setEncryptionKey(key);



void loop()
    {
        if (RF.available())
        {
            uint8_t sentdat[] = data;
            RF.send(sentdat, sizeof (sentdat));
            RF.waitPacketSent();
        }
        
        
    }
//Sends the collected data to the ground station
void RFM96W::send(const char *data) 
{
    //RF.send("hello world", sizeof 11);
    //RF.waitPacketSent();
}