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
#include "sx1272_contiki_radio.h"
#include "sx1272.h"

#include "arduino_spi.h"


// LoRa configs
#define LORA_CONFIG_NB 5
static u8 lora_bw[LORA_CONFIG_NB]               = {0, 1,  2,  2,  0};
static u8 lora_spreading_factor[LORA_CONFIG_NB] = {7, 9, 11, 12, 12};
static u8 lora_coding_rate[LORA_CONFIG_NB]      = {1, 2,  3,  4,  3};


static bool restart_rx = FALSE;

PROCESS(arduino_cmd_process, "arduino cmd process");

PROCESS_THREAD(arduino_cmd_process, ev, data)
{
	u16 len;
	u32 new_freq;
	u8 rf_cfg_index;
  u8 new_param;

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
						lora_radio_driver.off();
						// send packet
						lora_radio_driver.send(&arduino_cmd_buf[3], arduino_cmd_len - 3);
						// Turn On RX
						//						lora_radio_driver.on();
						set_last_cmd_status(ARDUINO_CMD_STATUS_OK);
					}


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
						lora_radio_driver.off();

						Radio.SetChannel( new_freq );

						lora_radio_driver.on();
					}
					else {
						Radio.SetChannel( new_freq );
					}

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);

					break;

				case ARDUINO_CMD_RF_CFG:
					
					restart_rx = FALSE;
					rf_cfg_index = arduino_cmd_buf[3];

					if (rf_cfg_index >= LORA_CONFIG_NB) {
						printf("Error RF config %d not allowed. Replaced by config 0\n\r", rf_cfg_index);
						rf_cfg_index = 0;
					}

					if ( SX1272GetStatus() == RF_RX_RUNNING ){
						lora_radio_driver.off();
						restart_rx = TRUE;
					}

					Radio.SetRxConfig( MODEM_LORA, lora_bw[rf_cfg_index], lora_spreading_factor[rf_cfg_index],
							lora_coding_rate[rf_cfg_index], 0, LORA_PREAMBLE_LENGTH,
							LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, TRUE );

					Radio.SetTxConfig( MODEM_LORA, lora_current_tx_power, 0, lora_bw[rf_cfg_index],
							lora_spreading_factor[rf_cfg_index], lora_coding_rate[rf_cfg_index],
							LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
							TRUE, LORA_IQ_INVERSION_ON, 3000000 );

					printf("Lora radio config changed with  BW %d, SF %d, CR %d\n\r",
							 lora_bw[rf_cfg_index], lora_spreading_factor[rf_cfg_index],
							lora_coding_rate[rf_cfg_index]); 

          // save current configs
          lora_current_sf       = lora_spreading_factor[rf_cfg_index];
          lora_current_bw       = lora_bw[rf_cfg_index];
          lora_current_cr       = lora_coding_rate[rf_cfg_index];

					set_last_cmd_status(ARDUINO_CMD_STATUS_OK);

					break;
				
        case ARDUINO_CMD_SF:
          new_param =  arduino_cmd_buf[3];
          if (new_param < 6 || new_param > 12) {
            printf("Spreading Factor out of range %d, must be between 6 and 12 included\n", new_param);   
            set_last_cmd_status(ARDUINO_CMD_STATUS_PARAM_OUT_OF_RANGE);
          }
          else {
            lora_current_sf = new_param;

            if ( SX1272GetStatus() == RF_RX_RUNNING ){
              lora_radio_driver.off();
              restart_rx = TRUE;
            }

            Radio.SetRxConfig( MODEM_LORA, lora_current_bw, lora_current_sf,
                lora_current_cr, 0, LORA_PREAMBLE_LENGTH,
                LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, TRUE );

            Radio.SetTxConfig( MODEM_LORA, lora_current_tx_power, 0, lora_current_bw,
                lora_current_sf, lora_current_cr,
                LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, 3000000 );

            printf("Spreading Factor changed: %d\n", lora_current_sf);   

            set_last_cmd_status(ARDUINO_CMD_STATUS_OK);

          }
          break;

          case ARDUINO_CMD_BW:
          new_param = arduino_cmd_buf[3];
          if (new_param < 0 || new_param > 2) {
            printf("LoRa Band width out of range %d, must be between 0 and 2 included\n", new_param);   
            set_last_cmd_status(ARDUINO_CMD_STATUS_PARAM_OUT_OF_RANGE);
          }
          else {
            lora_current_bw = new_param;

            if ( SX1272GetStatus() == RF_RX_RUNNING ){
              lora_radio_driver.off();
              restart_rx = TRUE;
            }

            Radio.SetRxConfig( MODEM_LORA, lora_current_bw, lora_current_sf,
                lora_current_cr, 0, LORA_PREAMBLE_LENGTH,
                LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, TRUE );

            Radio.SetTxConfig( MODEM_LORA, lora_current_tx_power, 0, lora_current_bw,
                lora_current_sf, lora_current_cr,
                LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, 3000000 );

            printf("Band Width changed: %d\n", lora_current_bw);   

            set_last_cmd_status(ARDUINO_CMD_STATUS_OK);

          }
          break;
        case ARDUINO_CMD_CR:
          new_param =  arduino_cmd_buf[3];
          if (new_param < 1 || new_param > 4) {
            printf("Coding Rate out of range %d, must be between 1 and 4 included\n", new_param);   
            set_last_cmd_status(ARDUINO_CMD_STATUS_PARAM_OUT_OF_RANGE);
          }
          else {
            lora_current_cr = new_param;
            
            if ( SX1272GetStatus() == RF_RX_RUNNING ){
              lora_radio_driver.off();
              restart_rx = TRUE;
            }
            
            Radio.SetRxConfig( MODEM_LORA, lora_current_bw, lora_current_sf,
                lora_current_cr, 0, LORA_PREAMBLE_LENGTH,
                LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, TRUE );

            Radio.SetTxConfig( MODEM_LORA, lora_current_tx_power, 0, lora_current_bw,
                lora_current_sf, lora_current_cr,
                LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, 3000000 );

            printf("Coding Rate changed: %d\n", lora_current_cr);   

            set_last_cmd_status(ARDUINO_CMD_STATUS_OK);

          }
        break;
        
        case ARDUINO_CMD_TX_POW:
          new_param =  arduino_cmd_buf[3];
          if (new_param < 2 || new_param > 14) {
            printf("TX Power out of range %d, must be between 2 and 14 included\n", new_param);   
            set_last_cmd_status(ARDUINO_CMD_STATUS_PARAM_OUT_OF_RANGE);
          }
          else {
            lora_current_tx_power = new_param;
            if ( SX1272GetStatus() == RF_RX_RUNNING ){
              lora_radio_driver.off();
              restart_rx = TRUE;
            }

            Radio.SetRxConfig( MODEM_LORA, lora_current_bw, lora_current_sf,
                lora_current_cr, 0, LORA_PREAMBLE_LENGTH,
                LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, TRUE );

            Radio.SetTxConfig( MODEM_LORA, lora_current_tx_power, 0, lora_current_bw,
                lora_current_sf, lora_current_cr,
                LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                TRUE, LORA_IQ_INVERSION_ON, 3000000 );

            printf("TX power changed: %d\n", lora_current_tx_power);   

            set_last_cmd_status(ARDUINO_CMD_STATUS_OK);

          }


        break;

 				default :
					printf("SPI Command unknown: %d\n\r", arduino_cmd_buf[0]);
					set_last_cmd_status(ARDUINO_CMD_STATUS_UNKNOWN);
					break;

			}

			if (restart_rx){
			  lora_radio_driver.on();
        restart_rx = FALSE;
			}

		}
  }
 
  PROCESS_END();
}


