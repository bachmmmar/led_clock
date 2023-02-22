#include "ambient.h"
#include "logger.h"


Ambient::Ambient(LightSensorConfig_t *config, Logger *logger) :
        _sensor(config->tsl_i2c_addr), _logger(logger), _config(config), _state(idle), _last_update_time_ms(0) {
}


void Ambient::setup() {
    _sensor.begin();

    // You can change the gain on the fly, to adapt to brighter/dimmer light situations
    _sensor.setGain(TSL2561_GAIN_0X);         // set no gain (for bright situtations)

    // Changing the integration time gives you a longer time over which to sense light
    _sensor.setTiming(_config->integration_time);  // medium integration time (medium light)
}

uint16_t Ambient::getIntegrationTimeInMs() const {
    switch (_config->integration_time) {
        case TSL2561_INTEGRATIONTIME_13MS:
            return 14;
        case TSL2561_INTEGRATIONTIME_101MS:
            return 102;
        default:
            return 403;
    }
}

void Ambient::doSampling() {
    _sensor.enable();
}

void Ambient::stopSampling() {
    _sensor.disable();
}

void Ambient::doReadout() {
    sensor_raw.full_lumosity = _sensor.read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN1_LOW);
    sensor_raw.full_lumosity <<= 16;
    sensor_raw.full_lumosity |= _sensor.read16(TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REGISTER_CHAN0_LOW);
}

bool Ambient::update() {
    uint16_t time_in_state = static_cast<uint16_t>(millis() - _last_update_time_ms);

    switch (_state) {
        case idle:
            if (time_in_state > _config->update_time_ms) {
                doSampling();
                _state = sampling;
                _last_update_time_ms = millis();
            }
            return false;
        case sampling:
            if (time_in_state > getIntegrationTimeInMs()) {
                doReadout();
                _state = stop_sampling;
                _last_update_time_ms = millis();
            }
            return false;
        case stop_sampling:
            stopSampling();
            _state = calculating;
            return false;
        case calculating:
            brightness_lux = _sensor.calculateLux(sensor_raw.full_ch0, sensor_raw.ir_ch1);
            _state = returning_true;
            return false;
        case returning_true:
            _state = idle;
            _last_update_time_ms = millis();
    }

    _logger->log(Logger::INFO, String("Current brightness: ") + String(brightness_lux) +" lux, ");
    return true;
}
