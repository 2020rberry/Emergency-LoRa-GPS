/*
    This file is the code that should be uploaded to all client modules of your LoRa network.  All of these settings are for
    an Arduino Uno, with three buttons connected to ports 3, 4, and 5.
*/
#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

RH_RF95 rf95;
TinyGPS gps;
float ffinaladdition;
char cfinal [1];
float fnode;
char cnode [1];
uint8_t dataoutgoing[50];

void setup()
{
    //CHANGE THESE VALUES IF YOUR BUTTONS ARE PLUGGED INTO DIFFERENT PORTS THAN MINE
    pinMode(3, INPUT); 
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    
    Serial.begin(9600);
    if (!rf95.init())
      Serial.println("init failed");
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x0c  // Reg 0x26: LowDataRate=On, Agc=On
    };
    rf95.setModemRegisters(&modem_config);
}

void loop()
{ 
  while(digitalRead(3) == LOW && digitalRead(4) == LOW && digitalRead(5) == LOW)
  {}
  if(digitalRead(3) == HIGH)
  {
    ffinaladdition = 1;
  }
  else if(digitalRead(4) == HIGH)
  {
    ffinaladdition = 2;
  }
  else
  {
    ffinaladdition = 3;
  }
  Serial.println("Sending to rf95_server");
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial.available())
    {
      char c = Serial.read();
      if (gps.encode(c)) 
      newData = true;
    }
  }
  if (newData)
  {
    float flat, flon;
    fnode = 1;
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    //This block of Serial.print()s is really not needed, but if you want to run the client off of a computer and 
    //check the GPS output, leave it in.  
    Serial.print("LAT=");
    Serial.print(flat, 4);
    Serial.print(" LON=");
    Serial.println(flon, 4);

    char clat [10];
    char clon [10];
    dtostrf(flat, 9, 4, clat);
    dtostrf(flon, 9, 4, clon); 
    dtostrf(ffinaladdition, 1, 0, cfinal);

    strcat(clat, ", ");
    strcat(clat, clon);
    strcat(clat, " : ");
    strcat(clat, cfinal);
    dtostrf(fnode, 1, 0, cnode);
    strcat(clat, cnode);
    Serial.println(clat);
    strcpy((char *)dataoutgoing,clat); 
    rf95.send(dataoutgoing, sizeof(dataoutgoing));
   
    uint8_t indatabuf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(indatabuf);

    if (rf95.waitAvailableTimeout(3000))
     { 
       if (rf95.recv(indatabuf, &len))
      {
         Serial.print("got reply: ");
         Serial.println((char*)indatabuf);
      }
      else
      {
      Serial.println("recv failed");
      }
    }
    else
    {
      Serial.println("No reply, is rf95_server running?");
    }
  delay(400);
  
 }
}
