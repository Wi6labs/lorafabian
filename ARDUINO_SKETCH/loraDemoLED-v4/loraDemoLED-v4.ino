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
  lora.getContikiDebug(false);
}

void loop()
{
  delay(2000);
  int pktsize = lora.dataAvailable();
  if (pktsize > 0) {
    String msg = lora.read(true);
    if(msg != "") Serial.println("\nMessage: " + msg);
  }
  else Serial.println("0");
}
