#include "leds.h"
#include "arduino-ds1302/DS1302.h"
#include "utils.h"


uint32_t COLOR_HOURS = Adafruit_NeoPixel::Color(200, 0, 0);
uint32_t COLOR_MINUTES = Adafruit_NeoPixel::Color(0, 0, 200);
uint32_t COLOR_SECONDS = Adafruit_NeoPixel::Color(0, 200, 0);
uint32_t COLOR_MARKS = Adafruit_NeoPixel::Color(30, 30, 30);


Leds::Leds(LedConfig_t *config, Logger *logger) :
        _logger(logger),
        _config(config),
        _inner_ring(config->inner_ring_num_leds, config->inner_ring_pin, NEO_GRB + NEO_KHZ800),
        _outer_ring(config->outer_ring_num_leds, config->outer_ring_pin, NEO_GRB + NEO_KHZ800),
        _previous_second(0),
        _time_previous_second(0),
        _last_status_time_ms(0) {

    // Time 0..60 --> unit8_t (6bit)
    // num_pixel max 255 --> uint8_t (8bit)
    // shift --> 256
    const uint16_t shift_min = 256;
    _outer_ring_min_multiplier = (shift_min * static_cast<uint16_t>(_config->outer_ring_num_leds)) / 60;

    // Time 0..60000 --> unit16_t (16bit)
    // num_pixel max 255 --> uint8_t (8bit)
    // shift --> 256*256
    const uint32_t shift_sec = 256L*256L;
    _outer_ring_second_multiplier = (shift_sec * static_cast<uint32_t>(_config->outer_ring_num_leds)) / 60000;

}


void Leds::setTimeOnOuterRing(uint8_t hour, uint8_t min, uint16_t ms_in_min) {
    // hours
    _outer_ring.setPixelColor((static_cast<uint16_t>(hour) * 5 * _outer_ring_min_multiplier) >> 8, COLOR_HOURS);

    // minutes
    _outer_ring.setPixelColor((static_cast<uint16_t>(min) * _outer_ring_min_multiplier) >> 8, COLOR_MINUTES);

    // seconds
    _outer_ring.setPixelColor(static_cast<uint16_t>((static_cast<uint32_t>(ms_in_min) * _outer_ring_second_multiplier) >> 16), COLOR_SECONDS);
}

void Leds::setTimeOnInnerRing(uint8_t hour, uint8_t min, uint8_t sec) {
    // hours
    _inner_ring.setPixelColor(hour * 5, COLOR_HOURS);

    // minutes
    _inner_ring.setPixelColor(min, COLOR_MINUTES);

    // seconds
    _inner_ring.setPixelColor(sec, COLOR_SECONDS);
}


void Leds::setInnerRingDefaults(void) {
    _inner_ring.clear();

    // add orientation marks
    _inner_ring.setPixelColor(0, COLOR_MARKS);
    uint16_t one_forth = _config->inner_ring_num_leds >> 2;
    _inner_ring.setPixelColor(one_forth, COLOR_MARKS);
    _inner_ring.setPixelColor(_config->inner_ring_num_leds>>1, COLOR_MARKS);
    _inner_ring.setPixelColor(one_forth*3, COLOR_MARKS);
}

void Leds::setOuterRingDefaults(void) {
    _outer_ring.clear();

    // add orientation marks
    _outer_ring.setPixelColor(0, COLOR_MARKS);
    uint16_t one_forth = _config->outer_ring_num_leds >> 2;
    _outer_ring.setPixelColor(one_forth, COLOR_MARKS);
    _outer_ring.setPixelColor(_config->outer_ring_num_leds>>1, COLOR_MARKS);
    _outer_ring.setPixelColor(one_forth*3, COLOR_MARKS);
}

uint16_t Leds::estimateMillisecondsInMinute(uint8_t current_sec) {
    if (current_sec == _previous_second) {
        // return millis since last change
        uint32_t current_time = millis();

        return static_cast<uint16_t>(current_time - _time_previous_second) + current_sec * 1000;
    } else {
        _previous_second = current_sec;
        _time_previous_second = millis();

        return current_sec * 1000;
    }
}

void Leds::setup(void) {
    _outer_ring.begin();
    _inner_ring.begin();
    setInnerRingDefaults();
    setOuterRingDefaults();

    _time_previous_second = millis();
}


void Leds::update(const Time & current_time) {
    setInnerRingDefaults();
    setOuterRingDefaults();

    uint8_t hour = current_time.hr;
    if (hour > 12) {
        hour -= 12;
    }
    if (hour == 12) {
        hour = 0;
    }

    uint16_t ms = estimateMillisecondsInMinute(current_time.sec);

    setTimeOnOuterRing(hour, current_time.min, ms);
    _outer_ring.show();

    setTimeOnInnerRing(hour, current_time.min, current_time.sec);
    _inner_ring.show();

    uint16_t dt = static_cast<uint16_t>(millis() - _last_status_time_ms);
    if (dt > 5000) {
        _logger->log(Logger::DEBUG, Utils::padding0(current_time.hr) + ":" + Utils::padding0(current_time.min) + ":"
        + Utils::padding0(current_time.sec) + " (" + String(ms) + "ms)");
        _last_status_time_ms = millis();
    }
}
