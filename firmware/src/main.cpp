#include <Arduino.h>

#include <ds18b20/ds18b20.hpp>

//TENSIOMETER deps
#include <Wire.h>

#define motorPin  2
#define buttonPin 3
#define startPin  4
#define TEMPERATURE_SENSOR_PIN 6

#define BLOOD_PREASURE_TIMEOUT 7000

bool startMeasure=false;
bool readI2C=false;

//tensiometer vars
//aux vars for calculation
volatile byte d1=0x0F , d2=0xF0 , aux;
//target bytes
volatile byte b1,b2,b3,b4,b12,b14,b16,b18;
//blood preasure values
volatile byte Pmax, Pmin;
//counter of the total bytes
int count=0;
//timer to detect bad measure
long int t1=0;

ds18b20 temperatureSensor(TEMPERATURE_SENSOR_PIN);

void readTensiometer();

void start_i2c();
void start_sample();
void receiveEvent(int);

void disableI2C()
{
    Wire.begin(0x52);
}
void enableI2C()
{
    Wire.begin(0x50);
}
void startSample()
{
    startMeasure=1;
}
void startI2c()
{ 
    readI2C = 1;
}

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);     
    pinMode(motorPin, INPUT); 
    pinMode(startPin, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(buttonPin), startSample , FALLING);
    attachInterrupt(digitalPinToInterrupt(motorPin), startI2c , FALLING);
    
    temperatureSensor.begin();

    disableI2C();

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

        //Start blood preasure measure
        digitalWrite(startPin, LOW);
        delay(200);
        digitalWrite(startPin, HIGH);
        delay(200);
        //Automate shutdown
        
        startMeasure = false;
    }

    if ( readI2C )
    {
        enableI2C();
        Wire.onReceive(receiveEvent);

        readI2C = false;
    }

}


void receiveEvent(int howMany) 
{  
    if( millis()-t1 > BLOOD_PREASURE_TIMEOUT )
    {
        Serial.println("[!] ERROR EN LA MEDICION ");
        count=0;
        disableI2C();
    }
    volatile byte i2cDataRx;
    while (0 < Wire.available()) 
    {   
        t1 = millis();
        count++;
        i2cDataRx = Wire.read();           
        
        Serial.print("Byte ");
        Serial.print(count);
        Serial.print(" es:  ");
        Serial.println(i2cDataRx); 

        bool breakLoop = false;

        switch (count)
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
                    count=0;
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

void readTensiometer()
{
    count=0;
    disableI2C();

    Pmax = ( (b12 & d2) >> 4 )*100 + ( (b14 & d2) >> 4 )*10 + (b14 & d1) ;
    Pmin = ( b12 & d1 ) * 100 + ( (b16&d2) >> 4 ) * 10 + ( b16 & d1) ;

    Serial.print("presion sistolica:   ");
    Serial.println(Pmax);
    Serial.print("presion diastolica:  ");
    Serial.println(Pmin);
    Serial.print("Pulsaciones:  ");
    Serial.println(b18);
}