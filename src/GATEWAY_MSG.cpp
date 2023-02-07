#include "SYS_COM.h"
#include "GATEWAY_MSG.h"

#define RS_485_TX_PIN    A5
#define RS_485_RX_PIN    A4
#define HMI_TX_PIN       4
#define HMI_RX_PIN       9


SoftwareSerial USART_RS485_SW(RS_485_RX_PIN,RS_485_TX_PIN);                  //RX,TX
SoftwareSerial USART_HMI_SW  (HMI_RX_PIN,HMI_TX_PIN);                  
HardwareSerial &USART_BLUE_HW  = Serial;           

SYS_COM      DID_BLUE_CMD  (SERIAL_HW);     //BLEETOOTH <-> DID_MASTER
SYS_COM      DID_HMI_CMD   (SERIAL_SW);     //HMI  <-> DID_MASTER
SYS_COM      DID_MASTER_485(MASTER_485_SW); //DID_MASTER



void CONTROL_MSG::Sys_start_MSG(void)
{
    DID_BLUE_CMD.Begin((void*)&USART_BLUE_HW,OCTO_BUFFER_SIZE);
    DID_HMI_CMD.Begin((void*)&USART_HMI_SW,OCTO_BUFFER_SIZE);
    DID_MASTER_485.Begin((void*)&USART_RS485_SW,OCTO_BUFFER_SIZE);
    _In_CMD.DID_CMD_User=NO_DID_CMD;
}

//Detecta si llego un comando valido,             (USART)->_wire_data_IN 
bool CONTROL_MSG::Incoming_CMD(void)
{
    TRAMA_TYPE IN_data_type;
   
    _Source=NA_SOCMD;
    if(DID_BLUE_CMD.Arrive_new_trama(&IN_data_type,0x00))  //Sale con trama ok
    {
        
        if(IN_data_type==CMD_TYPE)
        {
            //No importa direccion porque estoy lleyendo lo que llego por SERIAL!
            DID_BLUE_CMD.Read_Data((uint8_t*)&_In_CMD,sizeof(DID_PERFIL));
            _Source=BLUE_SOCMD;
            Serial.println("llego_CMD_BLUE!!");
            /*Serial.println("Llego_Info x I2C:");      
            Serial.print("_In_CMD.User_CMD-> ");      
            Serial.println(_In_CMD.DID_CMD_User,HEX);
            Serial.print("_In_CMD.Speed-> ");      
            Serial.println(_In_CMD.Speed,HEX);
            Serial.print("_In_CMD.Luz_time_Out-> ");      
            Serial.println(_In_CMD.Luz_time_Out,HEX); */
            return true;
        }
    }

    if(DID_HMI_CMD.Arrive_new_trama(&IN_data_type,0x00))
    {
        if(IN_data_type==CMD_TYPE)
        {
            //No importa direccion porque estoy lleyendo lo que llego por SERIAL!
            DID_HMI_CMD.Read_Data((uint8_t*)&_In_CMD,sizeof(DID_PERFIL));
            _Source=HMI_SOCMD;
            Serial.println("llego_CMD_HMI!!");
            return true;
        }
    }
    return false;
}   


uint16_t CONTROL_MSG::Get_CMD(void)
{
    return (uint16_t)_In_CMD.DID_CMD_User;
}

//Copia info en _wire_data_IN (USART)          a  _wire_data_OUT (I2C_M-> I2C_S)
bool CONTROL_MSG::Dispatcher_CMD(void)  
{
    switch (_In_CMD.DID_CMD_User)
    {
        //ESP32   32bits
        case MOV_A:     //Moviemiendo adelante  -> ACK
        case MOV_R:     //Moviemiendo reversa   -> ACK
        case MOV_D:     //Moviemiendo derecha   -> ACK
        case MOV_I:     //Moviemiendo izquierda -> ACK
        case MOV_MS:    //Medida del mov        -> MOV_MEASURE
        case MOV_S:     //Moviemiendo stop      -> ACK
            //Contruyo la trama 
            return DID_MASTER_485.Send_Data(CMD_TYPE,0x30,(uint8_t*)&_In_CMD);
        break;
        //ATmega 328p 8bits
        case LUZ_ON:    //Encender luces uvc    -> ACK       
        case LUZ_OFF:   //Apagar  luces uvc     -> ACK
        case LUZ_RST:   //Borra horas uso       -> ACK
        case BAT_RST:   //Borra contadores.     -> ACK
        case LUZ_ST:    //Retorna horas uso     -> LAMP_STATUS
        case CH_ST:     //Charger status        -> CHARGE_STATUS
        case BAT_ST:    //capacidad disp        -> BATERRY_STATUS
        case BAT_MSA:
        case BAT_MSB:
             //Contruyo la trama 
            return DID_MASTER_485.Send_Data(CMD_TYPE,0x40,(uint8_t*)&_In_CMD);
        break;    
        case  NO_DID_CMD:
        break;
    }
    return false;
}



