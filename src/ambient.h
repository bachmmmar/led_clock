#pragma once

#include "TSL2561-Arduino-Library/TSL2561.h"
#include "config.h"


class Logger;

class Ambient {

public:
    explicit Ambient(LightSensorConfig_t * config, Logger * logger);

    void setup(void);

    // return true if a new value has been calculated
    bool update(void);

    // Todo: Change back to uint16_t if range is verified
    uint32_t brightness_lux;


private:
    typedef union {
        uint32_t full_lumosity;
        struct {
            uint16_t full_ch0;
            uint16_t ir_ch1;
        };
    } raw_data_t;

    Logger * _logger;
    LightSensorConfig_t * _config;
    TSL2561 _sensor;
    uint32_t _last_update_time_ms;
    raw_data_t sensor_raw;

    void updateLux();
};
