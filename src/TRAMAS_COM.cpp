#include "SYS_COM.h"
#include "SYS_TIPOS_COMUNES.h"

#define I_BYTE_INICIO        0
#define I_BYTE_ADD           1
#define I_BYTE_TYPE          2
#define I_BYTE_INFO          3


/////////////////////////////////////////////// TRAMA  MAX 32 bytes //////////////////////////////////////////
// BYTE INCIO      // BYTE ADD   // BYTE TYPE         // BYTES INFOs         // BYTE CKS      //   BYTE FIN //     
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// I_BYTE_INCIO    (TOKEN INICIO TRAMA)                                                                     //
// I_BYTE_ADD      (BYTE DE DESTINO)                                                                        //
// I_BYTE_TYPE     (TIPO DE INFORMACION)                                                                    //                                      
// I_BYTE_INFO     (INFORMACION EN SI)                                                                      //
// I_BYTE_CKS      (BYTE DE LLAVE)                                                                          //
// I_BYTE_FIN      (TOKEN FIN TRAMA)                                                                        //
// MAX LONGITUD  15 BYTES                                                                                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define TRAMA_DEBUG


//Contructora del objeto, se inicializa el tamano a cero. ok
TRAMA_DID::TRAMA_DID(void)
{
  _buffer_tam=0;
}

//Reserba memoria, devulve true si hay espacio.  ok
bool TRAMA_DID::Begin(char Buffer_mode,int buffer_tam)
{
  #ifdef TRAMA_DEBUG
  int pPrint;
   #endif

  if(buffer_tam<=0)
    return false;

  _Buffer_mode=Buffer_mode;
  
  if(_Buffer_mode==N_BUFFER)
  {
    _Strem_INFO= (uint8_t*)malloc(sizeof(uint8_t)*buffer_tam);             //Datos recibidos         (I2C-Serial) 
    if(_Strem_INFO==NULL)
      return false;
    memset ((void*)_Strem_INFO, 0x00,buffer_tam);
    #ifdef TRAMA_DEBUG
    pPrint=(int)_Strem_INFO;
     #endif
  }
  else
  {
    _wire_data_INFO = (volatile uint8_t*)malloc(sizeof(uint8_t)*buffer_tam);             //Datos recibidos         (I2C-Serial) 
    if(_wire_data_INFO==NULL)
      return false;
    memset ((void*)_wire_data_INFO, 0x00,buffer_tam);
    #ifdef TRAMA_DEBUG
    pPrint=(int)_wire_data_INFO;
     #endif
  }
  _buffer_tam=buffer_tam;
  #ifdef TRAMA_DEBUG
    Serial.print("DrrBUFFER-> ");
    Serial.println(pPrint,HEX);
  #endif
  return true;

}

//Detecta tipo de dato correcto ok
bool TRAMA_DID::_Is_data_type(uint8_t info_data)
{
  switch(info_data)    
  {
    case  CHG_ST_TYPE:             // struct CHARGE_STATUS   (Ciclos carga y paso del stepper )
    case  LAMP_ST_TYPE:
    case  BAT_ST_TYPE:          // struct BATERRY_STATUS  (Cap_Nominal, Cap_Usada)
    case  BAT_MSR_TYPE:         // struct BATERRY_MEASURE (I/V y modo (CHG-DIS-IDEL-FAIL))
    case  MOV_TYPE:             // struct MOV_MEASURE     (Sensado: RPM, Reversa y giro )
    case  CMD_TYPE:             // struct DID_PERFIL      (CMD: Mov,Luz AMD_CMD: Pide Medidas, Velo, Luz time)
    case  ACK_TYPE:             // byte ACK
      return true;
    break;
  }
  return false;
}

//Devuelve el tama?o del dato. ok
uint8_t TRAMA_DID::_Data_type_size(uint8_t info_data)
{
  switch(info_data)    
  {
    case  CHG_ST_TYPE:             
      return sizeof(CHARGE_STATUS);
    break;  
    case  BAT_ST_TYPE:          
      return sizeof(BATERRY_STATUS);
    break;  
    case  BAT_MSR_TYPE:         
      return sizeof(BATERRY_MEASURE);
    break;  
    case  MOV_TYPE:             
      return sizeof(MOV_STATUS);
    break;  
    case  CMD_TYPE:             
      return sizeof(DID_PERFIL);
    break;  
    case LAMP_ST_TYPE:
      return sizeof(LAMP_STATUS);
    break;
    case ACK_TYPE:
      return sizeof(char);
    break;
  }
 return 0;
}

