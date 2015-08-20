#include "LoraShield.h"
#include <SPI.h>

LoraShield lora;
void setup()
{
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect. Needed for Leonardo only
  
  Serial.println("Initialisation du Shield");
  lora.init();
  String host = "beta.s.ackl.io";
  Serial.println("Begin : " + host);
  lora.begin(host);
  Serial.println("Debug is off");
  lora.setContikiDebug(false);
}

void loop()
{
  /*delay(10000);
  byte tab[5] = {0x01,0x02,0x03,0x04,0x05};
  lora.write(tab,5);
  delay(10000);
  byte tab2[6] = {0x01,0x02,0x03,0x04,0x05,0x06};
  lora.write(tab2,6);*/
  /**/delay(2000);
  int pktsize = lora.dataAvailable();
  if (pktsize > 0) {
    String msg = lora.read(true);
    if(msg != "") Serial.println("\nMessage: " + msg);
  }
  else Serial.println("0");/**/
}
