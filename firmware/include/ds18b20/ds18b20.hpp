#ifndef UNITEC_LIB_DS18B20
#define UNITEC_LIB_DS18B20

#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdio.h>

class ds18b20
{
    private:    
        OneWire * oneWireBus;
        DallasTemperature * sensor;
        u8 pin;
    public:
        ds18b20(u8 pin);
        void begin();        
        int readTemperature();
};

#endif