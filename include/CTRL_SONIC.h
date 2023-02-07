#ifndef CTRL_SONIC
#define CTRL_SONIC

#include <Arduino.h>
#include <string.h>
#include "CTRL_SONIC.h"
#include "SYS_TIPOS_COMUNES.h"


//Etiquetas para cada sensor ultrasonico
enum SONIC_ID{SONIC_ID_1,SONIC_ID_2,SONIC_ID_3,SONIC_ID_4};

#define M_IZQUIERA           SONIC_ID_1
#define M_ADELANTE           SONIC_ID_2
#define M_DERECHA            SONIC_ID_3
#define M_ATRAS              SONIC_ID_4
#define M_PARO               5

/////////////////////////////////WARNING//////////////////////////////////////////////
//Se esta manejando una INTERRUPCION solo se debe usar una istancia de la clase     //
//si no se hace caso a esta advertencia la clase arroja resultados inesperados      //
/////////////////////////////////WARNING//////////////////////////////////////////////



class ULTRA_SONIC
{
  public:
    ULTRA_SONIC(char SNCM);
    ~ULTRA_SONIC() {};
    void  Sys_start_ULT(void);
    void  Sensor_test(void);
    float Scanid_nearby(int* Sensor_id);
    float Sensorid_cm(int Sensor_id);
    bool  Sensorid_warning(int Sensor_id);
    String Sensor_print(int Sensor_index);
    bool   Sonic_manager(DID_CMD test_CMD);
  private:  
    char          _SNCM_range;                          //rango de alerta
    int*          _TrigerPIN;                           //Almacena la def de los pines de pulling
    float*        _distancesCm;                         // Distances in cm
    unsigned long _lastPingMillis;                      // Refencia de tiempo
    //////////////////////////////////////////////////////////////////////////////////////////////
                                                    //        
    //////////////////////////////////////////////////////////////////////////////////////////////
};

#endif