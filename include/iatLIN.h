#ifndef _iatLIN_h
#define _iatLIN_h

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include <esp_now.h>


class iatLIN {

    public:
        //my data to send
        void generateIATData(int tempInF);
        void dataToSend(byte* data);

    private:

        int _iat = 0;
        int _iatOffset = 0;
        int _iatLow = 0;
        int _iatCheck = 187;
        
        #define SEGMENT (27.0f)     //how many temp points make up a single POINT, Fahrenheit
        #define TOP_TEMP (221)      //magic number

};


void iatLIN::generateIATData(int tempInF) {
    //convert Fahrenheit into LIN data ( would be easier if we used celsius )
    _iatOffset = ceil((TOP_TEMP - tempInF) / SEGMENT);
    _iatLow = 0;

    int modNum = 0;
    modNum = tempInF - (TOP_TEMP - (_iatOffset * SEGMENT));

    if (modNum > 0){
        _iatLow = (int)(modNum * (255.0 / SEGMENT));
        if (_iatLow > 254){
            _iatLow = 254;//just in case
        }
    }

}

void iatLIN::dataToSend(byte* data) {

    int secondHigh = 236 - _iatOffset;
    if (_iat == 1){
        secondHigh = 140 - _iatOffset;
    } else if (_iat == 2){
        secondHigh = 172 - _iatOffset;
    } else if (_iat == 3){
        secondHigh = 204 - _iatOffset;
        _iat = -1;
    }

    _iat = _iat + 1;

    data[0] = _iatLow;
    data[1] = secondHigh;
    data[2] = _iatCheck;
    data[3] = _iatLow;
    data[4] = (252-_iatOffset);

}


#endif