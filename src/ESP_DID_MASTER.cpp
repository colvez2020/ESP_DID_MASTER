#include <Arduino.h>
#include "SYS_COM.h"
#include "SYS_TIPOS_COMUNES.h"
#include "GATEWAY_MSG.h"
#include "CTRL_SONIC.h"
#include "TRAMAS_COM.h"

// Deybar estuvo aquí :P

ULTRA_SONIC Soinc_DID(25);                    //Controla lectura de los ultrasonicos
CONTROL_MSG Main_msg;                         //Crea 2 streams Serial (HDMI SW y BLUE HW) + MASTES I2C
TRAMA_DID   CMD_test_buffer;
DID_PERFIL  CMD_Test;
uint8_t     CMD_Trama_test[40];

int attempt;
bool Comu_status = false;
bool Get_RSP_OK= false;

void setup() 
{
  //Inicia streams  
  Main_msg.Sys_start_MSG();
  //Configura interupcion SONIC y puertos
  CMD_test_buffer.Begin(N_BUFFER,32); //
  CMD_Test.DID_CMD_User=LUZ_ON;
  CMD_Test.Luz_time_Out=50;
  CMD_Test.Speed=70;
  CMD_test_buffer.Make_OUT_buffer(CMD_TYPE,0x40,CMD_Trama_test,(uint8_t *)&CMD_Test);
  Soinc_DID.Sys_start_ULT();
  Serial.print("CHARGE_STATUS->");
  Serial.println((uint8_t)sizeof(CHARGE_STATUS));
  Serial.print("TIME_MEASURE->");
  Serial.println((uint8_t)sizeof(TIME_MEASURE));
  Serial.print("BATERRY_STATUS->");
  Serial.println((uint8_t)sizeof(BATERRY_STATUS));
  Serial.print("BATERRY_MEASURE->");
  Serial.println((uint8_t)sizeof(BATERRY_MEASURE));
  Serial.print("LAMP_STATUS->");
  Serial.println((uint8_t)sizeof(LAMP_STATUS));
  Serial.print("MOV_STATUS->");
  Serial.println((uint8_t)sizeof(MOV_STATUS));
  Serial.print("DID_PERFIL->");
  Serial.println((uint8_t)sizeof(DID_PERFIL));
  Serial.print("ENCODER_INFO->");
  Serial.println((uint8_t)sizeof(ENCODER_INFO));
  Serial.println("Master_Gateway");
  Serial.println("Master_Gateway");
  Serial.println("Master_Gateway");
}


void loop() 
{
  //Llego una trama valida? con comando 
  if(Main_msg.Incoming_CMD())
  {
    //Serial.println("Trama_OK");
    attempt=0;
    //Envio a la tarjeta respectiva

    if(Main_msg.Dispatcher_CMD())
    //  Get_RSP_OK=Main_msg.Get_RSP();
    do
    {
      attempt++;
      Get_RSP_OK=Main_msg.Get_RSP();
      if(attempt==2000)
      {
        Serial.println("Sin_respuesta");
        Comu_status=false;
        break; 
      }
    }while (Get_RSP_OK==false);

    if(Get_RSP_OK==true)
    { 
      //Main_msg.Return_RSP();
      Serial.println("COMU_COMPLETE_OK!");
    }
  }
}


  

/*

CMD_test_buffer.Begin(N_BUFFER,32); //
  CMD_Test.DID_CMD_User=MOV_S;
  CMD_Test.Luz_time_Out=40;
  CMD_Test.Speed=59;
  CMD_test_buffer.Make_OUT_buffer(CMD_TYPE,0x30,CMD_Trama_test,(uint8_t *)&CMD_Test);

  if(Main_msg.Incoming_CMD())
  {
    Serial.println("Trama_OK");
    //Limpio bandara, si hay 3 intentos y no hay respuesta pasa a false.
    Comu_status=true;
    attempt=0;
    //Envio a la tarjeta respectiva
    //Main_msg.Dispatcher_CMD(); 
    do
    {
      attempt++;
      Get_RSP_OK=Main_msg.Get_RSP();
      if(attempt==2000)
      {
        Serial.println("Sin_respuesta");
        Comu_status=false;
        break; 
      }
    }while (Get_RSP_OK==false);

    if(Get_RSP_OK==true)
    { 
      Main_msg.Return_RSP();
      Serial.println("COMU_COMPLETE_OK!");
    }
  }   
  
  //Si llego un cmd de movimiento
  if(Soinc_DID.Sonic_manager((DID_CMD)Main_msg.Get_CMD())==true)
  {
    //Detecto un distancia menor del sensor para ese movimiento
    Serial.println("choque");
  }
  
  //Verifico que ambos Esclavos esten en linea
  //delay(10);
*/