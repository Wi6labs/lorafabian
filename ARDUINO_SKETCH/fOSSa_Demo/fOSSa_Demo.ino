#include <ht1632c.h>
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;

ht1632c dotmatrix = ht1632c(&PORTD, 7, 6, 4, 5, GEOM_32x16, 2);

void setup() {
  dotmatrix.clear();
  
  // Brightness from 0 to 15
  dotmatrix.pwm(15);
  dotmatrix.putchar(13, 4, '0', ORANGE);
  dotmatrix.sendframe();

     
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
  SPI.begin(); 
  SPI.setDataMode(SPI_MODE0) ;
  SPI.setClockDivider(SPI_CLOCK_DIV32); 

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

 
  Serial.begin(9600);

}
int x =0;

int count = 0;

void loop() {
char msg[5];
int msg_len;
/*    
    dotmatrix.clear(); 
x = 0;
    dotmatrix.putchar(x, 0, 'W', RED); x+=5;
    dotmatrix.putchar(x, 0, 'i', RED); x+=5;
    dotmatrix.putchar(x, 0, '6', RED); x+=5;
    dotmatrix.putchar(x, 0, 'l', RED); x+=5;
    dotmatrix.putchar(x, 0, 'a', RED); x+=5;
    dotmatrix.putchar(x, 0, 'b', RED); x+=5;
    dotmatrix.putchar(x, 0, 's', RED); x+=5;

    dotmatrix.sendframe();
    
    delay(100);
    
 //   dotmatrix.clear(); 
    x = 0;   
      dotmatrix.putchar(x, 9, 'R', GREEN); x+=6;  
      dotmatrix.putchar(x, 9, 'O', GREEN); x+=6;  
      dotmatrix.putchar(x, 9, 'C', GREEN); x+=6;  
      dotmatrix.putchar(x, 9, 'K', GREEN); x+=6;  
      dotmatrix.putchar(x, 9, 'S', GREEN); x+=6;  
      dotmatrix.putchar(x, 9, '!', GREEN); x+=6;  
  
     dotmatrix.sendframe();
             
        delay(100);    
 */
/*
uint16_t busSprite[9] = { 0x00fc, 0x0186, 0x01fe, 0x0102, 0x0102, 0x01fe, 0x017a, 0x01fe, 0x0084}; 
uint16_t bikeSprite[9] = { 0x020c, 0x0102, 0x008c, 0x00f8, 0x078e, 0x0ab9, 0x0bd5, 0x0891, 0x070e};
// emails app sprite:
dotmatrix.clear();
      dotmatrix.putbitmap(x, 0, bikeSprite, 16, 9, ORANGE); x+=1;
      
     dotmatrix.sendframe();
     
     if (x>32) x=-10;
     delay (200);
*/

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
     Serial.println(msg[msg_len]); 
     msg_len++;
              
      delay(1);
 //      Serial.print(read_data, HEX);   
       
     }
    Serial.println("");
    Serial.print ("len :"); Serial.println(msg_len);   
     Serial.println(msg);  
    
     if (msg_len == 5 && msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'r' && msg[3] == 'a' && msg[4] == 'y' ) {
   Serial.println("ON");

   /*   count ++;
          dotmatrix.clear();
          dotmatrix.putchar(13, 4, count +48, ORANGE);
          dotmatrix.sendframe();
          */
  dotmatrix.clear();
       x = 0;
    dotmatrix.putchar(x, 0, 'f', RED); x+=6;
    dotmatrix.putchar(x, 0, 'O', RED); x+=6;
    dotmatrix.putchar(x, 0, 'S', RED); x+=6;
    dotmatrix.putchar(x, 0, 'S', RED); x+=6;
    dotmatrix.putchar(x, 0, 'a', RED); x+=6;

    x = 0;
    dotmatrix.putchar(x, 8, '2', RED); x+=6;
    dotmatrix.putchar(x, 8, '0', RED); x+=6;
    dotmatrix.putchar(x, 8, '1', RED); x+=6;
    dotmatrix.putchar(x, 8, '4', RED); x+=6;
    dotmatrix.putchar(x, 8, '!', RED); x+=6;

    dotmatrix.sendframe();
             
          
          
     }
     else if (msg_len == 5 && msg[0] == 'l' && msg[1] == 'o' && msg[2] == 'r' && msg[3] == 'a' && msg[4] == 'n' ) {
  Serial.println("OFF");          dotmatrix.clear();
   
     }
    
  }
  	  
 delay(100);






}
