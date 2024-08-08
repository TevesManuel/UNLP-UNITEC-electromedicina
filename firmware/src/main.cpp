#include <Arduino.h>

// LIBRERIAS DS18B20
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//LIBRERIA TENSIOMETRO
#include <Wire.h>

//                           variables para la interrupcion
#define motorPin  (2)                        // MACRO motor  input pin
#define buttonPin (3)                        // MACRO button input pin
#define startPin  (4)                        // MACRO start output pin
int flag1=0, flag2=0;

//                           variables para los datos de presion
volatile byte d1=0x0F , d2=0xF0 , aux;         //var auxiliares para hacer las cuentas 
volatile byte b1,b2,b3,b4,b12,b14,b16,b18;                 //bytes de interes
volatile byte Pmax, Pmin;                      //valores de las presiones

//                           variables auxiliares para la funcion receiveEvent
volatile byte i2c_data_rx;                     // indica datos disponibles del bus i2c 
int count=0, flag=0;                           // contador para indicar total de bytes
long int t1=0;                                 // timer para detectar cuando una medicion salio mal
//*******************************
// variables para la temperatura
const int oneWirePin = 6;
//*******************************
// Temporizador
int t0 = 0;
volatile int h = 0;
unsigned long marcaTiempoDate = 0;
unsigned long tiempoRefreshDate = 1000;

OneWire oneWireBus (oneWirePin);
DallasTemperature sensor(&oneWireBus);

int ds18b20(); //prototipo de la funcion del sensor de temperatura
void Tensiometro(); //prototipo de la funcion del tensiometro

void start_i2c();
void start_sample();
void receiveEvent(int);

void setup(){
  pinMode(buttonPin, INPUT_PULLUP);     
  pinMode(motorPin, INPUT); 
  pinMode(startPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), start_sample , FALLING);
  attachInterrupt(digitalPinToInterrupt(motorPin), start_i2c , FALLING);
  Serial.begin(115200);                         // la direccion de la mem EEPROM es 0x50. Para poder leer el puerto arduino debe tener la misma direccion.             
}

void loop(){
  if ( flag1 == 1 )                             // condicion para iniciar la medicion
    {
      Serial.println("Se apreto boton");
      int Temperatura;
      Temperatura = ds18b20();
      Serial.print("La temperatura es: ");
      Serial.print(Temperatura); //imprime en el monitor serie
      Serial.println(" °C");
      delay(500);    
      digitalWrite(startPin, LOW);            // simula apretar el boton
      delay(200);
      digitalWrite(startPin, HIGH);
      delay(2000);
      flag1=0;
    }

    if ( flag2 == 1 )                             // condicion para poder leer el puerto I2C
    {
      Serial.println("Preparando adquisicion...");
      delay(4000);
      Wire.begin(0x50);
      Wire.onReceive(receiveEvent);             // inicializa la interrupcion para recibir data X i2c 
      Tensiometro();
      flag2=0;
    }

}

int ds18b20()
{
     sensor.requestTemperatures();
     int Temp = sensor.getTempCByIndex(0);
     return Temp; //Devuelve la medicion de temperatura
}

void Tensiometro()
{
    if ( flag==1 )
    {     
       if( count == 20 )
       {
         flag=0;  count=0;
         Wire.begin(0x52);                         // cambio de direccion de la memoria para que no moleste el pulso  NACK 

         Pmax = ( (b12 & d2) >> 4 )*100 + ( (b14 & d2) >> 4 )*10 + (b14 & d1) ;
         Pmin = ( b12 & d1 ) * 100 + ( (b16&d2) >> 4 ) * 10 + ( b16 & d1) ;

         Serial.print("presion sistolica:   ");
         Serial.println(Pmax);
         Serial.print("presion diastolica:  ");
         Serial.println(Pmin);
         Serial.print("Pulsaciones:  ");
         Serial.println(b18);
       }
       
       if( millis()-t1 > 7000 )
       {
         Serial.print(" ERROR EN LA MEDICION ");
         flag=0; count=0;
         Wire.begin(0x52);                         // cambio de direccion de la memoria para que no moleste el pulso  NACK       
       }      
       
    }
}
void start_sample() {
  flag1=1;
  Wire.begin(0x52);                         // cambio de direccion de la memoria para que no moleste el pulso  NACK
}


void start_i2c() { 
 flag2=1; 
}


void receiveEvent(int howMany) 
{  
 flag=1;
 while (0 < Wire.available()) 
  {   
      t1 = millis();
      count++;
      i2c_data_rx = Wire.read();           
      Serial.print("Byte ");   Serial.print(count); Serial.print(" es:  "); Serial.println(i2c_data_rx); 

      switch (count)
      {
          case 1:
          b1 = i2c_data_rx;
          break;
          
          case 2:
          b2 = i2c_data_rx;
          break;
          
          case 3:
          b3 = i2c_data_rx;
          break;
          
          case 4:
          b4 = i2c_data_rx;
          if(b1==10 && b2==11 && b3==2  && b4==3){ count=0; Serial.println("ERROR CRITICO: reiniciar tensiometro");}
          break;
          
          case 12:
          b12 = i2c_data_rx;
          break;
          
          case 14:
          b14 = i2c_data_rx;
          break;
          
          case 16:
          b16 = i2c_data_rx;
          break;
          
          case 18:
          b18 = i2c_data_rx;
          break;
          
          case 20:
          Serial.print("Se obtuvieron 20 BYTEs"); Serial.println();
          break;
      }
  }    
}
