/**************************************************************************/
/*!
    @file     lora_receive.ino
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

	Transmit messages using LoRa Shield from Froggy Factory - Wi6labs
	
    @section  HISTORY

    v0.1 - First beta release
*/
/**************************************************************************/


#include <SPI.h>
#include <FroggyFactory_Lora.h>

#define LOOP_DELAY 5000
#define CSPIN 10
#define FREQ 868300000


FROGGYFACTORY_LORA myLoRaShield;
//FROGGYFACTORY_LORA myLoRaShield(CSPIN, FREQ); //alternative constructor call

int available_payload;
String default_payload = "FF LoRa Shield - ";
String payload;


void setup() {
	
	myLoRaShield.lora_init();
	
	//myLoRaShield.lora_rfConf(3); //done in init()
	//myLoRaShield.lora_setChannelFreq(255); //done in init() - 255 = default channel
	
	//Start serial always after LoRa Init
	Serial.begin(9600);
	while (!Serial) ; //wait until Serial ready
	
	Serial.println("Startup of LoRa transmitter...");
}

void loop() {
	payload = default_payload + analogRead(A0); //add random data to the string to be sent
	
	myLoRaShield.lora_send(payload);	
	delay(LOOP_DELAY);
}

