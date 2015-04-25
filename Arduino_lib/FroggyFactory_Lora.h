/**************************************************************************/
/*! 
    @file     FroggyFactory_Lora.h
    @author   SOSAndroid.fr (E. Ha.)
	
    @section  HISTORY

    v0.1 - First beta release
	v0.2 - Adding flexibility for init() and capability to get Frequency setting & RFConf

    This library allows to talk to the Froggy Factory LoRa shield for Arduino - Wi6labs
	
    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2014, SOSAndroid.fr (E. Ha.)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#ifndef _FROGGYFACTORY_LORA_H_
#define _FROGGYFACTORY_LORA_H_

	#if ARDUINO >= 100
		#include <Arduino.h>
	#else
		#include <WProgram.h>
	#endif

	#include <SPI.h>


	#define BOARD MCU16 	// MCU8 || MCU16 || DUE - defining the MCU clock speed used allows to set the SPI timing
							// please have a look on http://www.arduino.cc/en/Products.Compare
							// The MCU frequency must be divided to get a 500 KHz SPI frequency
	#define SS_PIN 10
	#define SS_PIN_ACTIVE LOW
	#define SPI_DATA_MODE SPI_MODE0

	#ifdef BOARD == MCU8
		#define SPI_CLK_DIVIDER SPI_CLOCK_DIV16
	#endif
	#ifdef BOARD == MCU16
		#define SPI_CLK_DIVIDER SPI_CLOCK_DIV32
	#endif
	/**** //DUE support removed at the moment
	#ifdef BOARD == DUE
		#define SPI_CLK_DIVIDER 168
	#endif
	*****/

	#define CMD_NULL 0x00
	#define CMD_READ 0x01
	#define CMD_SEND 0x02
	#define CMD_RFCFG 0x05
	#define CMD_FREQ 0x04
	#define CMD_SNR 0x06
	#define CMD_RSSI 0x07

	#define WAIT_SPI_UP 500
	#define WAIT_SPI_TRFRT 1
	#define WAIT_SPI_SS_OFF 100

	#define FREQ_8680 868000000
	#define FREQ_8681 868100000
	#define FREQ_8682 868200000
	#define FREQ_8683 868300000
	#define FREQ_8695 869500000
	#define FREQ_8696 869600000
	#define FREQ_8698 869800000
	#define FREQ_8699 869900000
	#define FREQ_DEFAULT FREQ_8680
	#define FREQ_MIN 863000000
	#define FREQ_MAX 870000000
	
	#define RFCONF_DEFAULT 3




	class FROGGYFACTORY_LORA {

		public:
			FROGGYFACTORY_LORA(void);
			FROGGYFACTORY_LORA(uint8_t ss_pin, long freq);
			
			void	lora_init(boolean doRFsetting = true);
			void	lora_rfConf(uint8_t cfg);
			void	lora_setFreq(long freq);
			void	lora_setChannelFreq(uint8_t channel);
			int		lora_available(void);
			void	lora_send(String data);
			char	lora_read(void);
			int		lora_getSnr(void);
			int		lora_getRssi(void);
			long	lora_getFreq(void);
			uint8_t	lora_getRFConf(void);
			
		private:
		
			long	_frequency;
			uint8_t	_ss_pin;
			uint8_t _rfconf;
			
	};
#endif