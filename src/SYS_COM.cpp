#include "SYS_COM.h"
#include "SYS_TIPOS_COMUNES.h"

#define RS_485_CTR_PIN     18
#define STREAM_SPEED       9600
#define STREAM_TIMEOUT     400

//#define COM_DEBUG

SYS_COM::SYS_COM(WIRE_TYPE Buffer_mode)
{
  _Com_type=Buffer_mode;
}

//returna true, si hay memoria. ok
bool      SYS_COM::Begin(void * HWDriver,int buffer_tam)
{
  bool result=true;

  switch (_Com_type)
  {
    case MASTER_I2C:
    break;
    case SLAVE_I2C:
    break;
    case RS_485_SW:
    case SERIAL_SW:
    case MASTER_485_SW:
      _Pserial_SW=(SoftwareSerial*)HWDriver;
      _Pserial_SW->begin(STREAM_SPEED);
      result=_buffer_stream.Begin(N_BUFFER,buffer_tam);
      if(_Com_type==RS_485_SW || _Com_type==MASTER_485_SW)
      {
        pinMode(RS_485_CTR_PIN, OUTPUT); 
        digitalWrite(RS_485_CTR_PIN, LOW);
      }
    break;
    case RS_485_HW:
    case SERIAL_HW:
    case MASTER_485_HW:
      _Pserial=(HardwareSerial*)HWDriver;
      _Pserial->begin(STREAM_SPEED);
      result=_buffer_stream.Begin(N_BUFFER,buffer_tam);
      if(_Com_type==RS_485_HW || _Com_type==MASTER_485_HW)
      {
        pinMode(RS_485_CTR_PIN, OUTPUT); 
        digitalWrite(RS_485_CTR_PIN, LOW);
      }
    break;
  }
  _buffer_size=buffer_tam;
  return result;
}

//Devuelve true, cuando llega un dato valido.  ok
//Para el modo serial verifica que hay datos en el buffer stream
bool   SYS_COM::_Arrive_new_data(void)
{
  switch (_Com_type)
  {
    case MASTER_I2C:
    break;
    case SLAVE_I2C:
    break;
    case RS_485_SW:
    case SERIAL_SW:
    case MASTER_485_SW:
      if(_Com_type==RS_485_SW || _Com_type==MASTER_485_SW)
        digitalWrite(RS_485_CTR_PIN, LOW);
      if(_Pserial_SW->available()> 0)
      {
        //Serial.println("1");
        return true;

      }
 
    break;
    case RS_485_HW:
    case SERIAL_HW:
    case MASTER_485_HW:
      if(_Com_type==RS_485_HW || _Com_type==MASTER_485_HW)
        digitalWrite(RS_485_CTR_PIN, LOW);
      if(_Pserial->available()> 0)
        return true;
    break;
  }
  return false;
}

//Indice si ha llegado una trama nueva. Busca token de inicio, si lo encuentra se queda en 
//ciclo hasta timeout o token FIN
//Luego compara la dirección de la trama para ver si soy el destino. ok
bool      SYS_COM::Arrive_new_trama(TRAMA_TYPE* Incommin_type,uint8_t MyDDr)
{
  uint8_t c;
  unsigned long startTime = millis();
  bool trama_incio=false;
  bool trama_fin=false;
  bool result = false;

  *Incommin_type=NN_TYPE;   
  _buffer_stream.Clear_index();

  switch (_Com_type)
  {
    case MASTER_I2C:
    break;
    case SLAVE_I2C:
    break;
    case RS_485_SW:
    case SERIAL_SW:
    case MASTER_485_SW:
    if(_Arrive_new_data()==true)  //Llego un dato i2c /serial
    {
      //Se queda pegado en el do-while esperando informacion
      do
      {
        if(_Arrive_new_data()) //No pasa nada que vuelva preguntar,
        {
          c=_Pserial_SW->read();
         
          if(_buffer_stream.Is_incio_trama(c)==true || trama_incio==true)
          {
            trama_incio=true;
             Serial.println(c,HEX);
            result=_buffer_stream.Load_buffer(c);
            trama_fin=_buffer_stream.Is_fin_trama(c);
          }
          startTime = millis();
        }  
        yield();
      }while(!(trama_fin) && (millis() - startTime) < STREAM_TIMEOUT);
    }
    break;
    case RS_485_HW:
    case SERIAL_HW:
    case MASTER_485_HW:
    if(_Arrive_new_data()==true)  //Llego un dato i2c /serial
    {
      //Se queda pegado en el do-while esperando informacion
      do
      {
        if(_Arrive_new_data()) //No pasa nada que vuelva preguntar,
        {
          c=_Pserial->read();
          
          if(_buffer_stream.Is_incio_trama(c)==true || trama_incio==true)
          {
            trama_incio=true;
            Serial.println(c,HEX);
            result=_buffer_stream.Load_buffer(c);
            trama_fin=_buffer_stream.Is_fin_trama(c);
          }
          startTime = millis();
        }  
        yield();
      }while(!(trama_fin) && (millis() - startTime) < STREAM_TIMEOUT);
    }
    break;
  }

  //Apenas llegue el token inicio el buffer se queda enclabado esperando a token FIN 
  //Si resul == true significa que el token fin llego y que la trama que llego es valida.
  //Si no es valida debo borrar el buffer.
  //Si es valida el buffer se borrar al leerlo.
  if(trama_incio==true)
  {
    Serial.print("_Com_type->");
    Serial.println(_Com_type);
    Serial.println("T_Buffer=");
    _buffer_stream.Print_buffer();

  }

  if(result==true)
  {
    //Trama correecta, pero sera para mi?
    //Si el parameto MyDDr==x00 recivo todo.
    // MyDDr (slave RS485)
    // MASTER recive todo
    // Serial recive todo 
    if(_buffer_stream.Read_trama_DDR()==MyDDr || 
        _Com_type==MASTER_485_HW || _Com_type==MASTER_485_SW ||
        _Com_type==SERIAL_HW || _Com_type==SERIAL_SW)
    {
      *Incommin_type=_buffer_stream.Read_trama_type(); 
      //#ifdef COM_DEBUG
      
      Serial.print("Trama_OK ");
      
      //#endif
    }
    else
    {
      //#ifdef COM_DEBUG
      Serial.println("NO_ES_PARA_MI");
      //#endif
      result=false;
    }
  }
  
  //Si se cumple esto es poque la trama no era para mi o era invalida. Tengo que limpar el 
  //buffer para recivir nueva informacion.
  if(result==false)
    _buffer_stream.Clear_buffer();
  
  
  return result;
}

