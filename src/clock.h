#pragma once

#include "arduino-ds1302/DS1302.h"
#include "config.h"


class Logger;

class Clock {

public:
    explicit Clock(ClockConfig_t * config, Logger * logger);

    Time current_time;
    uint16_t milliseconds_in_minute;

    void setup(void);
    void update(void);
    void incrementMinutes(void);
    void incrementHours(void);

    void logCurrentTime(void);
    void logCurrentTime(const String & text);

private:
    void estimateMillisecondsInMinute();
    uint8_t _previous_second;
    uint8_t _previous_minute;
    uint32_t _millis_last_call;
    uint32_t _millis_compensation_div_64;
    Logger * _logger;
    DS1302 _rtc;
};
