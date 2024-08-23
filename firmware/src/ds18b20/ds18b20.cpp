#include <ds18b20/ds18b20.hpp>

ds18b20::ds18b20(u8 pin)
{
    this->pin = pin;
}

void ds18b20::begin()
{
    this->oneWireBus = new OneWire(this->pin);
    this->sensor = new DallasTemperature(this->oneWireBus);
}

int ds18b20::readTemperature()
{
    this->sensor->requestTemperatures();
    return this->sensor->getTempCByIndex(0);
}