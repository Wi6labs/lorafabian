// inslude the SPI library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int SPIslavePin = 10;

void setup() {
  // set the SPIslavePin as an output:
  pinMode (SPIslavePin, OUTPUT);
  digitalWrite(SPIslavePin,HIGH); 
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV32); 

 
// Wait to be sure SPI is initialised
  delay(1000);
// Change RF config

digitalWrite(SPIslavePin,LOW);
  //  data available cmd
  SPI.transfer(0x05);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(SPIslavePin,HIGH); 
 

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
  int snr;
  int rssi;
  	 
 digitalWrite(SPIslavePin,LOW);

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
  digitalWrite(SPIslavePin,HIGH); 


  available = (available_msb<<8) + available_lsb;

  if (available) {
    Serial.print("Message received, size:");
    Serial.print(available);
    Serial.print(" data: ");
    delay(1);
    
    byte data_rcv[available];
    
     for (i=0; i<available; i++) {

       digitalWrite(SPIslavePin,LOW);          
       
       //  read data
       previous_cmd_status = SPI.transfer(0x01);
       delayMicroseconds(wait_time_us_between_bytes_spi);
       shield_status = SPI.transfer(0x00);
       delayMicroseconds(wait_time_us_between_bytes_spi);
       data_rcv[i] = SPI.transfer(0x00);
       delayMicroseconds(wait_time_us_between_bytes_spi);
       
       
       digitalWrite(SPIslavePin,HIGH); 
       delayMicroseconds(wait_time_us_between_spi_msg);
       
          
     }
    
    PrintHex8(data_rcv , available); // Serial.println( data_rcv[i] , HEX); println as HEX does not include trailing zeroes! 0x00 gets printed as '0'
    Serial.println("");
     snr  = LoRa_last_snr();
     rssi = LoRa_last_rssi();

    if (available == 5 && data_rcv[0] == 'l' && data_rcv[1] == 'o' && data_rcv[2] == 'r' && data_rcv[3] == 'a' && data_rcv[4] == 's' ) {      
       delay(5000); 
 
       // send response
         digitalWrite(SPIslavePin,LOW);

      //  send data
      SPI.transfer(0x02);
      delay(1);
      SPI.transfer(0x00);
      delay(1);
      SPI.transfer(0x9);
      delay(1);
      SPI.transfer('l');
      delay(1);
      SPI.transfer('o');
      delay(1);
      SPI.transfer('r');
      delay(1);
      SPI.transfer('a');
      delay(1);
       SPI.transfer('r');
      delay(1);
      SPI.transfer((snr >> 8) & 0xFF);
      delay(1);
      SPI.transfer(snr & 0xFF);
      delay(1);
      SPI.transfer((rssi >> 8) & 0xFF);
      delay(1);
      SPI.transfer(rssi & 0xFF);
      delay(1);
  
      // take the SS pin high to de-select the chip:
      digitalWrite(SPIslavePin,HIGH);  

      delay(100); 

     
      
    }
   
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
