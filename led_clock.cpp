#include <Arduino.h>
#include "src/timer/Timer.h"
#include "src/NeoPixel/Adafruit_NeoPixel.h"
#include "src/arduino-ds1302/DS1302.h"
#include "src/TSL2561-Arduino-Library/TSL2561.h"



/* **** Logging ***** */
// uncomment define to enable logging to serial console
#define LOG_ENABLED

void log(const char str[]) {
#ifdef LOG_ENABLED
    Serial.print(str);
    Serial.print("\n");
#endif
}

/** Arduino setup function */
void setup() {
    #ifdef LOG_ENABLED
    Serial.begin(9600);
    #endif

    log("Start initializing inputs...");

}


/** Arduino loop function */
void loop() {
   
}
