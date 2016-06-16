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


Description: Status led init and set functions.
-----------------------------------------------------------------------------*/                                                             
#include <stdint.h>
#include <stdio.h>
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"


#define STATUS_LED_TX_PORT					GPIOC
#define STATUS_LED_TX_PORT_CLK			RCC_AHBPeriph_GPIOC
#define STATUS_LED_TX_PIN						GPIO_Pin_8

#define STATUS_LED_RX_PORT					GPIOC
#define STATUS_LED_RX_PORT_CLK			RCC_AHBPeriph_GPIOC
#define STATUS_LED_RX_PIN						GPIO_Pin_9

#define STATUS_LED_SYNC_PORT        GPIOC
#define STATUS_LED_SYNC_PORT_CLK    RCC_AHBPeriph_GPIOC
#define STATUS_LED_SYNC_PIN         GPIO_Pin_10

void status_led_tx_on(bool on){
	if (on)
		GPIO_ResetBits(STATUS_LED_TX_PORT, STATUS_LED_TX_PIN);
	else
		GPIO_SetBits(STATUS_LED_TX_PORT, STATUS_LED_TX_PIN);
 }

void status_led_rx_on(bool on){
	if (on)
		GPIO_ResetBits(STATUS_LED_RX_PORT, STATUS_LED_RX_PIN);
	else
		GPIO_SetBits(STATUS_LED_RX_PORT, STATUS_LED_RX_PIN);
 }

void status_led_sync_on(bool on){
	if (on)
		GPIO_ResetBits(STATUS_LED_SYNC_PORT, STATUS_LED_SYNC_PIN);
	else
		GPIO_SetBits(STATUS_LED_SYNC_PORT, STATUS_LED_SYNC_PIN);
 }



void status_led_init( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd( STATUS_LED_TX_PORT_CLK | STATUS_LED_RX_PORT_CLK, ENABLE );

	/* GPIO configuration ------------------------------------------------------*/
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

	GPIO_InitStructure.GPIO_Pin = STATUS_LED_TX_PIN;
	GPIO_Init( STATUS_LED_TX_PORT, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = STATUS_LED_RX_PIN;
	GPIO_Init( STATUS_LED_RX_PORT, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = STATUS_LED_SYNC_PIN;
	GPIO_Init( STATUS_LED_SYNC_PORT, &GPIO_InitStructure );

	status_led_rx_on(FALSE);
	status_led_tx_on(FALSE);

}
