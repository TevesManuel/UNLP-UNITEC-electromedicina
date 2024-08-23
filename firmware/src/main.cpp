#include <Arduino.h>

#include <ds18b20/ds18b20.hpp>
#include <tensiometer/tensiometer.hpp>

#define motorPin  2
#define buttonPin 3
#define startPin  4
#define TEMPERATURE_SENSOR_PIN 6

bool startMeasure=false;
bool readI2C=false;

ds18b20 temperatureSensor(TEMPERATURE_SENSOR_PIN);
Tensiometer tensiometerSensor;

void startSample()
{
    startMeasure=true;
}
void startI2c()
{ 
    readI2C = true;
}

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);     
    pinMode(motorPin, INPUT); 

    attachInterrupt(digitalPinToInterrupt(buttonPin), startSample , FALLING);
    attachInterrupt(digitalPinToInterrupt(motorPin), startI2c , FALLING);
    
    temperatureSensor.begin();

    Serial.begin(115200);   
}

void loop()
{
    if ( startMeasure )
    {
        Serial.println("Se apreto boton");
        Serial.print("La temperatura es: ");
        Serial.print(temperatureSensor.readTemperature());
        Serial.println(" °C");
        
        startMeasure = false;
    }

    if ( readI2C )
    {
        tensiometerSensor.update();
        if(tensiometerSensor.readed)
        {
            Serial.print("presion sistolica:   ");
            Serial.println(tensiometerSensor.systolicPressure);
            Serial.print("presion diastolica:  ");
            Serial.println(tensiometerSensor.diastolicPressure);
            Serial.print("Pulsaciones:  ");
            Serial.println(tensiometerSensor.beatsPerMinute);
            readI2C = false;
        }
    }
}

