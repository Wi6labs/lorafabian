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


/*  Minimum wait time between SPI messages: 160 us
 *  Minimum wait time between two bytes in a SPI message: 15 us
 */
const int wait_time_us_between_spi_msg   = 200;
const int wait_time_us_between_bytes_spi = 20;

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
    
    byte data_rcv[available];
    
     for (i=0; i<available; i++) {

       digitalWrite(slaveSelectPin,LOW);          
       
       //  read data
       previous_cmd_status = SPI.transfer(0x01);
       delayMicroseconds(wait_time_us_between_bytes_spi);
       shield_status = SPI.transfer(0x00);
       delayMicroseconds(wait_time_us_between_bytes_spi);
       data_rcv[i] = SPI.transfer(0x00);
       delayMicroseconds(wait_time_us_between_bytes_spi);
       
       
       digitalWrite(slaveSelectPin,HIGH); 
       delayMicroseconds(wait_time_us_between_spi_msg);
       
          
     }
    
    PrintHex8(data_rcv , available); // Serial.println( data_rcv[i] , HEX); println as HEX does not include trailing zeroes! 0x00 gets printed as '0'
      
   
    Serial.println("");
  }
  	  
 delay(100);
}

void PrintHex8(uint8_t *data, uint8_t length) // prints 8-bit data in hex
{
  char tmp[length*2+1];
  byte first ;
  int j=0;
  for (uint8_t i=0; i<length; i++) 
  {
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    j++;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39; 
    else tmp[j] = first;
    j++;
  }
  tmp[length*2] = 0;
  Serial.print(tmp);
}
