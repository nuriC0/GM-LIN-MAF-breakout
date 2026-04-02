#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <gpio.h>
#include <iatSensor.h>

#include <iatLIN.h>
#include <mapLIN.h>

#include <freertos/FreeRTOS.h>
#include <esp_now.h>
#include <WiFi.h>
#include <WiFiLib.h>
#include "driver/uart.h"

#define LIN_UART UART_NUM_1
#define TX_PIN 16
#define RX_PIN 15

static QueueHandle_t uart_queue;

int BAR_LIN = 0x2B;
int IAT_LIN = 0x6A;
int RES_LIN = 0xa8;

//  CPU CORES
TaskHandle_t MAIN_CPU;
TaskHandle_t WIFI_CPU;

//  BAROMETER
TwoWire baroPressure = TwoWire(0);
Adafruit_BMP085 bmp;

WiFiLib wiFiCom;
//  IAT
iatSensor iat;

mapLIN C8Map;
iatLIN C8Iat;

volatile WiFiLib::SensorData g_sensor;

//functions
void MainCpuCode( void * pvParameters );
void WiFiCpuCode( void * pvParameters );

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

uint8_t LINChecksum(const uint8_t* data, size_t size, uint8_t protectedId);

void initLIN();
void handlePID(uint8_t pid, byte* mapData, byte* iatData);

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    vTaskDelay(1); // wait for serial port to connect
  }

  pinMode(IAT_PIN, INPUT_PULLDOWN);
  
  iat.readIAT();

  //get our barometer going
  baroPressure.begin(4, 5, 1000);
  
  vTaskDelay(pdMS_TO_TICKS(500));

  if (!bmp.begin(BMP085_STANDARD, &baroPressure)) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }

  iat.readIAT();

  g_sensor.iat = iat.getIAT();
  g_sensor.pressure = (bmp.readPressure() + 500) / 1000;

  xTaskCreatePinnedToCore(
                MainCpuCode,
                "MAINCode",
                20000,
                NULL,
                12,
                &MAIN_CPU,
                0);

  xTaskCreatePinnedToCore(
                WiFiCpuCode,
                "WiFiCpuCode",
                20000,
                NULL,
                8,
                &WIFI_CPU,
                1);

}

void loop() {
    //this should be empty
}

void MainCpuCode( void * pvParameters ){

    initLIN();

    byte mapData[5];
    C8Map.generateMAPData(((bmp.readPressure() + 500) / 1000));//but how often does baro change ?

    byte iatData[6];
    C8Iat.generateIATData(iat.getIAT()); //get latest IAT temp

    Serial.println("LIN Started");

    uint8_t data[64];

    enum LinState {
        WAIT_BREAK,
        WAIT_SYNC,
        WAIT_PID
    };

    LinState state = WAIT_BREAK;

    for (;;)
    {
        uart_event_t event;

        if (xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            
            switch (event.type)
            {
                case UART_BREAK:
                    state = WAIT_SYNC;
                    break;
                case UART_FRAME_ERR:
                    // optional: reset safely, but don't assume valid break
                    state = WAIT_BREAK;
                    break;
                case UART_FIFO_OVF:
                    uart_flush_input(LIN_UART);
                    xQueueReset(uart_queue);
                    state = WAIT_BREAK;
                    break;
                case UART_DATA:
                {
                    int len = uart_read_bytes(LIN_UART, data, sizeof(data), 0);

                    for (int i = 0; i < len; i++)
                    {
                        uint8_t v = data[i];

                        switch (state)
                        {
                            case WAIT_BREAK:
                                break;
                            case WAIT_SYNC:
                                if (v == 0x55) {
                                    state = WAIT_PID;
                                } else {
                                    state = WAIT_BREAK;
                                }
                                break;
                            case WAIT_PID:
                                handlePID(v, mapData, iatData);
                                state = WAIT_BREAK;
                                break;
                        }
                    }
                    break;
                }
            }
        }
    }
}

void handlePID(uint8_t pid, byte* mapData, byte* iatData)
{
    if (pid == BAR_LIN)
    {
        C8Map.dataToSend(mapData);
        mapData[4] = LINChecksum(mapData, 4, BAR_LIN);

        uart_write_bytes(LIN_UART, (const char*)mapData, 5);
    }
    else if (pid == IAT_LIN)
    {
        C8Iat.dataToSend(iatData);
        iatData[5] = LINChecksum(iatData, 5, IAT_LIN);

        uart_write_bytes(LIN_UART, (const char*)iatData, 6);
    }
    else if (pid == RES_LIN)
    {
        C8Map.resetCounter();
        iat.readIAT();

        int currentF = iat.getIAT();
        C8Iat.generateIATData(currentF);
        g_sensor.iat = currentF;

    }
}

void initLIN()
{
    uart_config_t uart_config = {
        .baud_rate = 10417,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(LIN_UART, 256, 0, 10, &uart_queue, 0);
    uart_param_config(LIN_UART, &uart_config);
    uart_set_pin(LIN_UART, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_set_rx_timeout(LIN_UART, 2);  // small = faster interrupt

    uart_enable_rx_intr(LIN_UART);
    uart_set_rx_full_threshold(LIN_UART, 1);

}


uint8_t LINChecksum(const uint8_t* data, size_t size, uint8_t protectedId)
{
    uint16_t sum = 0;

    //  Enhanced mode
    sum += protectedId;

    // Add all data bytes
    for (size_t i = 0; i < size; ++i) {
        sum += data[i];
        
        // Fold carry immediately (LIN requires modulo-255 addition)
        if (sum > 255) {
            sum -= 255;
        }
    }

    // Final inversion
    return static_cast<uint8_t>(~sum);
}




// ESP-NOW
void WiFiCpuCode( void * pvParameters ){

    WiFi.mode(WIFI_STA); 
    //Serial.println(WiFi.macAddress());
    
    bool wifiActive = wiFiCom.wifi_setup(); 

    if (wifiActive == false){
        vTaskDelay(pdMS_TO_TICKS(1000));
        wifiActive = wiFiCom.wifi_setup();

        if (wifiActive == false){
            vTaskDelay(pdMS_TO_TICKS(2000));
            wifiActive = wiFiCom.wifi_setup();
        }
    }

    if (wifiActive == false) {
        return; //disable CPU and activities
    }


    for(;;){
        WiFiLib::SensorData local = *(WiFiLib::SensorData*)&g_sensor;

        wiFiCom.sendData(local);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

