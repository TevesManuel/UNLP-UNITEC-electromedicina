#include <Arduino.h>

//DS18B20 deps
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//TENSIOMETER deps
#include <Wire.h>

#define motorPin  2
#define buttonPin 3
#define startPin  4
#define ONE_WIRE_PIN 6

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

//Is necesary?: no
//counter of the total bytes
int count=0;
//timer to detect bad measure
long int t1=0;

OneWire oneWireBus (ONE_WIRE_PIN);
DallasTemperature sensor(&oneWireBus);

int readTemperature();
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
void start_sample() {
    startMeasure=1;
    disableI2C();
}
void start_i2c() { 
    readI2C = 1;
}

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);     
    pinMode(motorPin, INPUT); 
    pinMode(startPin, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(buttonPin), start_sample , FALLING);
    attachInterrupt(digitalPinToInterrupt(motorPin), start_i2c , FALLING);
    
    Serial.begin(115200);   
}

void loop(){
    if ( startMeasure )
    {
        Serial.println("Se apreto boton");
        Serial.print("La temperatura es: ");
        Serial.print(readTemperature());
        Serial.println(" °C");

        //Start blood preasure measure?: yes
        delay(500);
        digitalWrite(startPin, LOW);
        delay(200);
        digitalWrite(startPin, HIGH);
        delay(2000);
        //Automate shutdown
        
        startMeasure = false;
    }

    if ( readI2C )
    {
        Serial.println("Preparando adquisicion...");
        //why?: 
        delay(4000);
        
        enableI2C();
        Wire.onReceive(receiveEvent);
        readTensiometer();

        readI2C = false;
    }

}

//return temperature measure with the ds18b20
int readTemperature()
{
    sensor.requestTemperatures();
    int Temp = sensor.getTempCByIndex(0);
    return Temp;
}

void readTensiometer()
{
    if( millis()-t1 > BLOOD_PREASURE_TIMEOUT )
    {
        Serial.println("[!] ERROR EN LA MEDICION ");
        count=0;
        disableI2C();
    }
    else if( count == 20 )
    {
        //All work
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
}

void receiveEvent(int howMany) 
{  
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
                break;
        }
    }    
}
