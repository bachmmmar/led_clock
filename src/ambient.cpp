#include "ambient.h"
#include "logger.h"


Ambient::Ambient(LightSensorConfig_t *config, Logger *logger) :
        _sensor(config->tsl_i2c_addr), _logger(logger), _config(config), _last_update_time_ms(0) {
}


void Ambient::setup() {
    // You can change the gain on the fly, to adapt to brighter/dimmer light situations
    _sensor.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)
    //_sensor.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)

    // Changing the integration time gives you a longer time over which to sense light
    // longer timelines are slower, but are good in very low light situtations!
    //_sensor.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)
    _sensor.setTiming(TSL2561_INTEGRATIONTIME_101MS);  // medium integration time (medium light)
    //_sensor.setTiming(TSL2561_INTEGRATIONTIME_402MS);  // longest integration time (dim light)

    updateLux();
}

void Ambient::updateLux() {
    sensor_raw.full_lumosity = _sensor.getFullLuminosity();
    brightness_lux = _sensor.calculateLux(sensor_raw.full_ch0, sensor_raw.ir_ch1);
}

bool Ambient::update() {
    uint16_t dt = static_cast<uint16_t>(millis() - _last_update_time_ms);
    if (dt < _config->update_time_ms) {
        return false;
    }
    _last_update_time_ms = millis();

    updateLux();

    _logger->log(Logger::INFO, String("Current brightness: ") + String(brightness_lux) +" lux, ");

    // TODO: Verify memory allignment in Union if it is acording to ch1 ch0 of example application
    //_logger->log(Logger::DEBUG, String("RAW 0x") + String(sensor_raw.full_lumosity, 16)
    //+ ", CH0 0x" + String(sensor_raw.full_ch0, 16) + ", CH1 0x" + String(sensor_raw.ir_ch1, 16));
    return true;
}
