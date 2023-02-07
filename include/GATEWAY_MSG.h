#ifndef CTRL_MSG
#define CTRL_MSG

#include "SYS_TIPOS_COMUNES.h"



//Se encarga del envio y recepcion de datos entre los diferentes perifecos
//Solo se ejecuta en el master. El master no hace control alguno
//Solo pide y recibe informacion, del periferico BAT_UVC y de MOTOR_PW
class CONTROL_MSG
{
    public:
        CONTROL_MSG(void) {}; 
        ~CONTROL_MSG() {};
        void Sys_start_MSG(void);
        bool Check_device(void) {return true;};
        bool Incoming_CMD(void);            //Detecta si llego un comando valido,             (USART)->_wire_data_IN 
        bool Dispatcher_CMD(void);          //Copia info en _wire_data_IN (USART)          a  _wire_data_OUT (I2C_M-> I2C_S)
        bool Get_RSP(void);                 //Solicita por I2C los bytes de respuesta.
        bool Return_RSP(void);              //Copia info en _wire_data_IN (I2C_S -> I2C_M) a  _wire_data_OUT ->(USART)
        uint16_t Get_CMD(void);
    private:
        char            _ACK;           //1.
        SOURCE_CMD      _Source;        //1.Indica para donde debe ir la respuesta
        DID_PERFIL      _In_CMD;        //5.Indica cual es el comando actual.
        CHARGE_STATUS   _Ciclos_BAT;    //4.Contiene los ciclos de carga
        BATERRY_STATUS  _Capacidad_BAT; //4.Contiene la capacidad restante 
        BATERRY_MEASURE _IV_BATAB;       //9.Contiene la media de I/V y modo de operacion.
        LAMP_STATUS     _State_LAMP;    //3.Horas de uso y estado de la luces
        MOV_STATUS      _State_MOV;     //11.Mov actual, RPMs, reversa y giros. Total = 38 bytes
};

#endif