//Devuelve el tipo de trama recivida, no es conocida devulve NN_TYPE. ok
TRAMA_TYPE  TRAMA_DID::Read_trama_type(void)
{
  uint8_t* pdata=NULL;

  if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;

  if(pdata==NULL)
    return NN_TYPE;
  
  if(_Is_data_type(pdata[I_BYTE_TYPE])==true)
    return (TRAMA_TYPE)pdata[I_BYTE_TYPE];
  return NN_TYPE;
}

//Devuele la direccion de dispositvo destino.
uint8_t   TRAMA_DID::Read_trama_DDR(void)
{
   uint8_t* pdata=NULL;

  if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;

  if(pdata==NULL)
    return 0; 

  return pdata[I_BYTE_ADD];

}

void       TRAMA_DID::Clear_index(void)
{
  _in_data_index=0;
}

//Limpia el buffer e inicializa el contador de datos recividos.
void     TRAMA_DID::Clear_buffer(void)
{
  uint8_t* pdata=NULL;

  if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;
  
  if(pdata!=NULL)
    memset ((void*)pdata, 0x00,_buffer_tam);
  _in_data_index=0;
}

//imprime le buffer.
 void     TRAMA_DID::Print_buffer()
 {
  uint8_t* pdata=NULL;

  if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;
  
  if(pdata!=NULL)
  {
    for(int data_index=0;data_index<_buffer_tam;data_index++)
      Serial.print(pdata[data_index],HEX);    
  }
  Serial.println("");
 }

//Crea trama en el  buffer, a partir de Data_type y buffer_Fuente. Todo esta OK, devuelve la canditidad de bytes 
//de la trama. Cantidiad de bytes = 0, indica error.
uint8_t  TRAMA_DID::Make_OUT_buffer( TRAMA_TYPE Data_type,uint8_t ADD_Destino,
                                    uint8_t* pbuffer_Trama,uint8_t* pbuffer_Datos)
{
  int index_data;
  uint8_t Index_CHK=0;

  if(_buffer_tam==0)
    return 0;


  #ifdef TRAMA_DEBUG
    Serial.print("DDR_BUFFER_OUT ");
    Serial.println((int)pbuffer_Trama,HEX);
  #endif
  
  if(pbuffer_Trama==NULL || pbuffer_Datos==NULL)
    return 0;


  if(_Is_data_type((uint8_t)Data_type))
  {
    memset ((void*)pbuffer_Trama, 0x00,_buffer_tam);
    Index_CHK=_Data_type_size(Data_type)+ I_BYTE_INFO;

    //index 0   -> inicio
    //index 1   -> Data_type
    //index 2   -> INFO 
    //index CHK -> _Data_type_size(Data_type) + index 2
    //index FIN -> index CHK+1
    pbuffer_Trama[I_BYTE_INICIO]=TOKEN_INCIO;
    pbuffer_Trama[I_BYTE_ADD]=ADD_Destino;
    pbuffer_Trama[I_BYTE_TYPE]=(uint8_t)Data_type;

    //Copio los datos
    for(index_data=I_BYTE_INFO;index_data<Index_CHK;index_data++)
       pbuffer_Trama[index_data]=pbuffer_Datos[index_data-I_BYTE_INFO];
    
    //Calulo el CHKsum desde i = 0 hasta (Index_CHK-1)
    pbuffer_Trama[Index_CHK]=_CalCHK((uint8_t*)pbuffer_Trama,Index_CHK);
    pbuffer_Trama[Index_CHK+1]=TOKEN_FIN;

    //#ifdef TRAMA_DEBUG
    //Imprimo todo el buffer
      Serial.println("TRAMA_OUT-> ");
      for(int data_index=0;data_index<_buffer_tam;data_index++)
         Serial.print(pbuffer_Trama[data_index],HEX);
      Serial.println("-");
    //#endif
  }
  else
  {
    return 0;
  }

  //index del FIN <
  return Index_CHK+2;
} 

//Adiciona un byte al buffer y verifica que es una trama.
//Datos sin offset. ok
bool     TRAMA_DID::Load_buffer(uint8_t Incoming_Data)
{
  uint8_t* pdata=NULL;
  bool     Result=false;

  if(_buffer_tam==0)
    return Result;

  if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;

  if(pdata==NULL)
    return Result; 

  if(_in_data_index<_buffer_tam)
  {
    pdata[_in_data_index]=Incoming_Data;
    _in_data_index++;
    Result=_Is_trama();
  }
  else
  {
    _in_data_index=0;
    memset ((void*)pdata, 0x00,_buffer_tam);
  }
  return Result;

}

