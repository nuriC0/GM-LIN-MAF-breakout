# GM MAF LIN Breakout

Over the last few years, GM has begun using MAF sensors that communicate **barometric pressure** and **intake air temperature (IAT)** to the ECM over a LIN bus.  This can become problematic in modified vehicles where boost pressure is introduced upstream of the MAF.  In those cases, barometric pressure readings can become erratic, which in turn affects other ECM control routines.
Breaking out the IAT signal helps address this by allowing the temperature to be sourced from a location other than the MAF, providing more accurate data for modified configurations.

Another benefit is the ability to use an LS3 MAF on newer vehicles, providing improved resolution when needed.

This GitHub project demonstrates how to design a PCB, identifies the required components, and provides the code needed to implement a breakout for a C8 Corvette MAF. (same for other GM cars)


