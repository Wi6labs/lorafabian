/*--------------------------------------------------------------------------

              j]_                   .___                                   
._________    ]0Mm                .=]MM]=                                  
M]MM]MM]M]1  jMM]P               d]-' NM]i                                 
-~-~   4MM1  d]M]1              d]'   jM]'                                 
       j]MT .]M]01       d],  .M]'    d]#                                  
       d]M1 jM4M]1  .,  d]MM  d]I    .]M'                                  
       ]0]  M/j]0(  d]L NM]f d]P     jM-                                   
       M]M .]I]0M  _]MMi -' .]M'                                           
       M]0 jM MM]  jM-M>   .]M/                                            
       ]0F MT ]M]  M>      d]M1        .,                                  
      j0MT.]' M]M j]1 .mm .]MM ._d]_,   J,                                 
      jM]1jM  ]01 =] .]M/ jM]Fd]M]MM]   .'                                 
      j]M1#T .M]1.]1 jM]' M]0M/^ "M]MT  j         .",    .__,  _,-_        
      jMM\]' J]01jM  M]M .]0]P    ]0]1  i         1 1   .'  j .'  "1       
      j]MJ]  jM]1]P .]M1 jMMP     MM]1  I        J  t   1   j J    '       
      =M]dT  jM]q0' dM]  M]MT     ]MM  j        j   j  j    J 1            
      ]M]M`  j]0j#  ]MF  ]M]'    .M]P  J       .'   j  J  .J  4_,          
      M]0M   =MM]1 .M]'  MM]     jM](  1       r    j  1  --,   "!         
      ]0MT   ]M]M  jM@   ]M]     M]P  j       J     j j     4     1        
      MM]'   M]0P  j]1  .M]M    j]M'  J      j'     ",?     j     1        
     _]M]    M]0`  jM1 .MNMM,  .]M'   1     .'       11     1    j'        
     jM]1   jM]@   j]L_]'?M]M__MP'    \     J        1G    J    .'         
     j]0(   jM]1   "M]P'  "N]M/-      "L__J L________'?L__- *__,'          
     "-'    "--                                                            
                                                                           
----------------------------------------------------------------------------

Copyright (c) <2014>, <Wi6labs>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the wi6labs nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WI6LABS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Description: Simple example to test LoRa RX. It set sx1272 in receive mode on the
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

static struct etimer rx_timer;

static u8 rx_msg[256];	

void coap_beacon_send_response(){

	//5045525D-C0FF6E6F-64653D36-36
	uint8_t databuff_coap[] = {	0x50,  // 0101 0000 . Version 01 , Type Non-Confirmable: 1. Token Lenght: 0
								0x45,  // Code: 2.05 . Content (69)
								0x52,0x5d, // TODO: Message ID (must match received)
								0xc0, // Opt #1: Content-Format; text/plain. Opt 12. 1100 (C=12) 0000 (0 Lenght)
								0xff, // End of options marker (255)
								// The payload text-plain
								'n','o','d','e','=','6','6'
	};

	// TODO: encapsulate in 802.15.4 Frame? or its contiki work?

	lora_radio_driver.send(databuff_coap, sizeof(databuff_coap));

}


void respond_if_coap_beacon( u8 rx_msg[] , int size){

	int mac_header_size = 15;
	int iaux;

	if (size >= mac_header_size ) {
		printf("Size OK");
	} else{
		printf("Size is NOT OK");
		return;
	}

	/* 41c8d11010ffffaa666b00000000015001525db46e5f6964 // beacon coap
	 * 41c8c21010ffff3636323866606600610000004dbf0600ec170000e803000000000000000000000000000000000000 // oed frame
	 * 41c8aa1010ffff363632385953540071000600a0aa0f00000000000a00000000000000000000000000000000000000 // oed frame
	 * 41c8c31010ffff3636323866606600610000008ebf0600ec170000e803000000000000000000000000000000000000 // oed frame
	 * 41c8d11010ffffaa666b00000000015001525db46e5f6964 // beacon coap
	 * 41c8ab1010ffff363632385953540071000600e1aa0f00000000000a00000000000000000000000000000000000000 // oed frame
	 *
	 */


	// 1 ) Analyse 802.15.4 HEADER
	printf("\n\r\tHeader FCF (41C8)?: %02x%02x", rx_msg[0], rx_msg[1]);
	printf("\n\r\tHeader SEQ (dont care): %02x", rx_msg[2]);

	// 2 ) Analyse 802.15.4 PANID
	printf("\n\r\tPanID (must be 1101?): %02x%02x", rx_msg[3], rx_msg[4]);
	
	// 3 ) Addresses
	printf("\n\r\tADDR Dest: %02x%02x", rx_msg[5], rx_msg[6]);
	printf("\n\r\tADDR Src: ");
	for (iaux = 0 ; iaux < 8; iaux++ ) printf("%02x", rx_msg[7+iaux]);

	// 4 ) Check CoAP message
	printf("\n\r\tPayload: ");
	for (iaux= mac_header_size ; iaux< size; iaux++ ) printf("%02x", rx_msg[iaux]);

	printf("\n\r\tPailod is CoAP Beacon?: ");

	/* CoAP Packet  to send */
	/* Constrained Application Protocol, Non-Confirmable, GET, MID:21085 ( 0x525d )*/

	uint8_t databuff_coap[] = {0x50,0x01,0x52,0x5d,0xb4,0x6e,0x5f,0x69,0x64};
	unsigned int  databuff_coap_size = 9;

	if ( ( size - mac_header_size ) == (databuff_coap_size) ){

		int comp = memcmp(  & (rx_msg[mac_header_size]) , databuff_coap , databuff_coap_size);
		if ( comp == 0 ){
			printf("This is the LoRA CoAP Beacon");

			// Here we should send this to another non blocking thread (or maybe blocking is preferable ? for not use the radio at same time)
			// and implement a Collision avoidance algoritm (some random time), or rely on ContikiMAC?

			coap_beacon_send_response();

		}else{
			printf("Not LoRa Beacon. Same length but other bytes.");
		}

	} else{
		printf("Not LoRa Beacon. Not same length");
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
