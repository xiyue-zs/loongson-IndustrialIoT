#include "dht11_driver.hpp"
#include <wiringPi.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

DHT11_Sensor::DHT11_Sensor(int gpio_pin) : gpio_pin_(gpio_pin) {
    wiringPiSetupGpio();
    pinMode(gpio_pin_, OUTPUT);
    digitalWrite(gpio_pin_, HIGH);
}

bool DHT11_Sensor::read_raw(uint8_t data[5]) {
    // 发送开始信号
    digitalWrite(gpio_pin_, LOW);
    delay(18);
    digitalWrite(gpio_pin_, HIGH);
    delayMicroseconds(40);
    pinMode(gpio_pin_, INPUT);

    // 等待响应
    if (digitalRead(gpio_pin_) return false;
    delayMicroseconds(80);
    if (!digitalRead(gpio_pin_)) return false;
    delayMicroseconds(80);

    // 读取40位数据
    for (int i = 0; i < 40; i++) {
        while (digitalRead(gpio_pin_) == LOW);
        delayMicroseconds(30);
        data[i/8] <<= 1;
        if (digitalRead(gpio_pin_)) data[i/8] |= 1;
        while (digitalRead(gpio_pin_) == HIGH);
    }

    return true;
}

bool DHT11_Sensor::read(float& temperature, float& humidity) {
    uint8_t data[5] = {0};
    if (!read_raw(data)) return false;

    // 校验和验证
    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        std::cerr << "DHT11 checksum error" << std::endl;
        return false;
    }

    humidity = data[0];
    temperature = data[2];
    return true;
}