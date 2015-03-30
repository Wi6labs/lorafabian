//Include arduPi library
#include <stdlib.h>
#include <stdio.h>
#include "arduPi.h"
#include <time.h>

#define MAX_STRING 255
#define SEND_MSG_FILE "/var/www/lorafabian/msg2send.txt"
#define RECEIVED_MSG "/var/www/lorafabian/received_msg.txt"
#define LORA_MSG "LoraFabian:"


// TAG used to recognise our messages for this application
//const char LORA_MSG[]="LoraFabian:";

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;

void setup() 
{
  // set the slaveSelectPin as an output:
  // LBR  pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
  SPI.begin();
  // LBR
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0) ;
  //LBR  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setClockDivider(SPI_CLOCK_DIV512); // 500khz
  delayMicroseconds(100000);


  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  digitalWrite(slaveSelectPin,LOW);

  delayMicroseconds(100000);

  //  data available cmd, set config 0 for SF7, coding rate 2 (4/5), bandwith 0 (125kHz)
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
  delayMicroseconds(100000);
}

// send a message
// input parameter txt : the string message to send
void sendLoraMsg(char * txt) 
{
  // size of the message to send
  int len = strlen(txt);
  
  digitalWrite(slaveSelectPin,LOW);

  //  send data
  SPI.transfer(0x02);
  delay(1);
  SPI.transfer(0x00);
  delay(1);
  // len of string
  SPI.transfer(len);
  delay(1);

  for(int i=0; i<len; i++)
  {
    SPI.transfer(txt[i]);
    //printf("%c",txt[i]);
    delay(1);
  }
  //printf("\n");
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
  delay(100);
}

// Check if a message to send is available
void checkRequest() {
  time_t rawtime;
  struct tm * timeinfo;
  FILE* fichier = NULL;
  char chaine[MAX_STRING] = "";
  
  // open the which contains the message to send
  fichier = fopen(SEND_MSG_FILE, "r");
  
  // if file exists, send his content
  if (fichier != NULL)
  {
     // read the message in the first line of the file
    fgets(chaine, MAX_STRING, fichier); 
    // close the file
    fclose(fichier);
    // and delete it
    remove(SEND_MSG_FILE);
    
    // send the message
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ( "\n%s", asctime (timeinfo) );
    printf("Send Lora msg : \"%s\"\n",chaine);
    sendLoraMsg(chaine);
  }
}



// manage a received Lora message
void treatMsg(uint8_t *data,int len)
{
  char exe_cmd[MAX_STRING];
  char str[MAX_STRING];
  int lenTag;
  FILE *fp;

  // add null character
  data[len]=0;

  // check if the length of the message is higher the the TAG length
  lenTag = strlen(LORA_MSG);
  if (len<lenTag)
  {
    printf("Received message has not the Tag Lora Fabian : \"%s\"\n",(char*)data);
    return;
  }

  if (!strncmp(LORA_MSG,(char*)data,strlen(LORA_MSG)))
  {
    // the message contains the TAG, it's a Lora Fabian msg for us
    printf("Lora Fabian received msg : \"%s\"\n",(char*)data);
    fp=fopen(RECEIVED_MSG, "a");
    // copy the message at the end of the file received_msg.txt
    //sprintf(exe_cmd,"echo %s >> %s",(char*)data,RECEIVED_MSG);
    //system(exe_cmd);
    fprintf(fp, "%s\n",data);
    fclose(fp);
  }
  else
  {
    printf("Received message has not the Tag Lora Fabian : \"%s\"\n",(char*)data);
    //printf("Tag waited : %s\n", LORA_MSG);
  }
}


// the loop to receive messages
void loop() {
  int previous_cmd_status;
  int shield_status;
  int available_msb;
  int available_lsb;
  int available;
  int read_data;
  int i;
  time_t rawtime;
  struct tm * timeinfo;
  uint8_t data[MAX_STRING];

  digitalWrite(slaveSelectPin,LOW);

  delayMicroseconds(80);

  // data available cmd
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

  if (available) 
  {

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    printf ("\n%s", asctime (timeinfo) );
    printf("Message received, size=%d\n",available);
    delay(1);

    // loop for reading data
    for (i=0; i<available; i++) 
    {
      digitalWrite(slaveSelectPin,LOW);
      delayMicroseconds(80);
      // read data
      previous_cmd_status = SPI.transfer(0x01);
      delay(1);

      shield_status = SPI.transfer(0x00);
      delay(1);

      read_data = SPI.transfer(0x00);
      delay(1);
      digitalWrite(slaveSelectPin,HIGH);

      delay(1);
           
      data[i]=read_data;
    }
    treatMsg(data,available);
  }
  delay(100);
}


int main (){
  printf("Lora Fabian gateway running...\n");
  setup();
  while(1){
    loop();
    checkRequest();
  }
  return (0);
}
