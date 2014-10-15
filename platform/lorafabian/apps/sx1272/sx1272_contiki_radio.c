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


Description: Contiki radio interfec implementation for sx1272
-----------------------------------------------------------------------------*/                                                             
#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "dev/radio.h"
#include "sx1272_contiki_radio.h"
#include "sx1272_radio.h"
#include "arduino_spi.h"
#include "status_led.h"
#include "sx1272.h"

#define RF_FREQUENCY                                868100000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm
#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             3         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  FALSE
#define LORA_IQ_INVERSION_ON                        FALSE

#define LORA_CLEAR_CHANNEL_RSSI_THRESHOLD           (-90)     // RSSI threshold in dBm for CSMA

static int pending_packets = 0;

static uint8_t *rx_msg_ptr = NULL;
static uint16_t rx_msg_size = 0;

static int tx_ongoing =0;

static RadioEvents_t RadioEvents;
void OnTxDone( void )
{
	 // Reset RF state
	 lora_radio_driver.off();
	 printf("TX sent\n\r");
   tx_ongoing =0;
   status_led_tx_on(FALSE);
#ifdef LORAFAB_RESTART_RX_AFTER_TX
		lora_radio_driver.on();
		printf("Restart RX\n\r");
#endif

}
void OnRxDone( uint8_t *payload, uint16_t size, int8_t rssi, int8_t snr )
{
   int i;
   
	 rx_msg_ptr = payload;
	 rx_msg_size = size;
	  
	 status_led_rx_on(TRUE);

	 // indicate buffer to arduino interface
	 set_arduino_read_buf(payload, size);
	
	 pending_packets ++;

		if (pending_packets > 1) {
//			printf("WARNING: Previous RX msg lost\n\r");
		
			pending_packets = 1;
		}
	 
	 printf("RX received, size: %d RSSI: %d, SNR: %d\n\r", size, rssi, snr);
	 for (i = 0; i<size; i++) 
		 printf("%02x", payload[i] );

	 printf("\n\r");

//   printf("REG_LR_FIFOADDRPTR %d REG_LR_FIFORXCURRENTADDR %d\n\r", SX1272Read(REG_LR_FIFOADDRPTR), SX1272Read(REG_LR_FIFORXCURRENTADDR) ) ;

 	 // Reset RX to redo startup calibrations
	 lora_radio_driver.off();
   lora_radio_driver.on();
	
}

void OnTxTimeout( void )
{
	printf("TX timeout\n\r");
	tx_ongoing =0;

#ifdef LORAFAB_RESTART_RX_AFTER_TX
		lora_radio_driver.on();
#endif
}

void OnRxTimeout( void )
{
}

void OnRxError( void )
{
 	
}



static struct etimer et_reset_rx;
PROCESS(rx_reset_process, "rx_reset_process");

PROCESS_THREAD(rx_reset_process, ev, data)
{
  PROCESS_BEGIN();



  while( 1 )
  {

    PROCESS_WAIT_EVENT();

    if(ev == PROCESS_EVENT_TIMER) {

			printf( "Reset RX\n\r");
      // Reset RX to redo startup calibrations
	    lora_radio_driver.off();
      lora_radio_driver.on();
	
    }
  }
  PROCESS_END();
}


/*---------------------------------------------------------------------------*/
/*                        CONTIKI INTERFACE                                  */
/*---------------------------------------------------------------------------*/


static int
init(void)
{

  // Radio initialization
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;
	
	Radio.Init(&RadioEvents);

  Radio.SetChannel( RF_FREQUENCY );

  Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                 LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                 LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 TRUE, LORA_IQ_INVERSION_ON, TRUE );

  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 TRUE, LORA_IQ_INVERSION_ON, 3000000 );

  printf("Lora radio initialized with Freq %d TX output pwr %d, BW %d, SF %d, CR %d\n\r",
          RF_FREQUENCY, TX_OUTPUT_POWER, LORA_BANDWIDTH,
          LORA_SPREADING_FACTOR, LORA_CODINGRATE); 

	pending_packets = 0;

  process_start(&rx_reset_process, NULL);


return 1;
}

/*---------------------------------------------------------------------------*/
static int
radio_on(void)
{
	// Start infinite RX	
  Radio.Rx( 0 );
	// Start reset timer
	 etimer_set(&et_reset_rx, RESET_RX_DURATION );
	 // assign it to correct process
	 et_reset_rx.p = &rx_reset_process;

  return 1;
}
/*---------------------------------------------------------------------------*/
static int
radio_off(void)
{
	// stop reset timer
	etimer_stop(&et_reset_rx); 
	Radio.Standby();
	return 1;
}

/*---------------------------------------------------------------------------*/
static int
radio_read(void *buf, unsigned short bufsize)
{
	if (pending_packets > 0) {
	  pending_packets --;

	  if (bufsize < rx_msg_size) {
			printf("Error, rx buffer size too small\n\r");
			 return 0;
		}
		else {
			memcpy(buf, rx_msg_ptr, rx_msg_size);
			return rx_msg_size;
		}

	
	}

  return 0;
}

/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
	return Radio.IsChannelFree(MODEM_LORA, RF_FREQUENCY, LORA_CLEAR_CHANNEL_RSSI_THRESHOLD);
}
/*---------------------------------------------------------------------------*/
static uint8_t *tx_msg_ptr = NULL;
static uint16_t tx_msg_size = 0;
static int
prepare_packet(const void *data, unsigned short len)
{
	tx_msg_ptr = (uint8_t *) data;
	tx_msg_size = len;
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
transmit_packet(unsigned short len)
{
		if (tx_msg_ptr != NULL && tx_msg_size != 0) {
			if (!tx_ongoing) {
				status_led_tx_on(TRUE);
			  Radio.Send( tx_msg_ptr,  tx_msg_size);

				tx_ongoing =1;
			}
			else {
				printf("ERROR: TX already ongoing, message discarded\n\r");
			}
		}
		else {
			printf("ERROR: TX message empty\n\r");
		}


  return 0;
}
/*---------------------------------------------------------------------------*/
static int
radio_send(const void *payload, unsigned short payload_len)
{
	prepare_packet(payload, payload_len);
	transmit_packet(payload_len);

  return RADIO_TX_OK;

}
/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
  return pending_packets;
}





const struct radio_driver lora_radio_driver = {
    init,
    prepare_packet,
    transmit_packet,
    radio_send,
    radio_read,
    channel_clear,
    receiving_packet,
    pending_packet,
    radio_on,
    radio_off,
};

