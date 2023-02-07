#ifndef  COMU_H
#define  COMU_H


#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#include "SYS_TIPOS_COMUNES.h"
#include "TRAMAS_COM.h"
#include "SYS_COM.h"



enum WIRE_TYPE    {MASTER_I2C,SLAVE_I2C,
                   RS_485_SW,RS_485_HW,
                   MASTER_485_SW,MASTER_485_HW,
                   SERIAL_SW,SERIAL_HW};

class SYS_COM
{
  
  public:
    SYS_COM(WIRE_TYPE Buffer_mode);                                                    //Contructora del objeto, define el hardware a usar.
    ~SYS_COM()   {};                                                                 
    bool      Begin(void * HWDriver,int buffer_tam);                                                    //Reserba memoria, devulve true si hay espacio.  
    bool      Arrive_new_trama(TRAMA_TYPE* Incommin_type,uint8_t MyDDr);                              //Detecta si llego un trama nueva.
    bool      Read_Data(uint8_t* buffer_destino, uint8_t buffer_size);
    bool      Send_Data(TRAMA_TYPE Data_type,uint8_t Add_Destino,uint8_t* buffer_Fuente);                                                               
  private: 
    bool      _Arrive_new_data(void);
    SoftwareSerial* _Pserial_SW;                                                          //Driver USART x SW
    HardwareSerial* _Pserial;                                                             //Driver USART     
    WIRE_TYPE _Com_type;
    TRAMA_DID _buffer_stream;
    int       _buffer_size;
};

#endif
