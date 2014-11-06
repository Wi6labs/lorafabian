// inslude the SPI library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
int counter = 0;

const int buttonPin = 2;
int buttonState = 0; 

void setup() {
  pinMode(buttonPin, INPUT); 
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  Serial.begin(9600);

  delay(100);
// Change RF config

digitalWrite(slaveSelectPin,LOW);

  //  data available cmd
  SPI.transfer(0x05);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  
  delay(10);
 
 
}

void loop() {
 // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

 if (buttonState) {
 digitalWrite(slaveSelectPin,LOW);

  //  send data
  SPI.transfer(0x02);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x5);
  delay(1);
  SPI.transfer('l');
  delay(1);
  SPI.transfer('o');
  delay(1);
  SPI.transfer('r');
  delay(1);
  SPI.transfer('a');
  delay(1);
   SPI.transfer('y');
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);  

}
else {
digitalWrite(slaveSelectPin,LOW);

  //  send data
  SPI.transfer(0x02);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x5);
  delay(1);
  SPI.transfer('l');
  delay(1);
  SPI.transfer('o');
  delay(1);
  SPI.transfer('r');
  delay(1);
  SPI.transfer('a');
  delay(1);
  SPI.transfer('n');
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);  
 
 
} 
  delay(1000); 
 
}
