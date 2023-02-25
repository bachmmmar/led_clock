#include <Arduino.h>
#include <time.h>
#include "clock.h"
#include "logger.h"
#include "utils.h"


Clock::Clock(ClockConfig_t * config, Logger * logger) :
        _logger(logger),
        _rtc(config->ce_pin, config->io_pin, config->sclk_pin),
        current_time(2023, 1, 1, 0, 0, 0, Time::kMonday),  // yr, mon, date, hr, min, sec, day
        _previous_second(0),
        _millis_compensation_div_64(282) {
}


void Clock::setup(void) {
    _rtc.halt(false);

    current_time = _rtc.time();
    _millis_last_call = millis();
    milliseconds_in_minute = 1000 * current_time.sec;
    logCurrentTime();
}

void Clock::update(void) {
    current_time = _rtc.time();
    if (current_time.hr > 24) {
        current_time.hr = 24;
        _logger->log(Logger::ERROR, "Hours larger than 24!");
    }
    if (current_time.min > 59) {
        current_time.min = 0;
        _logger->log(Logger::ERROR, "Minutes larger than 59!");
    }
    if (current_time.sec > 59) {
        current_time.sec = 59;
        _logger->log(Logger::ERROR, "Seconds larger than 59!");
    }

    estimateMillisecondsInMinute();
}

void Clock::incrementMinutes(void) {
    uint8_t new_minute = current_time.min + 1;
    if (new_minute > 59) {
        new_minute = 0;
    }

    _rtc.writeProtect(false);
    _rtc.halt(false);


    Time t(current_time.yr,
           current_time.mon,
           current_time.date,
           current_time.hr,
           new_minute,
           0,
           current_time.day);

    // Set the time and date on the chip.
    _rtc.time(t);

    _rtc.writeProtect(true);

    logCurrentTime("minutes incremented, now it is: ");
}

void Clock::incrementHours(void) {
    uint8_t new_hour = current_time.hr + 1;
    if (new_hour > 23) {
        new_hour = 0;
    }

    _rtc.writeProtect(false);
    _rtc.halt(false);


    Time t(current_time.yr,
           current_time.mon,
           current_time.date,
           new_hour,
           current_time.min,
           current_time.sec,
           current_time.day);

    // Set the time and date on the chip.
    _rtc.time(t);

    _rtc.writeProtect(true);

    logCurrentTime("hours incremented, now it is: ");
}


void Clock::logCurrentTime() {
    logCurrentTime("current time is: ");
}

void Clock::logCurrentTime(const String &text) {
    String str = String(text +
                        String(current_time.date) + "." + String(current_time.mon) + "." + String(current_time.yr) + "  " +
                        Utils::padding0(current_time.hr) + ":" + Utils::padding0(current_time.min) + ":" + Utils::padding0(current_time.sec));

    _logger->log(Logger::INFO, str);
}

void Clock::estimateMillisecondsInMinute() {
    if (current_time.min != _previous_minute) {
        _previous_minute = current_time.min;
        _millis_last_call = millis();
        _logger->log(Logger::INFO, "comp=" + String(_millis_compensation_div_64) + ", millis=" + String(milliseconds_in_minute));
        milliseconds_in_minute = static_cast<int32_t>(current_time.sec) * 1000;
    }

    uint32_t ms_since_last_call = millis() - _millis_last_call;
    _millis_last_call = millis();
    milliseconds_in_minute += static_cast<uint16_t>((ms_since_last_call * _millis_compensation_div_64) >> 6);

    if (current_time.sec != _previous_second) {
        _previous_second = current_time.sec;
        int32_t real_millis = static_cast<int32_t>(current_time.sec) * 1000;
        int32_t difference = real_millis - static_cast<int32_t>(milliseconds_in_minute);
        if (difference < 0) {
            _millis_compensation_div_64--;
        } else {
            _millis_compensation_div_64++;
        }
    }
}
