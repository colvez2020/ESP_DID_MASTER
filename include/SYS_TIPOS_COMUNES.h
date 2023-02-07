#ifndef  TIPO_H
#define  TIPO_H

#include <Arduino.h>

#define DRR_POWER    0x30
#define DRR_BATUCV   0x40
#define DRR_TEST     0x35


//MOV_A,        Movimiento adelante
//MOV_R,        Movimiento reversa
//MOV_D,        Movimiento derecha 
//MOV_I,        Movimiento izquierda
//MOV_S,        Movimiento paro
//MOV_MS,       Movimiento medida 
//LUZ_ON,       Encender luces
//LUZ_OFF,      Apagar luces
//LUZ_ST,       Status luces
//LUZ_RST,      Reset luces
//CH_ST,        Cargador status 
//BAT_ST,       Bateria status 
//BAT_MS,       Bateria medida 
//BAT_RST,      Bateria reset
//NO_DID_CMD,   sin comando



enum DID_CMD      {MOV_A, MOV_R, MOV_D, MOV_I, MOV_S,MOV_MS, 
                   LUZ_ON, LUZ_OFF,LUZ_ST,LUZ_RST,
                   CH_ST,BAT_ST, BAT_MSA, BAT_MSB,BAT_RST, 
                   NO_DID_CMD};

enum SOURCE_CMD   {HMI_SOCMD,BLUE_SOCMD,NA_SOCMD};
enum TRAMA_TYPE   {CHG_ST_TYPE, BAT_ST_TYPE, BAT_MSR_TYPE, 
                   MOV_TYPE,LAMP_ST_TYPE, 
                   CMD_TYPE,ACK_TYPE,NN_TYPE};

enum STEPPER_ST   {ST_1, ST_2, ST_3, ST_4, ST_IDEL};
enum BATTERY_ST   {BT_CHARGER, BT_DISCHARGER, BT_IDEL,BT_FAIL,BT_NN};


//Define los datos de comunicion
struct CHARGE_STATUS
{
    uint16_t     Ciclos_Carga;       //Indica cuando ciclos completos de carga, estima vida util       
    uint16_t     STEPPER_ST_Status;     //Almacena la estado actual del STEP (ST_1, ST_2, ST_3, ST_4, ST_IDEL)
};
//4 bytes 816 ok


//Verifcar que esta estructura sea compatible con 32 btis
struct TIME_MEASURE
{
    uint16_t  Horas;             //Indica capacidad max, para hacer calculos
    uint16_t   Min;               //Indica capacidad usada, estimada.  
}; //4 bytes

//Se calcula en la clase charger
struct BATERRY_STATUS
{
    uint16_t   Cap_Disponible;
    uint16_t   Cap_Nominal;             //Indica capacidad max, para hacer calculos
    float      Cap_Usada;               //Indica capacidad usada, estimada.  
    float      Cap_Cargada;
};
//8 bytes 816 ok

struct BATERRY_MEASURE
{
    float      I_actual;            //Sensado de la corriente_actual.   4 bytes      
    float      V_actual;            //Sensado del voltaje
};
//4 byttes

struct LAMP_STATUS
{
    uint16_t   Estado_ON;          //Indica si la lucez estan encendidas.
    uint16_t   Horas_Uso;          //Estimado de horas de uso.
};
//4 bytes

struct MOV_STATUS
{
    uint16_t DID_CMD_Actual;
    uint16_t RPM_Setpoint;
    float    RPM_Medida_I;
    float    RPM_Medida_D;
    float    Giro;
};
//16 bytes 816 ok

struct DID_PERFIL
{ 
    uint16_t  Luz_time_Out; //Indica cuando la luz se deben apagar.
    uint16_t Speed;        //Indica la velocidad deseada
    uint16_t DID_CMD_User;     //Tipo char, almacena el comando 
};
//6 bytes.

struct ENCODER_INFO
{
    int32_t  dTick;
    int32_t  Tick_Actual;
}; //8bytes

#endif
