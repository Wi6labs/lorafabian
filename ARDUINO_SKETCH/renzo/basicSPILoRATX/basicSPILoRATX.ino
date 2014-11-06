#include <SPI.h>

/*
Circuit:
 SS:   pin 10
 
 MOSI: pin 11  // Handled by SPI.H
 MISO: pin 12  // Handled by SPI.H
 SCK:  pin 13  // Handled by SPI.H
 
 */
 
// LoRa Shield Arduino commands
#define ARDUINO_CMD_AVAILABLE 0x00
#define ARDUINO_CMD_READ      0x01
#define ARDUINO_CMD_WRITE     0x02
#define ARDUINO_CMD_TEST      0x03 

// pins used for the connection with the sensor
// the other you need are controlled by the SPI library):

//#define SLAVESELECT 10//ss
const int CS = 10; // SS already defined by SPI conflicts with same pin nr 10 :)


/*  Minimum wait time between SPI message: 160 us
 *  Minimum wait time between two bytes in a SPI message: 15 us
 */
const int wait_time_us_between_spi_msg   = 200;
const int wait_time_us_between_bytes_spi = 20;

void setup() {
  Serial.begin(9600);


  pinMode(CS, OUTPUT); // we use this for SS pin
  
  // start the SPI library:
  SPI.begin();  // wake up the SPI bus.
  SPI.setClockDivider(SPI_CLOCK_DIV32) ; // 16MHz/32 = 0.5 MHz
  

  SPI.setDataMode(SPI_MODE0);  // By Default SPI_MODE0, we make this fact explicit.
  SPI.setBitOrder(MSBFIRST);   // By Default MSBFIRST,  we make this fact explicit.

  delay(100);
}

void loop() {

   arduinoLoraTXWriteTest() ;

   Serial.println("Going Sleep 5 secs");
   delay(5000);
   Serial.println("Woke up!");
}



int  arduinoLoraTXWriteTest() {
  return arduinoLoraTXWriteTest("Arduino is doing Lora!");
}

/* 
  String msg will be sent as a null therminated sring (\0)
*/
int arduinoLoraTXWriteTest(String msg) {
  
  int  msg_length = msg.length();
  byte msg_as_bytes[ msg_length ];
  msg.getBytes(msg_as_bytes, msg_length) ;
  
  return arduinoLoraTXWriteTest ( msg_as_bytes , msg_length );
  
}

int arduinoLoraTXWriteTest( byte buf[], int buf_len) {
  
  
  digitalWrite(CS, LOW);

    //Send: COMMAND Write
    int  previous_cmd_status  =  SPI.transfer(ARDUINO_CMD_WRITE); // The first result byte is the one in wich the shield puts the status of last command
    
    //Send:  Size of bytes to send
      delayMicroseconds(wait_time_us_between_bytes_spi);
    SPI.transfer(buf_len >> 8);
          delayMicroseconds(wait_time_us_between_bytes_spi);
    SPI.transfer(buf_len);
    
    //Send:  payload as bytes to send
    for (int i = 0; i < buf_len ; i ++)
    {
        delayMicroseconds(wait_time_us_between_bytes_spi);
        SPI.transfer(buf[ i ]); 
    }
  
 
  digitalWrite(CS, HIGH);
  
  return previous_cmd_status;
  
  
}

