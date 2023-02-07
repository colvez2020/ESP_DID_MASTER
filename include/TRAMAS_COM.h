#ifndef  TRAMA_SYS
#define  TRAMA_SYS


#include <Arduino.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

#include "SYS_TIPOS_COMUNES.h"
#include "SYS_COM.h"


#define OCTO_BUFFER_SIZE     32
#define TOKEN_ACK            0x77
#define TOKEN_INCIO          0x31
#define TOKEN_FIN            0x61


enum BUF_TYPE    {N_BUFFER,HW_BUFFER};


class TRAMA_DID
{
  
  public:
    TRAMA_DID(void);                                                       //Contructora del objeto, selecccion entre volatile o uint8_t
    ~TRAMA_DID()   {};                                                                 
    bool       Begin(char Buffer_mode,int buffer_tam);                                                    //Reserba memoria, devulve true si hay espacio.
    void       Print_buffer(void);
    void       Clear_index(void);
    void       Clear_buffer(void);
    uint8_t    Make_OUT_buffer( TRAMA_TYPE Data_type,uint8_t ADD_Destino,
                             uint8_t* pbuffer_Trama,uint8_t* pbuffer_Datos);           //Crea trama en el  buffer, a partir de Data_type y buffer_Fuente
    bool       Load_buffer(uint8_t Incoming_Data);                                     //Adiciona un byte al buffer y verifica que es una trama.
    bool       Read_buffer(uint8_t* pbuffer_destino, uint8_t destino_size);             //Lee la trama -> y copia los datos a buffer_destino.                                       
    TRAMA_TYPE Read_trama_type(void); 
    uint8_t    Read_trama_DDR(void); 
    bool       Is_incio_trama(uint8_t info_data);                                          //Detecta inicio de trama
    bool       Is_fin_trama(uint8_t info_data);                                            //Detecta fin de trama
  private:     
    bool       _Is_data_type(uint8_t info_data);                                            //Detecta trama type
    bool       _Is_trama(void);                                                             //Detecto trama invalida 
    uint8_t    _Data_type_size(uint8_t info_data);                                          //Devuelve tamaño del dato
    uint8_t    _CalCHK(uint8_t *data, uint8_t count );                                      //Calcula CHK.
    char       _Buffer_mode;
    int        _buffer_tam;                                                                           //
    bool       _trama_incio;                                                            //Indica llego inicio de trama
    bool       _trama_fin;                                                              //Indica llego el fin de trama
    int        _in_data_index=0;
    uint8_t*   _Strem_INFO;
    volatile uint8_t*   _wire_data_INFO;                              //Datos recibidos         por int
    
};

#endif
