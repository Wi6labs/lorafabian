// inslude the SPI library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;

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

  int previous_cmd_status;
  int shield_status;
  int available_msb;
  int available_lsb;
  int available;
  int read_data;
  int i;
  	 
 digitalWrite(slaveSelectPin,LOW);

  //  data available cmd
  previous_cmd_status = SPI.transfer(0x00);
  delay(1);
  shield_status = SPI.transfer(0x00);
  delay(1);
  available_msb = SPI.transfer(0x00);
  delay(1);
  available_lsb = SPI.transfer(0x00);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 


  available = (available_msb<<8) + available_lsb;

  if (available) {
    Serial.print("Message received, size:");
    Serial.print(available);
     Serial.print(" data: ");   
    delay(1);
    
     for (i=0; i<available; i++) {

       digitalWrite(slaveSelectPin,LOW);          
       
       //  read data
       previous_cmd_status = SPI.transfer(0x01);
       delay(1);
       shield_status = SPI.transfer(0x00);
       delay(1);
       read_data = SPI.transfer(0x00);
       delay(1);
       
       digitalWrite(slaveSelectPin,HIGH); 
       
      delay(1);
       Serial.print(read_data, HEX);   
     }
    Serial.println("");
  }
  	  
 delay(100);
}
