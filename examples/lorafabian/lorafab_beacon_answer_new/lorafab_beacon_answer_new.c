/*--------------------------------------------------------------------------

Copyright (c) <2015>, <Wi6labs> ,  <Telcom Bretagne>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Telecom Bretagne, wi6labs nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL TELECOM BRETAGNE OR WI6LABS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Description:
LoRaFabian Beacon Answer code. LoRa RX setq sx1272 in receive mode on the
channel configured in sx1272_contiki_radio.c

-----------------------------------------------------------------------------*/                                                             

#include "contiki.h"
#include "leds-arch.h"
#include "stm32f10x.h"
#include <dev/leds.h>
#include <stdio.h> /* For printf() */
//#include "sx1272_radio.h"
#include "sx1272_contiki_radio.h"

#include "arduino_spi.h"

#include "er-coap.h"
#include "er-coap-constants.h"

static struct etimer rx_timer;

static u8 rx_msg[256];
static uint8_t tx_buffer[512];
int tx_buffer_index;

// Modifi here: the MAC and the reply to coap_payload_beacon (the response)
uint8_t header_802_15_4_hardcoded[] = {0x22,0x27,0x01,0xfa,0x81, // Don touch this
							0xfa,0xba,0x00,0x00,0x00,0x00,0x00,0x03}; // This is our 8Byte MAC. TODO Frame real 80215

char coap_payload_beacon[] = "{\"n\":\"gamma.s.ackl.io\"}";

void coap_beacon_send_response(){

	// TODO: encapsulate in 802.15.4 Frame? or its contiki work?
	tx_buffer_index = 0;

	// 802154 FRAME HEADER
	//uint8_t header_802_15_4[] = header_802_15_4_hardcoded;

	memcpy(tx_buffer, header_802_15_4_hardcoded , sizeof(header_802_15_4_hardcoded));
	tx_buffer_index += sizeof(header_802_15_4_hardcoded);

	// COAP
	//coap_packet_t coap_packet;
	size_t coap_packet_size;
	static coap_packet_t coap_request[1];      /* This way the packet can be treated as pointer as usual. */

	coap_init_message(coap_request, COAP_TYPE_NON, COAP_POST, coap_get_mid()); // random_rand();



	//char coap_payload[] = coap_payload_beacon;
	coap_set_payload(coap_request, (uint8_t *)coap_payload_beacon, sizeof(coap_payload_beacon) - 1);

	coap_packet_size = coap_serialize_message(coap_request, (void *)(tx_buffer + tx_buffer_index));
	tx_buffer_index += coap_packet_size;
	printf("We are sending the response to the coap beacon\n");
	int iaux;
	for (iaux = 0 ; iaux < tx_buffer_index; iaux++ ) printf("%02x", tx_buffer[iaux]);

	lora_radio_driver.send(tx_buffer, tx_buffer_index);

}


void respond_if_coap_beacon( u8 rx_msg[] , int size){

	int mac_header_size = 13;
	int iaux;

	if (size >= mac_header_size ) {
		printf("Size OK");
	} else{
		printf("Size is NOT OK");
		return;
	}

	// 1 ) Analyse 802.15.4 HEADER
	printf("\n\r\tHeader FCF (2227)?: %02x%02x", rx_msg[0], rx_msg[1]);
	printf("\n\r\tHeader SEQ (dont care): %02x", rx_msg[2]);

	// 2 ) Analyse 802.15.4 PANID
	//printf("\n\r\tPanID (must be FAB1): %02x%02x", rx_msg[3], rx_msg[4]);
	
	// 3 ) Addresses
	printf("\n\r\tADDR Dest: ");
	for (iaux = 0 ; iaux < 8; iaux++ ) printf("%02x", rx_msg[3+iaux]);
	printf("\n\r\tADDR Src: ");
	for (iaux = 0 ; iaux < 2; iaux++ ) printf("%02x", rx_msg[11+iaux]);

	// 4 ) Check CoAP message
	printf("\n\r\tPayload: ");
	for (iaux= mac_header_size ; iaux< size; iaux++ ) printf("%02x", rx_msg[iaux]);

	printf("\n\r\tPailod is CoAP Beacon?: ");

	 /* static declaration reduces stack peaks and program code size */
	static coap_packet_t coap_message[1]; /* this way the packet can be treated as pointer as usual */
	erbium_status_code = NO_ERROR;

	erbium_status_code =
	coap_parse_message(coap_message, (void *)(rx_msg + mac_header_size), size - mac_header_size);
	if(erbium_status_code == NO_ERROR) {
		int check_coap_beacon = (coap_message->type == COAP_TYPE_NON) && ((coap_message->code == COAP_POST)
								&& (coap_message->payload_len == 0));
		if ( check_coap_beacon ){
			printf("This is the LoRA CoAP Beacon");

			// Here we should send this to another non blocking thread (or maybe blocking is preferable ? for not use the radio at same time)
			// and implement a Collision avoidance algoritm (some random time), or rely on ContikiMAC?

			coap_beacon_send_response();

		}else{
			printf("Not LoRa Beacon. Other CoAP Message.");
		}


	} else{
		printf("Not LoRa Beacon. CoAP parse ERROR");
	}





	printf("\n\r");

}

	/*---------------------------------------------------------------------------*/
PROCESS(lorafab_bcn_process, "LoRaFabian Beacon process");
AUTOSTART_PROCESSES(&lorafab_bcn_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(lorafab_bcn_process, ev, data)
{
  PROCESS_BEGIN();

	int pending = 0;
	int i;
	int size;

  leds_init();
  leds_toggle(LEDS_ALL); 

	arduino_spi_init();

	lora_radio_driver.init();

	// Start infinite RX	
	lora_radio_driver.on();

		
	etimer_set(&rx_timer, 5 * CLOCK_SECOND);

  while( 1 )
  {

    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {
			leds_toggle(LEDS_ALL);

			pending = lora_radio_driver.pending_packet();
			printf("pending_packet: %d\n\r", pending );

			if (pending) {
				size = lora_radio_driver.read(rx_msg, sizeof(rx_msg));
				printf("rx_msg, size: %d\n\r", size);
				for (i = 0; i<size; i++) 
					printf("%02x", rx_msg[i] );

				printf("\n\r");


				respond_if_coap_beacon(rx_msg , size);
				//coap_beacon_send_response(); // for testing
			}


			//			printf( "test1\n\r");

			etimer_reset(&rx_timer);
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
