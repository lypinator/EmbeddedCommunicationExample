// serial_user.h

#ifndef _SERIAL_USER_
#define _SERIAL_USER_


#include "serial.h"

#define ESCAPE_CHAR 0xAA
#define BUFFER_SIZE 100
#define SMALL_BUFFER_SIZE 50
#define MED_BUFFER_SIZE 100
#define LARGE_BUFFER_SIZE 150


// use for <buffer>_p_t
//#define <name>_BUFFER_SIZE 100


// public references ********************
extern comm_input_buffer_t gps_inputBuffer;
extern comm_output_buffer_t gps_outputBuffer;
extern comm_input_buffer_t pc_inputBuffer;
extern comm_output_buffer_t pc_outputBuffer;
extern comm_input_buffer_t rs485_inputBuffer;
extern comm_output_buffer_t rs485_outputBuffer;
extern comm_input_buffer_t fo_inputBuffer;
extern comm_output_buffer_t fo_outputBuffer;

extern uint8_t PC_PacketRdy;
extern uint8_t Send_GPS_PC;

// prototypes

//Check buffer for valid packet
uint8_t ProcessInputChar(comm_input_buffer_t * _buff_instance);

//React to valid packets
uint8_t Process_Packet(comm_input_buffer_t * _buff_instance);

#endif