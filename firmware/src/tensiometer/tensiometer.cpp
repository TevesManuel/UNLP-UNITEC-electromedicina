#include <tensiometer/tensiometer.hpp>

#include <Wire.h>

Tensiometer* Tensiometer::instance = nullptr;

void Tensiometer::disableI2C()
{
    Wire.begin(0x52);
    this->i2cEnabled = false;
}
void Tensiometer::enableI2C()
{
    Wire.begin(0x50);
    this->i2cEnabled = true;
}

Tensiometer::Tensiometer(u8 startPin)
{
    disableI2C();
    this->bytesCount = 0;
    this->count      = 0;
    this->startPin   = startPin;
}

void Tensiometer::begin()
{
    pinMode(this->startPin, OUTPUT);
}

void Tensiometer::startMeasure()
{
    //Start blood preasure measure
    digitalWrite(this->startPin, LOW);
    delay(200);
    digitalWrite(this->startPin, HIGH);
    delay(200);
}

void Tensiometer::update()
{        
    if(!this->i2cEnabled)
    {
        enableI2C();
        this->bytesCount = 0;
        this->readed     = false;
        this->count      = 0;
    }
    Wire.onReceive(this->staticReceiveEventI2C);
}

void Tensiometer::readTensiometer()
{
    disableI2C();

    this->systolicPressure  = ( (b12 & d2) >> 4 )*100 + ( (b14 & d2) >> 4 )*10 + (b14 & d1) ;
    this->diastolicPressure = ( b12 & d1 ) * 100 + ( (b16&d2) >> 4 ) * 10 + ( b16 & d1) ;
    this->beatsPerMinute    = b18;
    this->readed            = true;

    Serial.print("presion sistolica:   ");
    Serial.println(this->systolicPressure);
    Serial.print("presion diastolica:  ");
    Serial.println(this->diastolicPressure);
    Serial.print("Pulsaciones:  ");
    Serial.println(this->beatsPerMinute);
}

void Tensiometer::staticReceiveEventI2C(int howMany)
{
    if (instance)
    {
        instance->receiveEventI2C(howMany);
    }
}

void Tensiometer::receiveEventI2C(int howMany) 
{  
    if( millis()-this->lastI2CMessage > BLOOD_PREASURE_TIMEOUT )
    {
        Serial.println("[!] ERROR EN LA MEDICION ");
        this->count=0;
        disableI2C();
    }
    volatile byte i2cDataRx;
    while (0 < Wire.available()) 
    {   
        this->lastI2CMessage = millis();
        this->count++;
        i2cDataRx = Wire.read();           
        
        Serial.print("Byte ");
        Serial.print(this->count);
        Serial.print(" is:  ");
        Serial.println(i2cDataRx); 

        bool breakLoop = false;

        switch (this->count)
        {
            case 1:
                b1 = i2cDataRx;
                break;

            case 2:
                b2 = i2cDataRx;
                break;

            case 3:
                b3 = i2cDataRx;
                break;

            case 4:
                b4 = i2cDataRx;
                if(b1==10 && b2==11 && b3==2 && b4==3)
                {
                    this->count=0;
                    Serial.println("[!] ERROR CRITICO: reiniciar tensiometro");
                }
                break;

            case 12:
                b12 = i2cDataRx;
                break;

            case 14:
                b14 = i2cDataRx;
                break;

            case 16:
                b16 = i2cDataRx;
                break;

            case 18:
                b18 = i2cDataRx;
                break;

            case 20:
                Serial.println("[i] Se obtuvieron 20 BYTEs");
                breakLoop = true;
                break;
        }
        if(breakLoop)
        {
            readTensiometer();
            break;
        }
    }    
}