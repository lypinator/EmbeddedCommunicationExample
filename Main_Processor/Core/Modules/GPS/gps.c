#include "gps.h"
#include "main.h"
#include "ux_manager.h"
#include <math.h>


uint8_t gpsPacketBuffer[GPS_PACKET_BUFFER_LENGTH] = "$GNGGA";
uint8_t gpsHeaderBuffer[10];

uint8_t gpsPacketBufferIndex = 0;
uint8_t gpsHeaderBufferIndex = 0;

GPS_Data_t gps_dat;

gps_packet_buffer_status gpsPacketStatus = WAITING_FOR_PACKET_START;

uint8_t gpsProcessPacket = false;
uint8_t GPS_Data_Valid = false;

void ProcessGpsInputChar(comm_input_buffer_t * _buff_instance)
{
  switch (gpsPacketStatus) {
  case WAITING_FOR_PACKET_START:
    if (_buff_instance->buffer[_buff_instance->nextBufferOut] == '$') {
      gpsHeaderBufferIndex = 0;
      gpsHeaderBuffer[gpsHeaderBufferIndex] = _buff_instance->buffer[_buff_instance->nextBufferOut];
      gpsHeaderBufferIndex++;
      gpsPacketStatus = IN_HEADER;
    }
    break;
  case IN_HEADER:
    gpsHeaderBuffer[gpsHeaderBufferIndex] = _buff_instance->buffer[_buff_instance->nextBufferOut];
    gpsHeaderBufferIndex++;
    
    if (gpsHeaderBufferIndex == 6) {
      if ((gpsHeaderBuffer[3] == 'G') && (gpsHeaderBuffer[4] == 'G') && (gpsHeaderBuffer[5] == 'A')) {
        gpsPacketBufferIndex = 6;
        gpsPacketStatus = IN_PACKET;
      }
      else 
        gpsPacketStatus = WAITING_FOR_PACKET_START;
    }
    break;
  case IN_PACKET:
    gpsPacketBuffer[gpsPacketBufferIndex] = _buff_instance->buffer[_buff_instance->nextBufferOut];
    
    if ((gpsPacketBuffer[gpsPacketBufferIndex] == '\r') || (gpsPacketBuffer[gpsPacketBufferIndex] == '\n'))
      gpsPacketStatus = WAITING_FOR_PACKET_START;
    
    if (gpsPacketBufferIndex >= GPS_PACKET_BUFFER_LENGTH - 1)
      gpsPacketStatus = WAITING_FOR_PACKET_START;
    
    if (gpsPacketBuffer[gpsPacketBufferIndex] == '*')
      gpsPacketStatus = IN_CHECKSUM;
    
    gpsPacketBufferIndex++;
    break;
  case IN_CHECKSUM:
    gpsPacketBuffer[gpsPacketBufferIndex] = _buff_instance->buffer[_buff_instance->nextBufferOut];
    
    if ((gpsPacketBuffer[gpsPacketBufferIndex] == '\r') || (gpsPacketBuffer[gpsPacketBufferIndex] == '\n')) {
      gpsProcessPacket = true;
      gpsPacketStatus = WAITING_FOR_PACKET_START;
    }
  
    if (gpsPacketBufferIndex >= GPS_PACKET_BUFFER_LENGTH - 1)
      gpsPacketStatus = WAITING_FOR_PACKET_START;

    gpsPacketBufferIndex++;
    break;
  }
  
  _buff_instance->nextBufferOut++;
  if (_buff_instance->nextBufferOut >= _buff_instance->bufferLength) _buff_instance->nextBufferOut = 0;
}

float DDDMM_to_DDDDD(float DDDMM_MMMM){
  float minutes, degrees;
  degrees = DDDMM_MMMM / 100.0f;
  degrees = trunc(degrees);
  minutes = fmodf(DDDMM_MMMM,100.0f);
  degrees += minutes/60.0;
  return (degrees);
}

void ProcessGPS_Packet(void)
{
  //Check packet type
  char *p = gpsPacketBuffer;
  char *e = strchr(p, ',');
  char temp[15];
  char chk_sum = 0x00;

  //Returns 0 if they are the same
  if(!strncmp(p+1,"GNGGA",(e-p)-1)){
    p = gpsPacketBuffer+1;
    e = strchr(p, '*');
    while(p != e){
      chk_sum ^= *p;
      p++;
    }
    uint8_t check = 0x00;
    uint8_t top_half = (*(e+1));
    uint8_t bot_half = (*(e+2));
    if(top_half >= '0' && top_half <= '9')
      check |= top_half << 4;
    else
      check = (top_half-31) << 4;
 
    if(bot_half >= '0' && bot_half <= '9')
      check |= (bot_half-(uint8_t)'0');
    else
      check |= (check-31);
    
    if (check != chk_sum) return;
    gps_dat.CHk_Sum = check;
    p = gpsPacketBuffer;
    e = strchr(p, ',');
    
    p = e+1;
    
    //Get UTC Time
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p));
      gps_dat.UTC_Time = atof(temp);
    }
    p = e+1;
    memset(temp,0x00,15);
    //Get Latitude
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p));
      gps_dat.Latitude = DDDMM_to_DDDDD(atof(temp));
    }
    if (gps_dat.Latitude != 0.0) GPS_Data_Valid = true;
    else GPS_Data_Valid = false;
     p = e+1;
     memset(temp,0x00,15);
    //Get N/S 
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p)-1);
      gps_dat.NS_Indicator = *(p);
    }
    if (gps_dat.NS_Indicator == 'S') gps_dat.Latitude = -gps_dat.Latitude;
    p = e+1;
    memset(temp,0x00,15);
    
    //Get Longitutde
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p));
      gps_dat.Longitude = DDDMM_to_DDDDD(atof(temp));
    }
    if (gps_dat.Longitude != 0.0) GPS_Data_Valid = true;
    else GPS_Data_Valid = false;
    p = e+1;
    memset(temp,0x00,15);
    //Get E/W 
    e = strchr(p, ',');
    if (*(e+1) != ',') gps_dat.EW_Indicator = *(p);
    if (gps_dat.EW_Indicator == 'W') gps_dat.Longitude = -gps_dat.Longitude;
    p = e+1;
    memset(temp,0x00,15);
    //Get Position Fix
    e = strchr(p, ',');
    if (e != p) gps_dat.Pos_fix = *(p);
    
    p = e+1;
    memset(temp,0x00,15);
    //Get Satelites used
    
    e = strchr(p, ',');

    if (e != p) gps_dat.Sat_Used = *(p);
    
    p = e+1;
    memset(temp,0x00,15);
    
    //Get HDOP
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p));
      gps_dat.HDOP = atof(temp);
    }   
    p = e+1;
    memset(temp,0x00,15);
    
    //Get Altitude above/below sea level
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p));
      gps_dat.MSL_Alt = atof(temp);
    }
    p = e+1;
    memset(temp,0x00,15);
    //Get Altitude unit
    e = strchr(p, ',');
    
    if (e != p) gps_dat.MSL_Unit = *(p);
    
    p = e+1;
    memset(temp,0x00,15);
    
    //Get Geoidal Seperation
    e = strchr(p, ',');
    if (*(e+1) != ','){
      strncpy(temp,p,(e-p));
      gps_dat.Geoid_Sep = atof(temp);
    }
    p = e+1;
    memset(temp,0x00,15);
    //Get Geoidal unit
    e = strchr(p, ',');
    
    if (e != p) gps_dat.Geoid_Unit = *(p);
    gps_dat.CHk_Sum = chk_sum;
  }    
  
  gpsProcessPacket = false;
}

