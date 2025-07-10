#pragma once
#include <cstdint>

class DHT11_Sensor {
public:
    DHT11_Sensor(int gpio_pin);
    bool read(float& temperature, float& humidity);

private:
    int gpio_pin_;
    bool read_raw(uint8_t data[5]);
};