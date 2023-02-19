#include "leds.h"
#include "arduino-ds1302/DS1302.h"
#include "utils.h"



Leds::Leds(LedConfig_t *config, Logger *logger) :
        BASE_COLOR({.hours=Adafruit_NeoPixel::Color(255, 0, 0),
                    .minutes=Adafruit_NeoPixel::Color(0, 0, 255),
                    .seconds=Adafruit_NeoPixel::Color(0, 255, 0),
                    .marks=Adafruit_NeoPixel::Color(30, 30, 30)}),
        MIN_COLOR({.hours=Adafruit_NeoPixel::Color(5, 0, 0),
                   .minutes=Adafruit_NeoPixel::Color(0, 0, 7),
                   .seconds=Adafruit_NeoPixel::Color(0, 5, 0),
                   .marks=Adafruit_NeoPixel::Color(2, 2, 2)}),
        _dimmed_color(BASE_COLOR),
        _logger(logger),
        _config(config),
        _inner_ring(config->inner_ring_num_leds, config->inner_ring_pin, NEO_GRB + NEO_KHZ800),
        _outer_ring(config->outer_ring_num_leds, config->outer_ring_pin, NEO_GRB + NEO_KHZ800),
        _previous_second(0),
        _time_previous_second(0),
        _last_status_time_ms(0),
        _rotation_pixels({initRotationPxl(0, config->inner_ring_num_leds, 255, 0, 0),
                          initRotationPxl(3, config->inner_ring_num_leds, 0, 255, 0),
                          initRotationPxl(6, config->inner_ring_num_leds, 0, 0, 255),
                          initRotationPxl(9, config->inner_ring_num_leds, 100, 100, 100),
                          initRotationPxl(12, config->inner_ring_num_leds, 180, 0, 180),
                          initRotationPxl(15, config->inner_ring_num_leds, 0, 180, 180),
                          initRotationPxl(18, config->inner_ring_num_leds, 180, 180, 0)
                          }) {

    // Time 0..60 --> unit8_t (6bit)
    // num_pixel max 255 --> uint8_t (8bit)
    // shift --> 256
    const uint16_t shift_min = 256;
    _outer_ring_min_multiplier = (shift_min * static_cast<uint16_t>(_config->outer_ring_num_leds)) / 60;
    _inner_ring_min_multiplier = (shift_min * static_cast<uint16_t>(_config->inner_ring_num_leds)) / 60;

    // Time 0..60000 --> unit16_t (16bit)
    // num_pixel max 255 --> uint8_t (8bit)
    // shift --> 256*256
    const uint32_t shift_sec = 256L*256L;
    _outer_ring_second_multiplier = (shift_sec * static_cast<uint32_t>(_config->outer_ring_num_leds)) / 60000;
    _inner_ring_second_multiplier = (shift_sec * static_cast<uint32_t>(_config->inner_ring_num_leds)) / 60000;
}

Leds::effect_rotation_pxl_t Leds::initRotationPxl(uint8_t pos, uint8_t max_pixel, uint8_t r, uint8_t g, uint8_t b) {
    effect_rotation_pxl_t pxl;
    pxl.r = r;
    pxl.g = g;
    pxl.b = b;
    pxl.direction = 1;
    pxl.current_pixel = (pos + 1) % max_pixel;
    pxl.last_pixel = pos;
    return pxl;
}

void Leds::updateRotationPxl(effect_rotation_pxl_t * pxl) {
    _inner_ring.setPixelColor(pxl->last_pixel, _inner_ring.Color(0, 0, 0));
    _inner_ring.setPixelColor(pxl->current_pixel, _inner_ring.Color(pxl->r, pxl->g, pxl->b));
    pxl->last_pixel = pxl->current_pixel;
    pxl->current_pixel = (pxl->current_pixel + 1);
    if (pxl->current_pixel == _config->inner_ring_num_leds) {
        pxl->current_pixel = 0;
    }
}

void Leds::setTimeOnOuterRing(uint8_t hour, uint8_t min, uint16_t ms_in_min) {
    // hours
    _outer_ring.setPixelColor((static_cast<uint16_t>(hour) * 5 * _outer_ring_min_multiplier) >> 8, _dimmed_color.hours);

    // minutes
    _outer_ring.setPixelColor((static_cast<uint16_t>(min) * _outer_ring_min_multiplier) >> 8, _dimmed_color.minutes);

    // seconds
    _outer_ring.setPixelColor(static_cast<uint16_t>((static_cast<uint32_t>(ms_in_min) * _outer_ring_second_multiplier) >> 16), _dimmed_color.seconds);
}

void Leds::setTimeOnInnerRing(uint8_t hour, uint8_t min, uint16_t ms_in_min) {
    // hours
    _inner_ring.setPixelColor((static_cast<uint16_t>(hour) * 5 * _inner_ring_min_multiplier) >> 8, _dimmed_color.hours);

    // minutes
    _inner_ring.setPixelColor((static_cast<uint16_t>(min) * _inner_ring_min_multiplier) >> 8, _dimmed_color.minutes);

    // seconds
    _inner_ring.setPixelColor(static_cast<uint16_t>((static_cast<uint32_t>(ms_in_min) * _inner_ring_second_multiplier) >> 16), _dimmed_color.seconds);
}


