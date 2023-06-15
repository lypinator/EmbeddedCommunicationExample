// serial_user.c

#include "serial_user.h"
#include "ux_manager.h"
#include <string.h>


//GPS Communication Buffers
uint8_t GPS_rxBuffer[RX_BUFFER_SIZE];
uint8_t GPS_txBuffer[TX_BUFFER_SIZE];

//PC Communication Buffers
uint8_t PC_rxBuffer[RX_BUFFER_SIZE];
uint8_t PC_txBuffer[TX_BUFFER_SIZE];

//RS485 Communication Buffers
uint8_t RS485_rxBuffer[RX_BUFFER_SIZE];
uint8_t RS485_txBuffer[TX_BUFFER_SIZE];

//Fiber Optic Communication Buffers
uint8_t FO_rxBuffer[RX_BUFFER_SIZE];
uint8_t FO_txBuffer[TX_BUFFER_SIZE];

//Input and Output Buffer for GPS
comm_input_buffer_t gps_inputBuffer = {0,0, GPS_rxBuffer, RX_BUFFER_SIZE, {0}, GPS_COM, false, 0, false, USART1};
comm_output_buffer_t gps_outputBuffer = {0,0, GPS_txBuffer, TX_BUFFER_SIZE, GPS_COM, USART1};

//Input and Output Buffer for PC
comm_input_buffer_t pc_inputBuffer = {0,0, PC_rxBuffer, RX_BUFFER_SIZE, {0}, PC_COM, false, 0, false, USART2};
comm_output_buffer_t pc_outputBuffer = {0,0, PC_txBuffer, TX_BUFFER_SIZE, PC_COM, USART2};

//Input and Output Buffer for RS485
comm_input_buffer_t rs485_inputBuffer = {0,0, RS485_rxBuffer, RX_BUFFER_SIZE, {0}, RS485_COM, false, 0, false, USART3};
comm_output_buffer_t rs485_outputBuffer = {0,0, RS485_txBuffer, TX_BUFFER_SIZE, RS485_COM, USART3};

//Input and Output Buffer for Fiber Optic
comm_input_buffer_t fo_inputBuffer = {0,0, FO_rxBuffer, RX_BUFFER_SIZE, {0}, FO_COM, false, 0, false, USART4};
comm_output_buffer_t fo_outputBuffer = {0,0, FO_txBuffer, TX_BUFFER_SIZE, FO_COM, USART4};


uint8_t Send_GPS_PC = false;    //Has PC requested GPS packet?


// function to process the input buffer with an ASCII-based protocol 
// Generalized for all input buffers
uint8_t ProcessInputChar(comm_input_buffer_t * _buff_instance)
{
  if (_buff_instance->buffer[_buff_instance->nextBufferOut] == '$') {
    _buff_instance->inPacket = true;
    _buff_instance->packetBuffer[0] = _buff_instance->buffer[_buff_instance->nextBufferOut];
    _buff_instance->nextPacketChar = 1;
  }
  else {
    if (_buff_instance->inPacket == true) {
      if (((_buff_instance->buffer[_buff_instance->nextBufferOut] >= '0') && (_buff_instance->buffer[_buff_instance->nextBufferOut] <= '9')) || 
          ((_buff_instance->buffer[_buff_instance->nextBufferOut] >= 'r') && (_buff_instance->buffer[_buff_instance->nextBufferOut] <= 'v')) ||
          ((_buff_instance->buffer[_buff_instance->nextBufferOut] >= 'R') && (_buff_instance->buffer[_buff_instance->nextBufferOut] <= 'V')) ||
          (_buff_instance->buffer[_buff_instance->nextBufferOut] >= '\n') || (_buff_instance->buffer[_buff_instance->nextBufferOut] <= '\r')) {
        
            _buff_instance->packetBuffer[_buff_instance->nextPacketChar++] = _buff_instance->buffer[_buff_instance->nextBufferOut];

            if (_buff_instance->buffer[_buff_instance->nextBufferOut] == '\n') {
              _buff_instance->PacketRdy = true;
              _buff_instance->inPacket = false;
            }
          }
      else {
        _buff_instance->inPacket = false;
      }
    }
  }
  
  
  if (++_buff_instance->nextBufferOut >= _buff_instance->bufferLength) {
    _buff_instance->nextBufferOut = 0;
  }
  return 0;
}

//Process each buffer's packets uniquely
uint8_t Process_Packet(comm_input_buffer_t * _buff_instance)
{
  char message[16] = "";
  uint8_t i = 0;
  switch(_buff_instance->buff_type){
  
  
  case PC_COM:  //Main board receives message from PC connection
    switch (_buff_instance->packetBuffer[1]) {
    // list of commands
    // each command has intentional fallthru for upper/lower case
    case 'r':     // r = GPS Data Request from PC
    case 'R':     
      Send_GPS_PC = true;
      break;
    case 's':     // s = Send incoming ASCII number to Fiber Optic for display on remote screen 1
    case 'S':
      while (_buff_instance->packetBuffer[i] != '\r' && _buff_instance->packetBuffer[i] != '\n'){
        message[i] = _buff_instance->packetBuffer[i];
        i++;
      }
      SendString(&fo_outputBuffer, message, i+1, StripZeros, AddCRLF);
      break;
    case 'x':
    case 'X':           //Message to pass to external process [$x<0,1><remote cmd>\r\n]
      message[0] = '$'; //Packet starting char
      message[1] = _buff_instance->packetBuffer[2];     //Take address from PC message
      i = 3;
      while (_buff_instance->packetBuffer[i] != '\r' && _buff_instance->packetBuffer[i] != '\n'){       
        message[i-1] = _buff_instance->packetBuffer[i];         //Copy all of the PC message contents
        i++;
      }
      SendString(&rs485_outputBuffer, message, i+1, StripZeros, AddCRLF);       //Send the message to the RS485 buffer for remote processor to handle.
      break;
    }
  break;
  
  case RS485_COM:       //Main board recieves message on RS485 bus
      switch (_buff_instance->packetBuffer[1]) {
    // list of commands
    // each command has intentional fallthru for upper/lower case
    case 't':     // t = Change to GPS stats on screen
    case 'T':
      SwitchScreens(SHOW_GPS_COORDS);
      break;
    }
  break;  
  
  case FO_COM:  //Main board receives communication from Fiber Optic loop
    switch (_buff_instance->packetBuffer[1]) {
    // list of commands
    // each command has intentional fallthru for upper/lower case
    case 's':     // s = send command to Remote processor 1 (PC->Main micro->Fiber Optic->Main Micro->Remote Processor 1)
    case 'S':
      message[0] = '$';
      message[1] = '0'; //Remote processor 1 address
      message[2] = 'd';
      int i = 2;
      while (_buff_instance->packetBuffer[i] != '\r' && _buff_instance->packetBuffer[i] != '\n'){
        message[i+1] = _buff_instance->packetBuffer[i];
        i++;
      }
      SendString(&rs485_outputBuffer, message, i+1, StripZeros, AddCRLF);       //Send to RS485 bus
      break;
    }
  break;
  
  case GPS_COM:         //GPS packets preocessed through GPS module -- Added call here for consisitency
    break;
  }
  _buff_instance->PacketRdy = false;
  return 0;
}
