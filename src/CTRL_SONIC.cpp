#include "CTRL_SONIC.h"



//Los pines del ultrasonico son configurables pero quemados, no por parametro.
#define SOUND_SPEED     343.0                       // Speed of sound in m/s (343m/s at 20°C with dry air)
#define tSENS           4                           // Total number of sensors
#define ECHO_INT_PIN    3                           // Pin to collect echo signal INT
#define TRIG_PIN1       5                          // Pin to triger SONIC1
#define TRIG_PIN2       6                          // Pin to triger SONIC2
#define TRIG_PIN3       7                          // Pin to triger SONIC3
#define TRIG_PIN4       8                          // Pin to triger SONIC4
#define PING_DELAY      500                         // How many milliseconds between each measurement (ping) ; best to keep > 5ms



volatile unsigned long _startEcho;           // Place to store start time (interrupt) //
volatile unsigned long _stopEcho;            // Place to store stop time (interrupt)  //
volatile int           _ultra_RSP;                                                    //
                                                    //

void  ISR_ECHO(void)
{
   _ultra_RSP=digitalRead(ECHO_INT_PIN);
  if (_ultra_RSP)  
  {
    _startEcho = micros();
  }
  else 
  {
    _stopEcho = micros();
  }
}



ULTRA_SONIC::ULTRA_SONIC(char SNCM)
{
 _SNCM_range=SNCM;
}

void ULTRA_SONIC::Sys_start_ULT(void)
{
    int8_t IRQ_Nun = digitalPinToInterrupt(ECHO_INT_PIN);

    _TrigerPIN=   (int*)malloc(sizeof(int)*tSENS);
    _distancesCm= (float*)malloc(sizeof(float)*tSENS);

    _TrigerPIN[0]=TRIG_PIN1;
    _TrigerPIN[1]=TRIG_PIN2;
    _TrigerPIN[2]=TRIG_PIN3;
    _TrigerPIN[3]=TRIG_PIN4;

    if(IRQ_Nun!=NOT_AN_INTERRUPT)
    {  
        pinMode(ECHO_INT_PIN, INPUT);

        for (int i = 0; i < tSENS; i++) 
            pinMode(_TrigerPIN[i], OUTPUT);   // Set trigger pin x as OUTPUT
        attachInterrupt(IRQ_Nun, ISR_ECHO, CHANGE );
    }       
}

//Hace un barrido con todos los sensores 
//retorna el Sensor_id del mas cerca
float ULTRA_SONIC::Scanid_nearby(int* Sensor_id)
{
    float Min_leng=100000.0;

    //Para cada sensor
    for (int Sensor_test = 0; Sensor_test < tSENS; Sensor_test++)
    {
        _distancesCm[Sensor_test] =Sensorid_cm(Sensor_test);   // in cm
        
        if((Min_leng>_distancesCm[Sensor_test]) && (_distancesCm[Sensor_test]!=0))
        {
            Min_leng=_distancesCm[Sensor_test];
            *Sensor_id=Sensor_test;
        }
    }
    return Min_leng;
}

float ULTRA_SONIC::Sensorid_cm(int Sensor_id)
{
    float result=100000.0;      //Lo inicializo a infinito
    unsigned long startWait;

    _startEcho = 0;  
    _stopEcho = 0;   
    //Triger sinal
    digitalWrite(_TrigerPIN[Sensor_id], HIGH);    
    delayMicroseconds(50);
    digitalWrite(_TrigerPIN[Sensor_id], LOW);
    startWait = micros();   

    while (_startEcho == 0 || _stopEcho == 0) 
    {
        //Espero maximo 60 ms
        if(micros()-startWait>60000)
        {
            _startEcho = 0;
            _stopEcho = 0;
            break;  
        }
    }

    noInterrupts();   
    //Si ambos (INicio y fin son validos)
    if(_startEcho!=0 && _stopEcho!=0)
       result = (_stopEcho - _startEcho) * (float)SOUND_SPEED / 1000.0 / 10.0 / 2.0;   // in cm
    interrupts();   // sei();

    return result;
}

bool  ULTRA_SONIC::Sensorid_warning(int Sensor_id)
{
    if(Sensor_id>=tSENS)
        return false;
        
    if (millis() - _lastPingMillis >= PING_DELAY)
    {
        if( Sensorid_cm(Sensor_id)<_SNCM_range) 
            return true;
    }         
    _lastPingMillis = millis();

    return false;
}

void  ULTRA_SONIC::Sensor_test(void)
{
    int id_Sensor;
    float Min_long;
    do
    {
        if (millis() - _lastPingMillis >= PING_DELAY)
        {
            Min_long=Scanid_nearby(&id_Sensor);
            Serial.println(   String(_distancesCm[0]) + " - " 
                            + String(_distancesCm[1]) + " - " 
                            + String(_distancesCm[2]) + " - " 
                            + String(_distancesCm[3]));
            Serial.print( "Minima a:");
            Serial.println(Min_long);
            Serial.print( "En iD:");
            Serial.println(id_Sensor);
            _lastPingMillis = millis();
        }
    }while(1);
}

bool    ULTRA_SONIC::Sonic_manager(DID_CMD test_CMD)
{
    static int Sensor_id=NO_DID_CMD;

    if (millis() - _lastPingMillis >= PING_DELAY)
    {
        switch(test_CMD)
        {
            case MOV_A:
                Sensor_id=M_ADELANTE;
            break;
            case MOV_R:
                Sensor_id=M_ATRAS;
            break;
            case MOV_D:
                Sensor_id=M_DERECHA;
            break;
            case MOV_I:
                Sensor_id=M_IZQUIERA;
            break;
            case MOV_S:
                Sensor_id=NO_DID_CMD;
                return false;
            break;
            default: 
            if(Sensor_id==NO_DID_CMD)// MOV_S o cualquier otro.
                return false;
            break;
        }
        return Sensorid_warning(Sensor_id);
    }
    return false;
}