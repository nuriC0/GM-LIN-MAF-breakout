#ifndef _mapLIN_h
#define _mapLIN_h

#include <Arduino.h>

class mapLIN {
    public:
        void dataToSend(byte* data);
        void generateMAPData(int mapKPA);
        void resetCounter();

    private:
        int _2b = 0;
        
        byte _2b1[4] = {1, 140, 255, 252};
        byte _2b2[4] = {1, 172, 255, 252};
        byte _2b3[4] = {1, 204, 255, 252};
        byte _2b4[4] = {1, 236, 255, 252};

};

void mapLIN::generateMAPData(int mapKPA) {
    // compute block index (rangeOffset)
    int delta = mapKPA - 96;

    int rangeOffset = (delta - (delta < 0) * 6) / 7;

    // 7-kPa window
    int baseKPA = 96 + (rangeOffset * 7);

    // position inside the block (0–7)
    int pos = mapKPA - baseKPA;

    // Clamp pos to 0–7 (branchless)
    pos = pos < 0 ? 0 : pos;
    pos = pos > 7 ? 7 : pos;

    // scale 1–255
    int _newValue = (pos * 254) / 7 + 1;

    _2b1[0] = _newValue;
    _2b2[0] = _newValue;
    _2b3[0] = _newValue;
    _2b4[0] = _newValue;

    _2b1[1] = 140 + rangeOffset;
    _2b2[1] = 172 + rangeOffset;
    _2b3[1] = 204 + rangeOffset;
    _2b4[1] = 236 + rangeOffset;
}

void mapLIN::dataToSend(byte* data) {

    if (_2b == 0){
        std::copy(_2b1, _2b1+4, data);
    } else  if (_2b == 1){
        std::copy(_2b2, _2b2+4, data);
    } else if (_2b == 2){
        std::copy(_2b3, _2b3+4, data);
    } else if (_2b == 3){
        std::copy(_2b4, _2b4+4, data);
        _2b = -1;
    }
    _2b = _2b+1;

}

void mapLIN::resetCounter(){
    _2b = 0;
}

#endif
