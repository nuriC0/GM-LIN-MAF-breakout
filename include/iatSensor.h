#ifndef _iatSensor_h
#define _iatSensor_h

#include <Arduino.h>
#include "gpio.h"
#include "MedianFilterLib2.h"
#include "map"
#include <cmath>

class iatSensor {

    public:
        int getIAT();
        void readIAT();
        int valueInF(int resValue);

    private:

        float BOX_RESISTOR = 2400.0;//known resistor

        std::map<long, long> ls9IAT = {
                                            //{46, 302},  //resistance, temp in F
                                            //{59, 284},
                                            //{76, 266},
                                            //{87, 257},
                                            //{99, 248},
                                            //{113, 239},   //you have bigger issues if you are here
                                            {131, 230},
                                            {151, 221},
                                            {175, 212},
                                            {203, 203},
                                            {238, 194},
                                            {279, 185},
                                            {329, 176},
                                            {389, 167},
                                            {462, 158},
                                            {551, 149},
                                            {660, 140},
                                            {796, 131},
                                            {965, 122},
                                            {1177, 113},
                                            {1443, 104},
                                            {1778, 94},
                                            {2205, 85},
                                            {2752, 76},
                                            {3457, 67},
                                            {4373, 58},
                                            {7153, 40},
                                            {9256, 31},
                                            {12073, 22},
                                            {15873, 13},
                                            {28146, -4},
                                            {51791, -22},
                                            {99326, -40}
                                        };
        //  Filters
        MedianFilter2<uint32_t> iatValues = MedianFilter2<uint32_t>(5);

};

/*
    gets temp in F
*/
int iatSensor::getIAT() {
    float voltage = iatValues.GetFiltered() / 1000.0;
    voltage = (3.3/voltage) - 1.0;
    float iatRes = BOX_RESISTOR / voltage;
    return valueInF(iatRes);;
}

/*
    reads data into Median Filter
*/
void iatSensor::readIAT(){
    uint32_t _iat = analogReadMilliVolts(IAT_PIN);
    if (_iat > 100 && _iat < 8100){
        iatValues.AddValue(_iat);
    }
}

/*
    resistance to temperature
*/
int iatSensor::valueInF(int resValue){


    // Exact match
    auto exact = ls9IAT.find(resValue);
    if (exact != ls9IAT.end()) {
        return static_cast<int>(exact->second);
    }

    // First element with key >= resistance
    auto upper = ls9IAT.lower_bound(resValue);

    // Clamp below range
    if (upper == ls9IAT.begin()) {
        return static_cast<int>(upper->second);
    }

    // Clamp above range
    if (upper == ls9IAT.end()) {
        return static_cast<int>(std::prev(upper)->second);
    }

    auto lower = std::prev(upper);

    long r1 = lower->first;
    long t1 = lower->second;
    long r2 = upper->first;
    long t2 = upper->second;

    // "map"-style linear interpolation:
    // temp = t1 + (resistance - r1) * (t2 - t1) / (r2 - r1)
    double temp = static_cast<double>(t1) +
                  static_cast<double>(resValue - r1) *
                  static_cast<double>(t2 - t1) /
                  static_cast<double>(r2 - r1);

    return static_cast<int>(std::lround(temp)); // nearest int

}


#endif