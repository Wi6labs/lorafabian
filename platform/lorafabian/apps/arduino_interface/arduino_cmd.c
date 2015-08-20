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


Description: This process handle the commands send by the Arduino (Except READ and
AVAILABLE commands which are handled in the SPI interrupt)
-----------------------------------------------------------------------------*/                                                             
#include "contiki.h"
#include "stm32f10x.h"
#include <stdio.h> /* For printf() */
#include "sx1272_radio.h"
#include "sx1272_contiki_radio.h" //For Radio.SetRxConfig & SetTxConfig
#include "layer802154_radio_lora.h"
#include "frame802154_lora.h"
#include "sx1272.h"
#include "debug_on_arduino.h"

#include "arduino_spi.h"
#include "cfs/cfs.h"


// LoRa configs
#define LORA_CFGIG_NB 5
static u8 lora_bw[LORA_CFGIG_NB]               = {0, 1,  2,  2,  0};
static u8 lora_spreading_factor[LORA_CFGIG_NB] = {7, 9, 11, 12, 12};
static u8 lora_coding_rate[LORA_CFGIG_NB]      = {1, 2,  3,  4,  3};

int current_bw = LORA_BANDWIDTH, current_cr = LORA_CODINGRATE, current_sf = LORA_SPREADING_FACTOR;
u32 new_freq = RF_FREQUENCY;



PROCESS(arduino_cmd_process, "arduino cmd process");

