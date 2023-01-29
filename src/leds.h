#pragma once

#include "NeoPixel/Adafruit_NeoPixel.h"
#include "config.h"
#include "logger.h"

class Time;

class Leds {

public:
    explicit Leds(LedConfig_t * config, Logger * logger);

    void setup(void);
    void update(const Time & current_time);
    void updateBrightness(const uint32_t & lux);

private:
    typedef struct {
        uint32_t hours;
        uint32_t minutes;
        uint32_t seconds;
        uint32_t marks;
    } colors_t;

    typedef union {
        uint32_t color;
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t dummy;
        } splitted;
    } color_t;

    const colors_t BASE_COLOR;
    const colors_t MIN_COLOR;
    colors_t _dimmed_color;

    Logger * _logger;
    LedConfig_t * _config;
    Adafruit_NeoPixel _inner_ring;
    Adafruit_NeoPixel _outer_ring;
    uint8_t _previous_second;
    uint32_t _time_previous_second;

    uint16_t _outer_ring_min_multiplier;
    uint32_t _outer_ring_second_multiplier;

    uint32_t _last_status_time_ms;

    void setInnerRingDefaults(void);
    void setOuterRingDefaults(void);
    void setTimeOnOuterRing(uint8_t hour, uint8_t min, uint16_t ms_in_min);
    void setTimeOnInnerRing(uint8_t hour, uint8_t min, uint8_t sec);
    uint16_t estimateMillisecondsInMinute(uint8_t current_sec);

    uint32_t scaleColor(const color_t & input, const color_t & min, uint16_t scale);
};
