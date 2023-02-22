#include <Arduino.h>
#include "src/clock.h"
#include "src/leds.h"
#include "src/ambient.h"


#define BUTTON_HOURS_PIN 8
#define BUTTON_MINUTE_PIN 9


Logger logger;
ClockConfig_t clk_cfg = {5, 6, 7};
Clock clk(&clk_cfg, &logger);
LedConfig_t led_cfg = {.inner_ring_pin=3, .outer_ring_pin=4,
                       .inner_ring_num_leds=229, .outer_ring_num_leds=252};
Leds led(&led_cfg, &logger);

LightSensorConfig_t light_cfg = {.tsl_i2c_addr=0x39, .update_time_ms=5000,
                                 .integration_time=TSL2561_INTEGRATIONTIME_101MS};
Ambient ambient(&light_cfg, &logger);


bool hour_pin_state, minute_pin_state;


bool isHourPinPressed() {
    return static_cast<bool>(digitalRead(BUTTON_HOURS_PIN));
}

bool isMinutePinPressed() {
    return static_cast<bool>(digitalRead(BUTTON_MINUTE_PIN));
}



/** Arduino setup function */
void setup() {
    logger.setup();
    logger.log(Logger::INFO, "Starting up clock...");

    // Configure Button Pin as input
    pinMode(BUTTON_MINUTE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_HOURS_PIN, INPUT_PULLUP);

    hour_pin_state = isHourPinPressed();
    minute_pin_state = isMinutePinPressed();

    clk.setup();
    led.setup();
    ambient.setup();
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

    if (ambient.update()) {
        led.updateBrightness(ambient.brightness_lux);
    }
    led.update(clk.current_time);
}
