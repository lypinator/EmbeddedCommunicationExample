
#include "serial_user.h"


typedef enum _gps_packet_buffer_status {
  WAITING_FOR_PACKET_START,
  IN_HEADER,
  IN_PACKET,
  IN_CHECKSUM,
  PACKET_RECEIVED
} gps_packet_buffer_status;

typedef struct _GPS_Data_t {
  float UTC_Time;
  float Latitude;
  char NS_Indicator;
  float Longitude;
  char EW_Indicator;
  uint8_t Pos_fix;
  uint8_t Sat_Used;
  float HDOP;
  float MSL_Alt;
  char MSL_Unit;
  float Geoid_Sep;
  char Geoid_Unit;
  uint8_t CHk_Sum;
} GPS_Data_t;

extern uint8_t gpsPacketBuffer[];
extern uint8_t gpsHeaderBuffer[];

extern uint8_t gpsPacketBufferIndex;
extern uint8_t gpsHeaderBufferIndex;

extern gps_packet_buffer_status gpsPacketStatus;

extern uint8_t gpsProcessPacket;
extern uint8_t GPS_Data_Valid;

extern GPS_Data_t gps_dat;

void ProcessGpsInputChar(comm_input_buffer_t * _buff_instance);
void ProcessGPS_Packet(void);

#define GPS_PACKET_BUFFER_LENGTH 150