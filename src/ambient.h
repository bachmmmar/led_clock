#pragma once

#include "TSL2561-Arduino-Library/TSL2561.h"
#include "config.h"


class Logger;

class Ambient {

public:
    explicit Ambient(LightSensorConfig_t * config, Logger * logger);

    void setup(void);
    void update(void);

    uint16_t brightness;


private:
    Logger * _logger;
    LightSensorConfig_t * _config;
    TSL2561 _sensor;
    uint32_t _last_update_time_ms;
};
