#include <SPI.h>
const int slaveSelectPin = 10;
const byte ResetPin          = 5; 


void LoRa_init() {
 
  // Reset The LoRa modem
  pinMode (ResetPin, OUTPUT);
  digitalWrite(ResetPin,HIGH); 
  delay(100);
  digitalWrite(ResetPin,LOW); 
  delay(50);
  digitalWrite(ResetPin,HIGH); 
  // Let it wake up
  delay(500);
  
  pinMode (slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin,HIGH); 
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV128);

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
  // CFG cmd
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

// Change radio frequency
// input is in Hz
// Range is 863 000 000 to 870 000 000
// Be careful of the band width used at band limits
void LoRa_freq(long f) {
  digitalWrite(slaveSelectPin,LOW);
  //  FREQ cmd
  SPI.transfer(0x04);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x04);
  delay(1);
  SPI.transfer((f>>24) & 0xFF);
  delay(1);
  SPI.transfer((f>>16) & 0xFF);
  delay(1);
  SPI.transfer((f>>8) & 0xFF);
  delay(1);
  SPI.transfer(f & 0xFF);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  delay(100);
  
}


// Pre implmented radio frequency channel configuration
// input is channel number
void LoRa_freq_channel(int nb) {
  switch(nb) {
    case 0:
      LoRa_freq(868000000);
    break;
    case 1:
      LoRa_freq(868100000);
    break;
    case 2:
      LoRa_freq(868200000);
    break;
    case 3:
      LoRa_freq(868300000);
    break;
    case 4:
      LoRa_freq(869500000);
    break;
    case 5:
      LoRa_freq(869600000);
    break;
    case 6:
      LoRa_freq(869800000);
    break;
    case 7:
      LoRa_freq(869900000);
    break;



  }


}


// Get Signal to Noise Ratio from la message received
int LoRa_last_snr(){
       char snr;

       //  read SNR
       digitalWrite(slaveSelectPin,LOW);          
 
       SPI.transfer(0x06);
       delay(1);
       SPI.transfer(0x00);
       delay(1);
       snr = SPI.transfer(0x00);
       delay(1);
       
       digitalWrite(slaveSelectPin,HIGH); 
       
      delay(100);     
 
       return int(snr);  
}

// Get Received signal strength indication from la message received
int LoRa_last_rssi(){
    char rssi;
        //  read RSSI
       digitalWrite(slaveSelectPin,LOW);          
 
      SPI.transfer(0x07);
       delay(1);
       SPI.transfer(0x00);
       delay(1);
       rssi = SPI.transfer(0x00);
       delay(1);
       
       digitalWrite(slaveSelectPin,HIGH); 
       
      delay(100);     
 
       return int(rssi);  
}
   
// Set Spreading Factor
void LoRa_SF(int in) {
  digitalWrite(slaveSelectPin,LOW);
  
  // CFG cmd
  SPI.transfer(0x08);
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

// Set Band Width
void LoRa_BW(int in) {
  digitalWrite(slaveSelectPin,LOW);
  
  // CFG cmd
  SPI.transfer(0x09);
  
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

// Set Coding Rate
void LoRa_CR(int in) {


  digitalWrite(slaveSelectPin,LOW);
  // CFG cmd
  SPI.transfer(0x0A);
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

// Set TX Power
void LoRa_TX_Power(int in) {

  digitalWrite(slaveSelectPin,LOW);
  // CFG cmd
  SPI.transfer(0x0B);
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