//Lee la trama -> y copia los datos a buffer_destino.  ok                                     
bool     TRAMA_DID::Read_buffer(uint8_t* pbuffer_destino, uint8_t destino_size)
{
  uint8_t* pdata;
  bool result=false;

  if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;

  if(pdata==NULL || pbuffer_destino==NULL)
    return false;

  if(_buffer_tam<0)
    return false;

  if(destino_size+I_BYTE_INFO>_buffer_tam)
    return false;
  
  //borro el destino.
  memset ((void*)pbuffer_destino, 0x00,destino_size);

  if(_Is_trama()==true)
  {
    //Trama valida 
    //desde I_BYTE_INFO a I_BYTE_INFO+data_size
    for(int data_index=I_BYTE_INFO;data_index<destino_size+I_BYTE_INFO;data_index++)
      pbuffer_destino[data_index-I_BYTE_INFO]=pdata[data_index];
    result=true;
  }
  else
  {
    Serial.println("Trama_Invalida");
  }
  //Simpre libero el buffer. Sea trama valida o no, para poder recivir la siguiente posible trama.

  memset ((void*)pdata, 0x00,_buffer_tam);
  return result;
}

//Detecta TOKEN inicio de trama
bool TRAMA_DID::Is_incio_trama(uint8_t info_data)
{
  if(info_data==TOKEN_INCIO)
    return true;
  return false;
}

//Detecta TOKEN fin de trama
bool TRAMA_DID::Is_fin_trama(uint8_t info_data)
{
  if(info_data==TOKEN_FIN)
    return true;
  return false;
}

//Prueba si en _wire_data_IN hay una trama valida
bool   TRAMA_DID::_Is_trama(void)
{
  uint8_t* pdata;
  uint8_t   size;
  uint8_t Index_CHK;


   if(_Buffer_mode==N_BUFFER)
    pdata=(uint8_t*)_Strem_INFO;
  else
    pdata=(uint8_t*)_wire_data_INFO;
  
  if(pdata==NULL)
    return false;

  size=_Data_type_size(pdata[I_BYTE_TYPE]);
  
  //Pos donde se guarda el CHK
  Index_CHK=size+ I_BYTE_INFO;  //BYTE (INCIO,DDR,TYPE)
  //Indica inicio de trama
  #ifdef TRAMA_DEBUG
  Serial.print("size = ");
  Serial.println(size);
  Serial.print("Index_CHK = ");
  Serial.println(Index_CHK);
  #endif
  
  if(Is_incio_trama(pdata[I_BYTE_INICIO])!=true)
    return false;
  //Si en la I_BYTE_TYPE hay un tipo de dato conocido
  #ifdef TRAMA_DEBUG
  Serial.println("test1 ");
  #endif
  if(_Is_data_type(pdata[I_BYTE_TYPE])!=true)
    return false;
  //Si justo en la posicion CHK+1, esta el byte fin de trama 
  #ifdef TRAMA_DEBUG
  Serial.println("test2 ");
  #endif
  //Index_CHK+1, pos de la ultimo byte.
  if(Is_fin_trama(pdata[Index_CHK+1])!=true)
    return false;
  //Y cuadra el chK, entonces es una trama valida
  #ifdef TRAMA_DEBUG
  Serial.print("test3-> ");
  Serial.println(pdata[Index_CHK],HEX);
  #endif
  
  //CHKsum desde index =0 hasta Index_CHK-1
  //No se incluye el CHK.
  if( _CalCHK(pdata,Index_CHK) != pdata[Index_CHK])
    return false;

  #ifdef TRAMA_DEBUG 
  Serial.println("test4 ");
  #endif
  return true;
}

//Caluclo el CHKsum la info apuntada por data
uint8_t TRAMA_DID::_CalCHK(uint8_t *data, uint8_t count )
{
  uint8_t value = 0;

  for (int i = 0; i < count; i++)
  {
    value += (uint8_t)data[i];
  }

  #ifdef TRAMA_DEBUG
  Serial.print("CHK-> ");
  Serial.println(value,HEX);
  Serial.print(" N-> ");
  Serial.println(~value,HEX);
  #endif
  return ~value;
}  