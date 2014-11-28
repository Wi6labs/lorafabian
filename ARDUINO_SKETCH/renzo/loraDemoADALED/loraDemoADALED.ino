#include <SPI.h>

/*

Circuit:

 SS:   pin 10
 MOSI: pin 11
 MISO: pin 12
 SCK:  pin 13
 
 
 NeoPixelsLED: pin 7
 */
 
 #include <Adafruit_NeoPixel.h>

// Arduino commands
#define ARDUINO_CMD_AVAILABLE 0x00
#define ARDUINO_CMD_READ      0x01
#define ARDUINO_CMD_WRITE     0x02
#define ARDUINO_CMD_TEST      0x03 

//#define SLAVESELECT 10 // == SS == CS
const int CS  = 10;
const int LED = 7 ;


// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            6
#define NUMPIXELS      64

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);



/*  Minimum wait time between SPI message: 160 us
 *  Minimum wait time between two bytes in a SPI message: 15 us
 */
const int wait_time_us_between_spi_msg   = 200;
const int wait_time_us_between_bytes_spi = 20;

void setup() {
  
  
  Serial.begin(9600);

  pinMode(LED, OUTPUT); // led pinn
  pinMode(CS, OUTPUT); // we use this for SS pin
  
  // start the SPI library:
  SPI.begin();  // wake up the SPI bus.
  SPI.setClockDivider(SPI_CLOCK_DIV32) ; // 16MHz/32 = 0.5 MHz
  

  SPI.setDataMode(SPI_MODE0);  // By Default SPI_MODE0, we make this fact explicit.
  SPI.setBitOrder(MSBFIRST);   // By Default MSBFIRST,  we make this fact explicit.

  pixels.begin(); // This initializes the NeoPixel library.
  
  delay(100);
}

int previous_cmd_status;
int shield_status;
int available_msb;
int available_lsb;
int available;

  
void loop() {

  available = checkifAvailableRXData() ;

  if (available){
    
    Serial.print("Message received, size:");
    Serial.print(available);
    

    byte data_rcv[available];
    readAvailableRXData(data_rcv , available);
    
    Serial.print(" data: ");
    String dataAsString = Hex8ToString(data_rcv , available); // Serial.println( data_rcv[i] , HEX); println as HEX does not include trailing zeroes! 0x00 gets printed as '0'       
    Serial.print (dataAsString);
    
    analyzeAndActOnRXData( dataAsString);
    
    Serial.println("");
    
    
     
  }

  delay(250);
  
}


int ada_led_delayval = 50; // delay for 50 ms

void led_off()
{
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, 0); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(ada_led_delayval); // Delay for a period of time (in milliseconds).
  }
}

void led_on()
{
  digitalWrite(LED, HIGH);    // turn the LED off by making the voltage LOW
  
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    //pixels.setPixelColor(i, pixels.Color(35, 5 , 0 )); // Moderately bright red color.
    pixels.setPixelColor(i, pixels.Color(35 * (i%2), 5 , 35 * (1 - i%2)));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(ada_led_delayval); // Delay for a period of time (in milliseconds).
  }
}

// As String is the double size of bytes. This is for convenience for the LoRA DEMO
const String str_802_15_4_coap_led_off = "41c8d1fab10001aa666b00000000015001958fb96e6f64655f30303031036c6564036f6666";
const String str_802_15_4_coap_led_on  = "41c8d1fab10001aa666b00000000015001eb3cb96e6f64655f30303031036c6564026f6e";

void analyzeAndActOnRXData( String  dataAsString)
{
  if (dataAsString.equals(str_802_15_4_coap_led_off)) {

    Serial.print(": LED OFF!!");
    led_off();
    

    
  }else if(dataAsString.equals(str_802_15_4_coap_led_on)){
    
    Serial.print(":LED ON!!");
    led_on();
    
  }else
  {
    Serial.print(": Unknown MSG");
  }
  
}


void readAvailableRXData(byte buf[] , int buf_len) {

  for (int i=0; i<buf_len; i++) {

    digitalWrite(CS,LOW);          
       
      //  READ BYTE CMD
      previous_cmd_status = SPI.transfer(ARDUINO_CMD_READ);
        delayMicroseconds(wait_time_us_between_bytes_spi);
      shield_status = SPI.transfer(0x00);
        delayMicroseconds(wait_time_us_between_bytes_spi);
      // Store the received byte
      buf[i] = SPI.transfer(0x00);
        delayMicroseconds(wait_time_us_between_bytes_spi);
       
    digitalWrite(CS,HIGH);
      
    delayMicroseconds(wait_time_us_between_spi_msg);
     
  }

  
}  



int checkifAvailableRXData() {
  
  int tmp = 0;
  
  digitalWrite(CS, LOW);

    // data available cmd
    previous_cmd_status = SPI.transfer(ARDUINO_CMD_AVAILABLE);
      delayMicroseconds(wait_time_us_between_bytes_spi);
    shield_status = SPI.transfer(0x00);
      delayMicroseconds(wait_time_us_between_bytes_spi);  
    available_msb = SPI.transfer(0x00);
      delayMicroseconds(wait_time_us_between_bytes_spi); 
    available_lsb = SPI.transfer(0x00);
      delayMicroseconds(wait_time_us_between_bytes_spi);
  
  // take the SS pin high to de-select the chip:
  digitalWrite(CS,HIGH); 


  tmp = (available_msb<<8) + available_lsb;
  
  return tmp;
  
  
}



String Hex8ToString(uint8_t *data, uint8_t length) // prints 8-bit data in hex
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
  tmp[length*2] = 0; // Null terminated!
  return String(tmp);
}