//Del buffer de informacon se lee los datos.
bool      SYS_COM::Read_Data(uint8_t* pbuffer_destino, uint8_t buffer_size)
{
  #ifdef COM_DEBUG  
    Serial.println("Buffer_a_leer");
    _buffer_stream.Print_buffer();
  #endif
  return _buffer_stream.Read_buffer(pbuffer_destino,buffer_size);
}

//Se envia la informacion. Creando un buffer temporal con formata tipo TRAMA y luego se invia,¿, segun protocolo. ok
bool      SYS_COM::Send_Data(TRAMA_TYPE Data_type,uint8_t Add_Destino,uint8_t* pbuffer_Fuente)                                                               
{
  int  Trama_size;
  bool Result=false;
  uint8_t* pbuffer_Trama;

  //Creo que vector, al reservar memoria.
  pbuffer_Trama= (uint8_t*)malloc(sizeof(uint8_t)*_buffer_size);         
  if(pbuffer_Trama==NULL)
      return Result;
  
  //Limpio la variable destino 
  memset ((void*)pbuffer_Trama, 0x00,_buffer_size);

  //Creo que la trama y la guardo en buffer_Trama.
  Trama_size=_buffer_stream.Make_OUT_buffer(Data_type,Add_Destino,pbuffer_Trama,pbuffer_Fuente);

  //Si es valida.
  if(Trama_size!=0 || Trama_size<_buffer_size)
  {
    //HWDriver
    switch (_Com_type)
    {
      case MASTER_I2C:
      break;
      case SLAVE_I2C:
      break;
      case RS_485_SW:
      case SERIAL_SW:
      case MASTER_485_SW:
        if(_Com_type==RS_485_SW || _Com_type==MASTER_485_SW)
        {
          digitalWrite(RS_485_CTR_PIN, HIGH);
          delay(2);
        }
        for(int data_index=0;data_index<Trama_size;data_index++)
        _Pserial_SW->write(pbuffer_Trama[data_index]);
        if(_Com_type==RS_485_SW || _Com_type==MASTER_485_SW)
        {
          delay(2);
          digitalWrite(RS_485_CTR_PIN, LOW);    
        }
      Result=true;   
      break;
      case RS_485_HW:
      case SERIAL_HW:
      case MASTER_485_HW:
         if(_Com_type==RS_485_HW || _Com_type==MASTER_485_HW)
        {
          digitalWrite(RS_485_CTR_PIN, HIGH);
          delay(2);
        }
        for(int data_index=0;data_index<Trama_size;data_index++)
        _Pserial->write(pbuffer_Trama[data_index]);
        if(_Com_type==RS_485_HW || _Com_type==MASTER_485_HW)
        {
          delay(2);
          digitalWrite(RS_485_CTR_PIN, LOW);    
        }    
      Result=true;   
      break;
    }
  }
  free(pbuffer_Trama);
  return Result;
}

