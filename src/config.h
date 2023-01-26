#pragma once

typedef struct {
    uint8_t ce_pin;
    uint8_t io_pin;
    uint8_t sclk_pin;
} ClockConfig_t;


typedef struct {
    uint8_t inner_ring_pin;
    uint8_t outer_ring_pin;
    uint8_t inner_ring_num_leds;
    uint8_t outer_ring_num_leds;
} LedConfig_t;

typedef struct {
    uint8_t tsl_i2c_addr;  // float (addr 0x39), ground (0x29), high (0x49)
    uint16_t update_time_ms;  // time in milliseconds to update the brightness
} LightSensorConfig_t;
