/**************************************************************************/
/*!
    @file     FroggyFactory_Lora.cpp
    @author   SOSAndroid (E. Ha.)
    @license  BSD (see license.txt)

    @section  HISTORY

    v0.1 - First beta release

    This library allows to talk to the Froggy Factory LoRa shield for Arduino - Wi6labs
*/
/**************************************************************************/

#include <SPI.h>
#include "FroggyFactory_Lora.h"

/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
    Constructor
*/
/**************************************************************************/
FROGGYFACTORY_LORA::FROGGYFACTORY_LORA(void) 
{
	_frequency = FREQ_DEFAULT;
	_ss_pin = SS_PIN;
}

FROGGYFACTORY_LORA::FROGGYFACTORY_LORA(uint8_t ss_pin, long freq) 
{
	_frequency = freq;
	_ss_pin = ss_pin;
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    @brief  init communication with the shield and RF conf and frequency
    
    @params 
				none
	@returns
				none
*/
/**************************************************************************/

void FROGGYFACTORY_LORA::lora_init(void) {

	pinMode (_ss_pin, OUTPUT);
	digitalWrite(_ss_pin, !SS_PIN_ACTIVE); 

	// initialize SPI:
	SPI.begin(); 
	
	/******
	#if BOARD == DUE
		SPI.setDataMode(_ss_pin, SPI_DATA_MODE);
		SPI.setClockDivider(_ss_pin, SPI_CLK_DIVIDER);	
	#else
		SPI.setDataMode(SPI_DATA_MODE);
		SPI.setClockDivider(SPI_CLK_DIVIDER);	
	#endif
	*******/
	
	SPI.setDataMode(SPI_DATA_MODE);
	SPI.setClockDivider(SPI_CLK_DIVIDER);

	delay(WAIT_SPI_UP);
	
	FROGGYFACTORY_LORA::lora_rfConf(RFCONF_DEFAULT);
	FROGGYFACTORY_LORA::lora_setFreq(_frequency);
	
	
	return;
}

/**************************************************************************/
/*!
    @brief  Set radio configuration
    
    @params 	uint8_t cfg
				from 0 to 3
				0 is the fastest but less robust
				3 is the slowest but more robust (better distance)
				
	@returns
				none
*/
/**************************************************************************/

void FROGGYFACTORY_LORA::lora_rfConf(uint8_t cfg) {

	if ((cfg >= 0) && (cfg <=3)) {
		digitalWrite(_ss_pin, SS_PIN_ACTIVE);
		
		// CFG cmd
		SPI.transfer(CMD_RFCFG);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer(CMD_NULL);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer(CMD_READ);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer(cfg);
		delay(WAIT_SPI_TRFRT);
		
		digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
		delay(WAIT_SPI_SS_OFF);
	
	}
	return;
}

/**************************************************************************/
/*!
    @brief  Set frequency
    
    @params 	long freq
				input is in Hz
				Range is 863 000 000 to 870 000 000
				Be careful of the band width used at band limits
				
	@returns
				none
*/
/**************************************************************************/

void FROGGYFACTORY_LORA::lora_setFreq(long freq) {

	if ((freq >= FREQ_MIN) && (freq <= FREQ_MAX)) {
		digitalWrite(_ss_pin, SS_PIN_ACTIVE);
		
		// CFG cmd
		SPI.transfer(CMD_FREQ);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer(CMD_NULL);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer(CMD_FREQ);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer((freq>>24) & 0xFF);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer((freq>>16) & 0xFF);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer((freq>>8) & 0xFF);
		delay(WAIT_SPI_TRFRT);
		SPI.transfer(freq & 0xFF);
		delay(WAIT_SPI_TRFRT);
		
		digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
		delay(WAIT_SPI_SS_OFF);
	}
	return;
}

/**************************************************************************/
/*!
    @brief  Set one out of 8 predifiened frequency
    
    @params 	uint8_t channel)
				from 0 to 7
				
	@returns
				none
*/
/**************************************************************************/

void FROGGYFACTORY_LORA::lora_setChannelFreq(uint8_t channel) {
	
	switch (channel) {
		case 0:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8680);
			break;
		case 1:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8681);
			break;
		case 2:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8682);
			break;
		case 3:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8683);
			break;
		case 4:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8695);
			break;
		case 5:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8696);
			break;
		case 6:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8698);
			break;
		case 7:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_8699);
			break;
		default:
			FROGGYFACTORY_LORA::lora_setFreq(FREQ_DEFAULT);
			break;		
	}
	return;
}

