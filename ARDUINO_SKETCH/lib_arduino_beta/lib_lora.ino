#include <SPI.h>
const int slaveSelectPin = 10;

void LoRa_init() {
  pinMode (slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin,HIGH); 
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  // Wait SPI to be ready
  delay(500);
}

void LoRa_send(String in) {

  int len = in.length();

 digitalWrite(slaveSelectPin,LOW);

  //  send data
  SPI.transfer(0x02);
  delay(1);
  SPI.transfer(len >> 8);
  delay(1);
  SPI.transfer(len & 0xFF);
  delay(1);
  for (int i=0; i<len; i++) {
    SPI.transfer(in.charAt(i));
  delay(1);
  }
 
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
 
  delay(100);

}

int LoRa_available(){
  int available_msb;
  int available_lsb;
  int available;

  digitalWrite(slaveSelectPin,LOW);

  //  data available cmd
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  available_msb = SPI.transfer(0x00);
  delay(1);
  available_lsb = SPI.transfer(0x00);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(1);

  available = (available_msb<<8) + available_lsb;

  return available;
}

char LoRa_read() {

  byte c;
  digitalWrite(slaveSelectPin,LOW);          

  //  read data
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  c = SPI.transfer(0x00);
  delay(1);


  digitalWrite(slaveSelectPin,HIGH); 
  delay(1);

  return c;
 
}

// Change radio configuration: 0 to 3
// 0 is the fastest but less robust
// 3 is the slowest but more robust (better distance)
void LoRa_RF_config(int in) {

  digitalWrite(slaveSelectPin,LOW);
  //  data available cmd
  SPI.transfer(0x05);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(in);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(100);
}
