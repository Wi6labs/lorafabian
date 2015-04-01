// inslude the SPI library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;
int counter = 0;

const int buttonPin = 2;
int buttonState = 0; 
int last_buttonState = 0; 

void setup() {
  pinMode(buttonPin, INPUT); 
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin,HIGH); 
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV32);


// Wait to be sure SPI is initialised
  delay(1000);
// Change RF config

digitalWrite(slaveSelectPin,LOW);

  //  Change RF config
  SPI.transfer(0x05);
  delay(1);  
  SPI.transfer(0x00);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  SPI.transfer(0x01);
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 

  
  delay(10);
 
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
 int response;
char msg[5];
int msg_len;
char rssi;
char snr;

  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

 if (buttonState && last_buttonState!= buttonState) {

  last_buttonState = buttonState;
  
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
   SPI.transfer('s');
  delay(1);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH);  

delay(100); 

// Check response
response = 100;

while (response) {
   response --;
    
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

  delay(100);

   if (available) {
     // get out the while loop
     response = 0;
     
     
    Serial.print("Message received, size:");
    Serial.print(available);
     Serial.print(" data: ");  
    
    msg_len = 0;
     
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
       
       if (msg_len < 5){ msg[msg_len] = read_data; }
  //   Serial.println(msg[msg_len]); 
     msg_len++;
              
      delay(1);
       Serial.print(read_data, HEX);   
       
     }
       if (msg_len == 5 && msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'r' && msg[3] == 'a' && msg[4] == 'r' ) {
           Serial.println("Test OK");
       }
       else {
           Serial.println("Test KO");         
       }

      delay(100);     
      
       //  read SNR
       digitalWrite(slaveSelectPin,LOW);          
 
       previous_cmd_status = SPI.transfer(0x06);
       delay(1);
       shield_status = SPI.transfer(0x00);
       delay(1);
       snr = SPI.transfer(0x00);
       delay(1);
       
       digitalWrite(slaveSelectPin,HIGH); 
       
      delay(100);     

       // read RSSI
        digitalWrite(slaveSelectPin,LOW);          
 
       previous_cmd_status = SPI.transfer(0x07);
       delay(1);
       shield_status = SPI.transfer(0x00);
       delay(1);
       rssi = SPI.transfer(0x00);
       delay(1);
       
       digitalWrite(slaveSelectPin,HIGH); 
      
      Serial.print("RSSI : ");
       Serial.print(int(rssi));
       Serial.print("  SNR : ");
       Serial.println(int( snr));
      delay(100);     
  }
}
//Serial.print("Out response ");
//Serial.println(response);

}
else if ( !buttonState && last_buttonState!= buttonState ){

  last_buttonState = buttonState;
  
delay(100);  
 
} 
//  delay(1000); 
 
}