/**************************************************************************/
/*!
    @brief  Check the number of bytes received & ready to be read
    
    @params 	
				none
				
	@returns
				Number of bytes to read as int
*/
/**************************************************************************/

int FROGGYFACTORY_LORA::lora_available(void) {

	int available_msb;
	int available_lsb;
	
	digitalWrite(_ss_pin, SS_PIN_ACTIVE);
	
	//  data available cmd
	SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	available_msb = SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	available_lsb = SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);	

	digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
	delay(WAIT_SPI_SS_OFF);

	return (available_msb<<8) + available_lsb;
}

/**************************************************************************/
/*!
    @brief  Sends a String Object through the LoRa transciever
    
    @params 	
				String
				
	@returns
				none
*/
/**************************************************************************/

void FROGGYFACTORY_LORA::lora_send(String data) {

	int len = data.length();

	digitalWrite(_ss_pin, SS_PIN_ACTIVE);
	
	//send data
	SPI.transfer(CMD_SEND);
	delay(WAIT_SPI_TRFRT);
	SPI.transfer(len >> 8);
	delay(WAIT_SPI_TRFRT);
	SPI.transfer(len & 0xFF);
	delay(WAIT_SPI_TRFRT);
	
	for (int i=0; i<len; i++) {
		SPI.transfer(data.charAt(i));
		delay(WAIT_SPI_TRFRT);
	}

	digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
	delay(WAIT_SPI_SS_OFF);

	return;
}

/**************************************************************************/
/*!
    @brief  Reads a char from received buffer
    
    @params 	
				none
				
	@returns
				char
*/
/**************************************************************************/

char FROGGYFACTORY_LORA::lora_read(void) {
	
	byte c;
	
	digitalWrite(_ss_pin, SS_PIN_ACTIVE);

	//read data
	SPI.transfer(CMD_READ);
	delay(WAIT_SPI_TRFRT);
	SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	c = SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	
	digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
	delay(WAIT_SPI_SS_OFF);
	
	return c;
}

/**************************************************************************/
/*!
    @brief  Reads SNR value
    
    @params 	
				none
				
	@returns
				SNR as int
*/
/**************************************************************************/

int FROGGYFACTORY_LORA::lora_getSnr(void) {

	byte snr;
	
	digitalWrite(_ss_pin, SS_PIN_ACTIVE);

    //  read SNR
	SPI.transfer(CMD_SNR);
	delay(WAIT_SPI_TRFRT);
	SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	snr = SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	
	digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
	delay(WAIT_SPI_SS_OFF);
	
	return (int) snr;
}

/**************************************************************************/
/*!
    @brief  Reads RSSI value
    
    @params 	
				none
				
	@returns
				RSSI as int
*/
/**************************************************************************/

int FROGGYFACTORY_LORA::lora_getRssi(void) {

	byte rssi;
	
	digitalWrite(_ss_pin, SS_PIN_ACTIVE);

    //  read Rssi
	SPI.transfer(CMD_RSSI);
	delay(WAIT_SPI_TRFRT);
	SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	rssi = SPI.transfer(CMD_NULL);
	delay(WAIT_SPI_TRFRT);
	
	digitalWrite(_ss_pin, !SS_PIN_ACTIVE);
	delay(WAIT_SPI_SS_OFF);
	
	return (int) rssi;
}