bool CONTROL_MSG::Get_RSP(void)
{
    TRAMA_TYPE in_RSP_TYPE;
    //Dependiendo del comando que envie, espero respuesta del dispositivo.

    //Llego una trama.
    if(DID_MASTER_485.Arrive_new_trama(&in_RSP_TYPE,0x00)==true)
    {
        //En este momento hay unta trama completa y valida en el buffer.
        //Dependiendo del comando de envie, espero respuesta
        switch (_In_CMD.DID_CMD_User)
        {
            case MOV_A:     //Moviemiendo adelante
            case MOV_R:     //Moviemiendo reversa
            case MOV_D:     //Moviemiendo derecha
            case MOV_I:     //Moviemiendo izquierda
            case MOV_S:     //Moviemiendo stop
                if(in_RSP_TYPE==ACK_TYPE)
                {
                    Serial.println("llego ACK");
                    return DID_MASTER_485.Read_Data((uint8_t*)&_ACK,sizeof(char));
                }
                    
            break;
            case MOV_MS:    //Medida del mov (Mov actual,rpms,reversa)
                if(in_RSP_TYPE==MOV_TYPE)
                    return DID_MASTER_485.Read_Data((uint8_t*)&_State_MOV,sizeof(MOV_STATUS));
                        
            break;
            case LUZ_ON:    //Encender luces uvc
            case LUZ_OFF:   //Apagar  luces uvc
            case LUZ_RST:   //Borra horas uso
            case BAT_RST:   //Borra contadores.
                if(in_RSP_TYPE==ACK_TYPE)
                    return DID_MASTER_485.Read_Data((uint8_t*)&_ACK,sizeof(char));
            break;
            case LUZ_ST:    //Retorna horas uso
                if(in_RSP_TYPE==LAMP_ST_TYPE)
                {
                    if(DID_MASTER_485.Read_Data((uint8_t*)&_State_LAMP,sizeof(LAMP_STATUS))==true)
                    {
                        Serial.print("Estado_ON->");
                        Serial.println(_State_LAMP.Estado_ON);
                        Serial.print("Horas_Uso->");
                        Serial.println(_State_LAMP.Horas_Uso);
                        return true;
                    }   
                }            
            break;
            case CH_ST:
                if(in_RSP_TYPE==CHG_ST_TYPE)
                {
                    if(DID_MASTER_485.Read_Data((uint8_t*)&_Ciclos_BAT,sizeof(CHARGE_STATUS))==true)
                    {
                        Serial.print("Ciclos_Carga->");
                        Serial.println(_Ciclos_BAT.Ciclos_Carga);
                        Serial.print("STEPPER_ST_Status->");
                        Serial.println(_Ciclos_BAT.STEPPER_ST_Status);
                        return true;
                    }
                }
            break;
            case BAT_ST:    //Retorno capacidad disponible
                if(in_RSP_TYPE==BAT_ST_TYPE)
                {
                    if(DID_MASTER_485.Read_Data((uint8_t*)&_Capacidad_BAT,sizeof(BATERRY_STATUS))==true)
                    {
                        Serial.print("Cap_Cargada->");
                        Serial.println(_Capacidad_BAT.Cap_Cargada);
                        Serial.print("Cap_Disponible->");
                        Serial.println(_Capacidad_BAT.Cap_Disponible);
                        Serial.print("Cap_Nominal->");
                        Serial.println(_Capacidad_BAT.Cap_Nominal);
                        Serial.print("Cap_Usada->");
                        Serial.println(_Capacidad_BAT.Cap_Usada);
                        return true;
                    }
                }
            break;
            case BAT_MSA:
            case BAT_MSB:        
                if(in_RSP_TYPE==BAT_MSR_TYPE)
                {
                    if(DID_MASTER_485.Read_Data((uint8_t*)&_IV_BATAB,sizeof(BATERRY_MEASURE))==true)
                    {
                        Serial.print("I_actual->");
                        Serial.println(_IV_BATAB.I_actual);
                        Serial.print("V_actual->");
                        Serial.println(_IV_BATAB.V_actual);
                        return true;
                    }
                }
            break;
            case NO_DID_CMD:
            break;
        }
    }
    return false;
} 

