/**************************************************************************/
/*!
    @file     lora_receive.ino
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

	Receive messages using LoRa Shield from Froggy Factory - Wi6labs
	
    @section  HISTORY

    v0.1 - First beta release
*/
/**************************************************************************/


#include <SPI.h>
#include <FroggyFactory_Lora.h>

#define LOOP_DELAY 100
#define CSPIN 10
#define FREQ 868300000


FROGGYFACTORY_LORA myLoRaShield;
//FROGGYFACTORY_LORA myLoRaShield(CSPIN, FREQ); //alternative constructor call

int available_payload;


void setup() {
	
	myLoRaShield.lora_init();
	
	//myLoRaShield.lora_rfConf(3); //done in init()
	//myLoRaShield.lora_setChannelFreq(255); //done in init() - 255 = default channel
	
	//Start serial always after LoRa Init
	Serial.begin(9600);
	while (!Serial) ; //wait until Serial ready
	
	Serial.println("Startup of LoRa Receiver...");
}

void loop() {

	available_payload = myLoRaShield.lora_available();
	
	if (available_payload) {
		Serial.print("Bytes received: ");
		Serial.println(available_payload, DEC);
		Serial.print("Payload: ");
		
		for (int i=0; i < available_payload; i++) {
			Serial.print(myLoRaShield.lora_read());
		}
		Serial.println("   ---PAYLOAD END");
	}
	
	Serial.print("SNR: ");
	Serial.print (myLoRaShield.lora_getSnr());
	Serial.print(" | RSSI: ");
	Serial.println (myLoRaShield.lora_getRssi());
	
	delay(LOOP_DELAY);
}

