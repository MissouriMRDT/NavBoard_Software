#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "RoveEthernet.h"
#include "RoveComm.h"

#include <Adafruit_GPS.h>
//#include <SoftwareSerial.h>

const uint16_t GPS_FIX_QUALITY_DATA_ID = 301;
const uint16_t GPS_LAT_LON_DATA_ID = 302;
const uint16_t GPS_SPEED_DATA_ID = 303;
const uint16_t GPS_ANGLE_DATA_ID = 304;
const uint16_t GPS_ALTITUDE_DATA_ID = 305;
const uint16_t GPS_SATELLITES_DATA_ID = 306;

uint64_t gps_lat_lon = 0;

Adafruit_GPS GPS(&Serial7);
//SoftwareSerial mySerial(3, 2);

void setup()  
{   
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  //Serial.begin(115200);
  
  //connect to roveComm
  Ethernet.enableActivityLed();
  Ethernet.enableLinkLed();
  roveComm_Begin(192,168,1,133);
  //Serial.println("roveComm_Begin");
  
  //9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  //Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate

  //Request updates on antenna status, comment out to keep quiet
  //GPS.sendCommand(PGCMD_ANTENNA);

}//end 

uint32_t timer = millis();

void loop() 
{
  uint16_t data_id = 0;
  size_t data_size = 0;
  uint16_t data = 0;
  roveComm_GetMsg(&data_id, &data_size, &data);
  //delay(300);
  
  //int16_t msg = 0;
  //roveComm_SendMsg(301, sizeof(msg), &msg);
  //delay(300);
  
  //Serial.print("Looping");
  //delay(1);
  
  char c = GPS.read();
  
  delay(1);
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
  
    if (!GPS.parse(GPS.lastNMEA() ) )// this also sets the newNMEAreceived() flag to false
    {   
      return;  // we can fail to parse a sentence in which case we should just wait for another
    }//end if
    
  }//end if

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) 
  {
    timer = millis();
  }//end if

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
   
    //debug
    /*
    GPS.fix = true;
    GPS.fixquality = 200;
    GPS.latitude_fixed = 407098514;
    GPS.longitude_fixed = -740079168;
    GPS.speed = 123.456;
    GPS.angle = 789.012;
    GPS.altitude = 345.678;
    GPS.satellites = 251;
   */
    if(!GPS.fix)
    {
      GPS.fixquality = 0;
    }//end if
    
    //Serial.print(" quality: "); Serial.println(GPS.fixquality);
    roveComm_SendMsg(GPS_FIX_QUALITY_DATA_ID, sizeof(GPS.fixquality), &GPS.fixquality);
    
    if (GPS.fix) 
    {  
      //TODO: VERIFY ADAFRUIT_GPS PULL #13
      gps_lat_lon = GPS.latitude_fixed;
      gps_lat_lon = gps_lat_lon << 32;
      gps_lat_lon += GPS.longitude_fixed;
      
      roveComm_SendMsg(GPS_LAT_LON_DATA_ID, sizeof(gps_lat_lon), &gps_lat_lon);
      
      //Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      roveComm_SendMsg(GPS_SPEED_DATA_ID, sizeof(GPS.speed), &GPS.speed);
      
      //Serial.print("Angle: "); Serial.println(GPS.angle);
      roveComm_SendMsg(GPS_ANGLE_DATA_ID, sizeof(GPS.angle), &GPS.angle);
      
      //Serial.print("Altitude: "); Serial.println(GPS.altitude);
      roveComm_SendMsg(GPS_ALTITUDE_DATA_ID, sizeof(GPS.altitude), &GPS.altitude);
      
      //Serial.print("Satellites: "); Serial.println(GPS.satellites);      
      roveComm_SendMsg(GPS_SATELLITES_DATA_ID, sizeof(GPS.satellites), &GPS.satellites);
    }//end if
  }//end if
}//end loop