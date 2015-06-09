/**************************************************************************/
/*!
    @file     lora_repeater.ino
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

	Receive messages on one channel and repeat then on another channel using LoRa shields from Froggy Factory - Wi6labs
	
    @section  HISTORY

    v0.1 - First beta release
*/
/**************************************************************************/


#include <SPI.h>
#include <FroggyFactory_Lora.h>

#define CHANNEL_RCV 0
#define CHANNEL_REPT 7
#define RFCONF_DEFAULT 3


FROGGYFACTORY_LORA myLoRaShield;

int available_payload;
String message;


void setup() {
	
	myLoRaShield.lora_init(false);
	myLoRaShield.lora_rfConf(RFCONF_DEFAULT);
	myLoRaShield.lora_setChannelFreq(CHANNEL_RCV);
	
	//Start serial always after LoRa Init
	Serial.begin(9600);
	while (!Serial) ; //wait until Serial ready
	
	Serial.println("Startup of LoRa Repeater...");
}

void loop() {

	available_payload = myLoRaShield.lora_available();
	
	if (available_payload) {
		lora_getMessage(available_payload);
		lora_repeat();
	}
	
}

void lora_getMessage(int available_payload) {

		Serial.print("Bytes received: ");
		Serial.println(available_payload, DEC);
		Serial.print("Payload: ");
		
		for (int i=0; i < available_payload; i++) {
			message += myLoRaShield.lora_read();
		}
		
		Serial.print(message);
		Serial.println("   ---PAYLOAD END");
		
		lora_snrrssi();

	return;
}

void lora_repeat(void) {

		myLoRaShield.lora_setChannelFreq(CHANNEL_REPT);
		myLoRaShield.lora_send(message);
		Serial.println("---PAYLOAD SENT---");	
		
		message = "";
		myLoRaShield.lora_setChannelFreq(CHANNEL_RCV);
	
	return;
}

void lora_snrrssi(void) {

		Serial.print("SNR: ");
		Serial.print (myLoRaShield.lora_getSnr());
		Serial.print(" | RSSI: ");
		Serial.println (myLoRaShield.lora_getRssi());
	
	return;
}
