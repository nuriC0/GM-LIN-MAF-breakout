# GM MAF LIN Breakout - @oz_tt_c8

Over the last few years, GM has begun using MAF sensors that communicate **barometric pressure** and **intake air temperature (IAT)** to the ECM over a LIN 2.x bus.  This can become problematic in modified vehicles where boost pressure is introduced upstream of the MAF.  In those cases, barometric pressure readings can become erratic, which in turn affects other ECM control routines.
Breaking out the IAT signal helps address this by allowing the temperature to be sourced from a location other than the MAF, providing more accurate data for modified configurations.

Another benefit is the ability to use an LS3 MAF on newer vehicles, providing improved resolution when needed.

This GitHub project demonstrates how to design a PCB, identifies the required components, and provides the code needed to implement a breakout for a C8 Corvette MAF. (same for other GM cars)  This new device assumes the role of the LIN slave and is responsible for responding to requests from the ECM master. As a result, the LIN signal or wiring is <ins>no longer required on the MAF side</ins>.


### Required Parts

ESP32 S3 DEV Board [DigiKey](https://www.digikey.ca/en/products/detail/espressif-systems/ESP32-S3-DEVKITC-1-N8/15199021) \
Recom DC-to-DC converter R-78E5.0-0.5 [DigiKey](https://www.digikey.ca/en/products/detail/recom-power/R-78E5-0-0-5/2834904) \
MIKROE-3816 LIN board [DigiKey](https://www.digikey.ca/en/products/detail/mikroelektronika/MIKROE-3816/10712654) \
BMP180 barometric module (Amazon) \
2.4K ohm resistor \
TE AMP PCB connector 6437288-4 \
TE AMP connector 3-1437290-8 \
TE AMP pins 3-1447221-5

The TE AMP connectors mentioned above are standard connectors widely used by Holley in their products. Other options can be used as well, but these were my preference and what I had available.  Can also be found on DigiKey.

### PCB Design
<img src="\images\PCB_editor.png" width="50%">

### Final Product
<img src="\images\PCB_done.png" width="50%">


### About Microcontroller Code
Code was written and deployed using free Visual Studio Code and [PlatformIO](https://platformio.org/). 

The ESP32‑S3 is a dual‑core MCU. The included source code leverages both cores using FreeRTOS. One core is responsible for handling LIN communication along with the IAT and barometric pressure sensors, while the second core uses ESP‑NOW to transmit messages to other devices. The ESP‑NOW functionality can be commented out or disabled if it’s not needed. 

The source code also supports the use of an IAT sensor from the LS9 engine (12614717). In general, any resistance‑based temperature sensor can be used; you simply need to configure the appropriate resistance and temperature ranges in the iatSensor.h file. These values are typically available online or can be obtained through tuning tools such as HP Tuners. 

The current implementation supports barometric pressure values in the range of 89 kPa to 110 kPa. This limitation is imposed by the source code, not by LIN itself — LIN can support values from 0 kPa up to 255 kPa.  If a different range is required, the code can be adjusted accordingly.
