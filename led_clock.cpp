#include <Arduino.h>
#include "src/timer/Timer.h"
#include "src/NeoPixel/Adafruit_NeoPixel.h"
#include "src/clock.h"
#include "src/TSL2561-Arduino-Library/TSL2561.h"


#define BUTTON_HOURS_PIN 8
#define BUTTON_MINUTE_PIN 9

#define NEOPIXEL_PIN 3

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 60

uint32_t COLOR_HOURS = Adafruit_NeoPixel::Color(200, 0, 0);
uint32_t COLOR_MINUTES = Adafruit_NeoPixel::Color(0, 0, 200);
uint32_t COLOR_SECONDS = Adafruit_NeoPixel::Color(0, 200, 0);


Logger logger;
ClockConfig_t clk_cfg = {5, 6, 7};
Clock clk(&clk_cfg, &logger);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);


bool hour_pin_state, minute_pin_state;



bool isHourPinPressed() {
    return static_cast<bool>(digitalRead(BUTTON_HOURS_PIN));
}

bool isMinutePinPressed() {
    return static_cast<bool>(digitalRead(BUTTON_MINUTE_PIN));
}



void setTimeOnLeds(){
    // hours
    uint16_t hour = clk.current_time.hr;
    if (hour > 12) {
        hour -= 12;
    }
    if (hour == 12) {
        hour = 0;
    }
    pixels.setPixelColor(hour * 5, COLOR_HOURS);

    // minutes
    pixels.setPixelColor(clk.current_time.min, COLOR_MINUTES);

    // seconds
    pixels.setPixelColor(clk.current_time.sec, COLOR_SECONDS);
}



void setLedsToDefault(void) {
    for(int i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }

    // add orientation marks
    pixels.setPixelColor(0, pixels.Color(40, 40, 40));
    pixels.setPixelColor(NUMPIXELS/4, pixels.Color(40, 40, 40));
    pixels.setPixelColor(NUMPIXELS/2, pixels.Color(40, 40, 40));
    pixels.setPixelColor(NUMPIXELS/4*3, pixels.Color(40, 40, 40));
}



/** Arduino setup function */
void setup() {
    logger.setup();
    logger.log(Logger::INFO, "Starting up clock...");

    // Configure Button Pin as input
    pinMode(BUTTON_MINUTE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_HOURS_PIN, INPUT_PULLUP);


    clk.setup();

    pixels.begin();
    setLedsToDefault();

    hour_pin_state = isHourPinPressed();
    minute_pin_state = isMinutePinPressed();
}


/** Arduino loop function */
void loop() {
    clk.update();

    if (isHourPinPressed() && !hour_pin_state) {
        clk.incrementHours();
    }
    else if (isMinutePinPressed() && !minute_pin_state) {
        clk.incrementMinutes();
    }

    hour_pin_state = isHourPinPressed();
    minute_pin_state = isMinutePinPressed();

    setLedsToDefault();
    setTimeOnLeds();
    pixels.show();

    //clk.logCurrentTime();

    delay(50);
}
