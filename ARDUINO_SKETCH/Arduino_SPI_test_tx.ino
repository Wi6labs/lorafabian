// inslude the SPI library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
int counter = 0;

void setup() {
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  Serial.begin(9600);
}

void loop() {
  
 digitalWrite(slaveSelectPin,LOW);

  //  send data
  SPI.transfer(0x02);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x05);
  delay(1);
  SPI.transfer(0x1A);
  delay(1);
  SPI.transfer(0xBF);
  delay(1);
  SPI.transfer(0xAB);
  delay(1);
  SPI.transfer(counter >> 8);
  delay(1);
   SPI.transfer(counter & 0xFF);
  delay(1);
 
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
    
  Serial.print("Message send, counter:");
  Serial.println(counter);
 
  counter ++;  

 delay(10000);
}
