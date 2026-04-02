#ifndef _WiFiLib_h
#define _WiFiLib_h

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include <esp_now.h>

//EC:DA:3B:96:EF:94
class WiFiLib {

    public:
        //my data to send

        typedef struct {
            int iat;
            int pressure;
        } SensorData;

        bool wifi_setup();
        bool sendData(SensorData dataOut);

    private:
        uint8_t MACScreen[6] = {0xDC, 0xDA, 0x0C, 0x54, 0x67, 0x04};//who are we sending data to
        esp_now_peer_info_t peerInfo;

};


bool WiFiLib::wifi_setup() {

  if (esp_now_init() != ESP_OK) {
    return false;
  }

  memset(&peerInfo, 0, sizeof(peerInfo));

  // Register peer
  memcpy(peerInfo.peer_addr, MACScreen, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    return false;
  }

  return true;
};

bool WiFiLib::sendData(SensorData dataOut) {
  return (esp_now_send(MACScreen, (uint8_t *) &dataOut, sizeof(dataOut)) == ESP_OK);
}

#endif