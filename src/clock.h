#pragma once

#include "arduino-ds1302/DS1302.h"
#include "config.h"


class Logger;

class Clock {

public:
    explicit Clock(ClockConfig_t * config, Logger * logger);

    Time current_time;

    void setup(void);
    void update(void);
    void incrementMinutes(void);
    void incrementHours(void);

    void logCurrentTime(void);
    void logCurrentTime(const String & text);

private:
    Logger * _logger;
    DS1302 _rtc;
};
