#include <Arduino.h>
#include <time.h>
#include "clock.h"



Clock::Clock(ClockConfig_t * config, Logger * logger) :
        _logger(logger),
        _rtc(config->ce_pin, config->io_pin, config->sclk_pin),
        current_time(2023, 1, 1, 0, 0, 0, Time::kMonday) {  // yr, mon, date, hr, min, sec, day
}


void Clock::setup(void) {
    _rtc.halt(false);

    current_time = _rtc.time();

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

String padding0(uint8_t num) {
    if (num > 9) {
        return String(num);
    } else {
        return "0" + String(num);
    }
}

void Clock::logCurrentTime() {
    logCurrentTime("current time is: ");
}

void Clock::logCurrentTime(const String &text) {
    String str = String(text +
                        String(current_time.date) + "." + String(current_time.mon) + "." + String(current_time.yr) + "  " +
                        padding0(current_time.hr) + ":" + padding0(current_time.min) + ":" + padding0(current_time.sec));

    _logger->log(Logger::INFO, str);
}