void Leds::setInnerRingDefaults(void) {
    _inner_ring.clear();

    // add orientation marks
    _inner_ring.setPixelColor(0, _dimmed_color.marks);
    uint16_t one_forth = _config->inner_ring_num_leds >> 2;
    _inner_ring.setPixelColor(one_forth, _dimmed_color.marks);
    _inner_ring.setPixelColor(_config->inner_ring_num_leds>>1, _dimmed_color.marks);
    _inner_ring.setPixelColor(one_forth*3, _dimmed_color.marks);
}

void Leds::setOuterRingDefaults(void) {
    _outer_ring.clear();

    // add orientation marks
    _outer_ring.setPixelColor(0, _dimmed_color.marks);
    uint16_t one_forth = _config->outer_ring_num_leds >> 2;
    _outer_ring.setPixelColor(one_forth, _dimmed_color.marks);
    _outer_ring.setPixelColor(_config->outer_ring_num_leds>>1, _dimmed_color.marks);
    _outer_ring.setPixelColor(one_forth*3, _dimmed_color.marks);
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

    fullHourSpinn(6);
}

uint32_t Leds::scaleColor(const color_t & input, const color_t & min, uint16_t scale) {
    color_t out;
    out.splitted.r = max(min.splitted.r, (static_cast<uint16_t>(input.splitted.r) * scale) >> 8);
    out.splitted.g = max(min.splitted.g, (static_cast<uint16_t>(input.splitted.g) * scale) >> 8);
    out.splitted.b = max(min.splitted.b, (static_cast<uint16_t>(input.splitted.b) * scale) >> 8);
    //_logger->log(Logger::DEBUG, "Scale:" + String(scale) + ", rin:" + String(input.splitted.r) + ", rout:" + String(out.splitted.r));
    return out.color;
}

void Leds::updateBrightness(const uint32_t & lux) {
    // lux should be 40000 in max. 1000 means very bright day
    uint16_t scale = min(255L, lux >> 1);

    color_t test {.color = _dimmed_color.marks};
    //_logger->log(Logger::DEBUG, "Scale:" + String(scale) + ", r:" + String(test.splitted.r) + ", g:" + String(test.splitted.g)+ ", b:" + String(test.splitted.b));

    _dimmed_color.hours = scaleColor(color_t {.color=BASE_COLOR.hours},color_t {.color=MIN_COLOR.hours}, scale);
    _dimmed_color.minutes = scaleColor(color_t {.color=BASE_COLOR.minutes}, color_t {.color=MIN_COLOR.minutes}, scale);
    _dimmed_color.seconds = scaleColor(color_t {.color=BASE_COLOR.seconds}, color_t {.color=MIN_COLOR.seconds}, scale);
    _dimmed_color.marks = scaleColor(color_t {.color=BASE_COLOR.marks}, color_t {.color=MIN_COLOR.marks}, scale);
}

void Leds::fixingClockRotation(uint8_t *hour, uint8_t *min, uint16_t *ms_in_min) {
    if (*hour > 6) {
        *hour -= 6;
    } else {
        *hour += 6;
    }

    if (*min > 30) {
        *min -= 30;
    } else {
        *min += 30;
    }

    if (*ms_in_min > 30000) {
        *ms_in_min -= 30000;
    } else {
        *ms_in_min += 30000;
    }
}

void Leds::update(const Time & current_time) {
    if ((current_time.min == 0) && (current_time.sec == 0)) {
        fullHourSpinn(current_time.hr);
    }
    showTime(current_time);
}

void Leds::fullHourSpinn(uint8_t rotations) {
    _inner_ring.clear();
    for (uint16_t n = rotations*_config->inner_ring_num_leds; n > 0; n--) {
        for (auto &pxl: _rotation_pixels) {
            updateRotationPxl(&pxl);
        }
        _inner_ring.show();
    }
}

void Leds::showTime(const Time & current_time) {
    setInnerRingDefaults();
    setOuterRingDefaults();

    uint8_t hour = current_time.hr;
    if (hour > 12) {
        hour -= 12;
    }
    if (hour == 12) {
        hour = 0;
    }

    uint8_t min = current_time.min;
    uint16_t ms = estimateMillisecondsInMinute(current_time.sec);
    fixingClockRotation(&hour, &min, &ms);

    setTimeOnOuterRing(hour, current_time.min, ms);
    _outer_ring.show();

    setTimeOnInnerRing(hour, min, ms);
    _inner_ring.show();

    uint16_t dt = static_cast<uint16_t>(millis() - _last_status_time_ms);
    if (dt > 5000) {
        _logger->log(Logger::DEBUG, Utils::padding0(current_time.hr) + ":" + Utils::padding0(current_time.min) + ":"
        + Utils::padding0(current_time.sec) + " (" + String(ms) + "ms)");
        _last_status_time_ms = millis();
    }
}
