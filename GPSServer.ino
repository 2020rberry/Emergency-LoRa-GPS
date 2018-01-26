/*
    This is the code to upload to the server side of the LoRa web.  It saves the coordinates and the severity level in 
    the Arduino's EEPROM
*/
#include <SPI.h>
#include <RH_RF95.h>
#include <EEPROM.h>
RH_RF95 rf95;
String dataString = "";
int address = 0;
char eepromCheck[50];
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

void setup() 
{
  Serial.begin(9600);
  if (!rf95.init())
    Serial.println("init failed");
  RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: LowDataRate=On, Agc=On
    };
  rf95.setModemRegisters(&modem_config);
  for(int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.write(i, -1);
  }
  Serial.println("Ready");
}

void loop()
{
  int count = 0;
  dataString="";
  if (rf95.available())
  {
    Serial.println("Get new message");
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      dataString += String((char*)buf);
      uint8_t data[] = "Coordinates Recieved";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.print("Coordinates: ");
      String coordinates = getCoordinates(dataString);
      String severity = getSeverity(dataString);
      String severityMessage = "";
      
      //CHANGE THE TEXT STRINGS IF YOU WANT IT TO DISPLAY SOMETHING OTHER THAN WHAT THE PRESETS ARE
      if(severity.equals("1"))
      {
        severityMessage = "Low on Supplies";
      }
      else if(severity.equals("2"))
      {
        severityMessage = "Need Medical Assistance";
      }
      else
      {
        severityMessage = "Everything ok";
      }
      
      
      Serial.println(coordinates);
      Serial.print("Status: ");
      Serial.println(severityMessage);
      Serial.print("Node: ");
      Serial.println(getNode(dataString));
      if(address < EEPROM.length())
      {
        Serial.println(len);
        for(int k = 0; k < len; k++)
        {
          EEPROM[address + k] = buf[k];
        }
        Serial.println("end");
        address = address + len;
      }
      else
      {
        Serial.println("EEPROM full");
      }
      Serial.println("");
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  
}
String getSeverity(String data)
{
  String temp = data.substring(data.indexOf(":") + 2, data.indexOf(":") + 3);
  temp.trim();
  return temp;
}
String getNode(String data)
{
  String temp = data.substring(data.indexOf(":") + 3);
  temp.trim();
  return temp;
}
String getCoordinates(String data)
{
  String temp = data.substring(0, data.indexOf(":"));
  temp.trim();
  return temp;
}