PROCESS_THREAD(arduino_cmd_process, ev, data)
{
	u16 len;
	u8 rf_cfg_index;
	bool restart_rx = FALSE;

  PROCESS_BEGIN();
  while( 1 )
  {
    PROCESS_WAIT_EVENT();
    if(ev == PROCESS_EVENT_POLL) {
			switch (arduino_cmd_buf[0] ) {
				case ARDUINO_CMD_WRITE:
					len = (arduino_cmd_buf[1]<<8) + arduino_cmd_buf[2];
					if (len != arduino_cmd_len - 3) {
						printf("Error: write command length mismatch. SPI msg len: %d, Len fields: %d\n\r", arduino_cmd_len - 3, len);
						set_last_cmd_status(ARDUINO_CMD_STATUS_LENGTH_MISMATCH);
					}
					else {
						printf("Command WRITE received\n\r");
						// Turn off rx
						layer802154_off();
						// send packet to the broadcast (without the signalisation flag)
						//The gateway is hardcoded because we do not parse the beacon yet
						uint8_t destAddr[] = {0xfa,0x01,0x00,0x00,0x00,0x00,0x00,0x00};
						layer802154_send(&arduino_cmd_buf[3], arduino_cmd_len - 3, destAddr, SIGNALISATION_OFF, DST_SHORT_FLAG);
						// Turn On RX
						//						layer802154_on();
						set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					}
					break;



				case ARDUINO_CMD_HOSTNAME://We want to change the contiki hostname
					len = (arduino_cmd_buf[1]<<8) + arduino_cmd_buf[2];
					if (len != arduino_cmd_len - 3) {
						printf("Error: write command length mismatch. SPI msg len: %d, Len fields: %d\n\r", arduino_cmd_len - 3, len);
						set_last_cmd_status(ARDUINO_CMD_STATUS_LENGTH_MISMATCH);
					}
					else {
						printf("Command DNS received\n\r");
						//Write the new url in /HOSTNAME_LORA
						int size = arduino_cmd_len - 3;
						char dns[size];
						int fd;
						cfs_remove("/HOSTNAME_LORA");
						fd = cfs_open("/HOSTNAME_LORA", CFS_WRITE);
						if(fd >= 0) {
							//Get the hostname
							int i;
							for(i = 3; i < size+3; ++i)
								cfs_write(fd, &arduino_cmd_buf[i], sizeof(arduino_cmd_buf[i]));
							//here we write a space for the end.
							cfs_write(fd, '\0', sizeof('\0'));
							cfs_close(fd);
						}
						else {
							printf("ERREUR LORS DE L'ECRITURE\n\r");
						}
						set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					}
					break;


				case ARDUINO_CMD_DEBUG:
					printf("Command DEBUG received\n\r");
					int size = arduino_cmd_len;
					if(arduino_cmd_buf[size-1]) {
						debug_on_arduino = 1;
						printf("Debug on arduino is on\n\r");
					} else {
						debug_on_arduino = 0;
						printf("Debug on arduino is off\n\r");
					}
					break;

				case ARDUINO_CMD_GET_MAC:
					printf("Command GET_MAC received\n\r");
					char MAC[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
					getMac(MAC);
					set_arduino_read_buf(MAC, sizeof(MAC));
					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
				break;

				case ARDUINO_CMD_TEST:
					printf("Command Test received\n\r");
					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_FREQ:
					// compute new freq:
					new_freq = arduino_cmd_buf[3] << 24 | arduino_cmd_buf[4] << 16 |   arduino_cmd_buf[5] << 8 | arduino_cmd_buf[6];
					printf("Command FREQ received: %d Hz\n\r", new_freq);

					if ( SX1272GetStatus() == RF_RX_RUNNING ){
						layer802154_off();

						Radio.SetChannel( new_freq );

						layer802154_on();
					}
					else {
						Radio.SetChannel( new_freq );
					}

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_GET_FREQ:
					printf("Command GET_FREQ received\n\r");
					char freqArduino[] = {(new_freq >> 24), (new_freq >> 16), (new_freq >> 8), new_freq};
					set_arduino_read_buf(freqArduino, sizeof(freqArduino));
					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_RF_CFG:
					
					restart_rx = FALSE;
					rf_cfg_index = arduino_cmd_buf[3];

					if (rf_cfg_index >= LORA_CFGIG_NB) {
						printf("Error RF config %d not allowed. Replaced by config 0\n\r", rf_cfg_index);
						rf_cfg_index = 0;
					}

					if ( SX1272GetStatus() == RF_RX_RUNNING ){
						layer802154_off();
						restart_rx = TRUE;
					}

					Radio.SetRxConfig( MODEM_LORA, lora_bw[rf_cfg_index], lora_spreading_factor[rf_cfg_index],
							lora_coding_rate[rf_cfg_index], 0, LORA_PREAMBLE_LENGTH,
							LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, TRUE );

					Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, lora_bw[rf_cfg_index],
							lora_spreading_factor[rf_cfg_index], lora_coding_rate[rf_cfg_index],
							LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, 3000000 );
					current_bw = lora_bw[rf_cfg_index];
					current_sf = lora_spreading_factor[rf_cfg_index];
					current_cr = lora_coding_rate[rf_cfg_index];

					printf("Lora radio config changed with  BW %d, SF %d, CR %d\n\r",
							 lora_bw[rf_cfg_index], lora_spreading_factor[rf_cfg_index],
							lora_coding_rate[rf_cfg_index]); 

					if (restart_rx){
						layer802154_on();
					}

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_BW_CFG:
					
					current_bw = arduino_cmd_buf[3];
					printf("Command BW received: %d\n\r", current_bw);

					if ( SX1272GetStatus() == RF_RX_RUNNING ){
						layer802154_off();
						restart_rx = TRUE;
					}

					Radio.SetRxConfig( MODEM_LORA, current_bw, current_sf,
							current_cr, 0, LORA_PREAMBLE_LENGTH,
							LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, TRUE );

					Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0,
                                                           current_bw, current_sf, current_cr,
							LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, 3000000 );

					printf("Lora radio config changed with  BW %d, SF %d, CR %d\n\r",
							 current_bw, current_sf, current_cr); 

					if (restart_rx){
						layer802154_on();
					}

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_GET_BW_CFG:
					printf("Command GET_BW_CFG received\n\r");
					char bwArduino[] = {current_bw};
              				set_arduino_read_buf(bwArduino, sizeof(bwArduino));
					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_SF_CFG:
					
					current_sf = arduino_cmd_buf[3];
					printf("Command SF received: %d\n\r", current_sf);

					if ( SX1272GetStatus() == RF_RX_RUNNING ){
						layer802154_off();
						restart_rx = TRUE;
					}

					Radio.SetRxConfig( MODEM_LORA, current_bw, current_sf,
							current_cr, 0, LORA_PREAMBLE_LENGTH,
							LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, TRUE );

					Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0,
                                                           current_bw, current_sf, current_cr,
							LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, 3000000 );

					printf("Lora radio config changed with  BW %d, SF %d, CR %d\n\r",
							 current_bw, current_sf, current_cr); 

					if (restart_rx){
						layer802154_on();
					}

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_GET_SF_CFG:
					printf("Command GET_SF_CFG received\n\r");
					char sfArduino[] = {current_sf};
              				set_arduino_read_buf(sfArduino, sizeof(sfArduino));
					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_CR_CFG:
					current_cr = arduino_cmd_buf[3];
					printf("Command CR received: %d\n\r", current_cr);

					if ( SX1272GetStatus() == RF_RX_RUNNING ){
						layer802154_off();
						restart_rx = TRUE;
					}

					Radio.SetRxConfig( MODEM_LORA, current_bw, current_sf,
							current_cr, 0, LORA_PREAMBLE_LENGTH,
							LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, TRUE );

					Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0,
                                                           current_bw, current_sf, current_cr,
							LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, 3000000 );

					printf("Lora radio config changed with  BW %d, SF %d, CR %d\n\r",
							 current_bw, current_sf, current_cr); 

					if (restart_rx){
						layer802154_on();
					}

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				case ARDUINO_CMD_GET_CR_CFG:
					printf("Command GET_CR_CFG received\n\r");
					char crArduino[] = {current_cr};
              				set_arduino_read_buf(crArduino, sizeof(crArduino));
					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					break;

				default :
					printf("SPI Command unknown: %d\n\r", arduino_cmd_buf[0]);
					set_last_cmd_status(ARDUINO_CMD_STATUS_UNKNOWN);
					break;
			}
		}
  }
 
  PROCESS_END();
}