//Envia la respuesta por el USAR fuente
bool CONTROL_MSG::Return_RSP(void)
{
    switch (_In_CMD.DID_CMD_User)
    {
        //ACK
        case MOV_A:     //Moviemiendo adelante
        case MOV_R:     //Moviemiendo reversa
        case MOV_D:     //Moviemiendo derecha
        case MOV_I:     //Moviemiendo izquierda
        case MOV_S:     //Moviemiendo stop
        case LUZ_ON:    //Encender luces uvc
        case LUZ_OFF:   //Apagar  luces uvc
        case LUZ_RST:   //Borra horas uso
        case BAT_RST:   //Borra contadores.
            if(_ACK!=TOKEN_ACK)
                return false;  
            if(_Source==BLUE_SOCMD)            //-> ACK (byte)
                DID_BLUE_CMD.Send_Data(ACK_TYPE,0x00,(uint8_t*)&_ACK);  
            if(_Source==HMI_SOCMD)
                DID_HMI_CMD.Send_Data(ACK_TYPE,0x00,(uint8_t*)&_ACK);  

        break;
        case LUZ_ST:    //Retorna horas uso    -> LAMP_STATUS _State_LAMP (LAMP_ST_TYPE)
            if(_Source==BLUE_SOCMD)
                DID_BLUE_CMD.Send_Data(LAMP_ST_TYPE,0x00,(uint8_t*)&_State_LAMP);  
            if(_Source==HMI_SOCMD)
                DID_HMI_CMD.Send_Data(LAMP_ST_TYPE,0x00,(uint8_t*)&_State_LAMP);  
        break;
        case MOV_MS:    //Medida del mov        -> MOV_MEASURE _State_MOV (MOV_TYPE)
            if(_Source==BLUE_SOCMD)
                DID_BLUE_CMD.Send_Data(MOV_TYPE,0x00,(uint8_t*)&_State_MOV);  
            if(_Source==HMI_SOCMD)
                DID_HMI_CMD.Send_Data(MOV_TYPE,0x00,(uint8_t*)&_State_MOV);  
        break;
        case CH_ST:  //Charger status        -> CHARGE_STATUS _Ciclos_BAT (CHG_TYPE)
            if(_Source==BLUE_SOCMD)
                DID_BLUE_CMD.Send_Data(CHG_ST_TYPE,0x00,(uint8_t*)&_Ciclos_BAT);  
            if(_Source==HMI_SOCMD)
                DID_HMI_CMD.Send_Data(CHG_ST_TYPE,0x00,(uint8_t*)&_Ciclos_BAT);  
        break;
        case BAT_ST:    //capacidad disp        -> BATERRY_STATUS _Capacidad_BAT (BAT_ST_TYPE)       
            if(_Source==BLUE_SOCMD)
                DID_BLUE_CMD.Send_Data(BAT_ST_TYPE,0x00,(uint8_t*)&_Capacidad_BAT);  
            if(_Source==HMI_SOCMD)
                DID_HMI_CMD.Send_Data(BAT_ST_TYPE,0x00,(uint8_t*)&_Capacidad_BAT);  
        break;  
        case BAT_MSA:
        case BAT_MSB:      
            if(_Source==BLUE_SOCMD)
                DID_BLUE_CMD.Send_Data(BAT_MSR_TYPE,0x00,(uint8_t*)&_IV_BATAB);  
            if(_Source==HMI_SOCMD)
                DID_HMI_CMD.Send_Data(BAT_MSR_TYPE,0x00,(uint8_t*)&_IV_BATAB);  
        break;
        case NO_DID_CMD:
            return false;
        break;
    }
    return true;
}
