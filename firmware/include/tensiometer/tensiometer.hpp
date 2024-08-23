#ifndef UNITEC_LIB_TENSIOMETER
#define UNITEC_LIB_TENSIOMETER

#include <Arduino.h>

#define BLOOD_PREASURE_TIMEOUT 7000

class Tensiometer
{
    private:
        //timer to detect bad measure
        long int lastI2CMessage=0;
        //counter of the total bytes
        int count;
        
        bool i2cEnabled;
        void disableI2C();
        void enableI2C();

        //aux vars for calculation
        volatile byte d1=0x0F , d2=0xF0 , aux;
        //target bytes
        volatile byte b1,b2,b3,b4,b12,b14,b16,b18;
        
        int bytesCount;

        u8 startPin;

        static Tensiometer* instance;
        static void staticReceiveEventI2C(int howMany);
        void receiveEventI2C(int howMany);
        void readTensiometer();
    public:
        bool readed;
        //blood preasure values
        volatile byte systolicPressure, diastolicPressure, beatsPerMinute;
        Tensiometer(u8 startPin);
        void begin();
        void startMeasure();
        void update();
};

#endif