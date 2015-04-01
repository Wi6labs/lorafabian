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


Description: Arduino SPI interface implementation 
-----------------------------------------------------------------------------*/                                                             
#include <stdint.h>
#include <stdio.h>
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_nvic.h"
#include "contiki.h"
#include "arduino_spi.h"
#include "status_led.h"
#include "sx1272_contiki_radio.h"

#define SPI_INTERFACE                               SPI2
#define SPI_CLK                                     RCC_APB1Periph_SPI2
#define SPI_IRQn																		SPI2_IRQn

#define SPI_PIN_SCK_PORT                            GPIOB
#define SPI_PIN_SCK_PORT_CLK                        RCC_APB2Periph_GPIOB
#define SPI_PIN_SCK                                 GPIO_Pin_13

#define SPI_PIN_MISO_PORT                           GPIOB
#define SPI_PIN_MISO_PORT_CLK                       RCC_APB2Periph_GPIOB
#define SPI_PIN_MISO                                GPIO_Pin_14

#define SPI_PIN_MOSI_PORT                           GPIOB
#define SPI_PIN_MOSI_PORT_CLK                       RCC_APB2Periph_GPIOB
#define SPI_PIN_MOSI                                GPIO_Pin_15

#define SPI_PIN_NSS_PORT                            GPIOB
#define SPI_PIN_NSS_PORT_CLK                        RCC_APB2Periph_GPIOB
#define SPI_PIN_NSS                                 GPIO_Pin_12

void arduino_spi_init( void )
{
	NVIC_InitTypeDef NVIC_InitStructure;


	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	// Start process to be ready when SPI wakes up
	process_start(&arduino_cmd_process, NULL);

	/* Enable peripheral clocks --------------------------------------------------*/
	/* Enable SPIy clock and GPIO clock for SPIy */
	RCC_APB2PeriphClockCmd( SPI_PIN_MISO_PORT_CLK | SPI_PIN_MOSI_PORT_CLK |
			SPI_PIN_SCK_PORT_CLK, ENABLE );
	RCC_APB1PeriphClockCmd( SPI_CLK, ENABLE );

	/* GPIO configuration ------------------------------------------------------*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_InitStructure.GPIO_Pin = SPI_PIN_SCK;
	GPIO_Init( SPI_PIN_SCK_PORT, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = SPI_PIN_MOSI;
	GPIO_Init( SPI_PIN_MOSI_PORT, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin = SPI_PIN_NSS;
	GPIO_Init( SPI_PIN_MOSI_PORT, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SPI_PIN_MISO;
	GPIO_Init( SPI_PIN_MISO_PORT, &GPIO_InitStructure );

	// reset SPI
	SPI_I2S_DeInit(SPI_INTERFACE);

	/* SPI_INTERFACE Config -------------------------------------------------------------*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // 500kHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init( SPI_INTERFACE, &SPI_InitStructure );
	SPI_Cmd( SPI_INTERFACE, ENABLE );

	// Preload first data to be send to arduino	
	SPI_I2S_SendData(SPI_INTERFACE, ARDUINO_CMD_STATUS_NO_STATUS);
	/* Enable SPI_SLAVE RXNE interrupt */
	SPI_I2S_ITConfig(SPI_INTERFACE, SPI_I2S_IT_RXNE, ENABLE);


	// NVIC configuration

	/* 1 bit for pre-emption priority, 3 bits for subpriority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = SPI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

u8 arduino_cmd_buf[ARDUINO_CMD_BUF_MAX_LEN] = {0};
u16 arduino_cmd_len=0;
u8 shield_status;
u16 lora_data_available = 0 ;
u8 * arduino_read_buf = NULL;
u16 arduino_read_buf_len = 0;


void SPI2_IRQHandler(void)
{
	u8 cmd;
	u8 nss = 0;	
	int i;
	bool read_error_flag = FALSE;

	/* DISABLE RXNE interrupt while full message is received */
	SPI_I2S_ITConfig(SPI_INTERFACE, SPI_I2S_IT_RXNE, DISABLE);

	SPI_I2S_SendData(SPI_INTERFACE, shield_status);

	// init len
	arduino_cmd_len=0;
	// read command
	cmd = SPI_I2S_ReceiveData(SPI_INTERFACE);
	arduino_cmd_buf[arduino_cmd_len] = cmd; arduino_cmd_len++;	


	while (GPIO_ReadInputDataBit(SPI_PIN_NSS_PORT, SPI_PIN_NSS) == 0) {
		// Wait next data to be read
		while ( (SPI_I2S_GetFlagStatus(SPI_INTERFACE, SPI_I2S_FLAG_RXNE) == RESET) && nss == 0) {
			//verify NSS is still low
			nss =GPIO_ReadInputDataBit(SPI_PIN_NSS_PORT, SPI_PIN_NSS) ;	
		}

		if (nss == 0) {

			if (cmd == ARDUINO_CMD_AVAILABLE) {
				if (arduino_cmd_len == 1) 
					SPI_I2S_SendData(SPI_INTERFACE, (lora_data_available >> 8) & 0xFF); // Send MSB
				else if (arduino_cmd_len == 2) 
					SPI_I2S_SendData(SPI_INTERFACE, lora_data_available & 0xFF); // Send LSB
				else 
					SPI_I2S_SendData(SPI_INTERFACE, 0);
			}
			else if (cmd == ARDUINO_CMD_READ) {
				if (arduino_cmd_len == 1 ) {
					if (lora_data_available > 0 && arduino_read_buf != NULL ) {
						SPI_I2S_SendData(SPI_INTERFACE, arduino_read_buf[arduino_read_buf_len - lora_data_available]  );
						lora_data_available--;
						if (lora_data_available == 0) {
							status_led_rx_on(FALSE);
						}
						read_error_flag = FALSE;
					}
					else {
						SPI_I2S_SendData(SPI_INTERFACE, 0);
						read_error_flag = TRUE;
					}
				}
				else {
					SPI_I2S_SendData(SPI_INTERFACE, 0);
				}
			}
			else if (cmd == ARDUINO_CMD_LAST_SNR) {
				if (arduino_cmd_len == 1)       
					SPI_I2S_SendData(SPI_INTERFACE, rx_last_snr_g);
        else 
					SPI_I2S_SendData(SPI_INTERFACE, 0);
      }
			else if (cmd == ARDUINO_CMD_LAST_RSSI) {
				if (arduino_cmd_len == 1)       
					SPI_I2S_SendData(SPI_INTERFACE, rx_last_rssi_g);
				else 
					SPI_I2S_SendData(SPI_INTERFACE, 0);
      
      }
			// store received data
			arduino_cmd_buf[arduino_cmd_len] = SPI_I2S_ReceiveData(SPI_INTERFACE); arduino_cmd_len++;
		}

	}	


//	printf ("\nSPI msg received:\n\r");
//	for (i=0; i< arduino_cmd_len; i++)
//		printf("%02x", arduino_cmd_buf[i]);
//	printf ("\n\r");


	// prepare last command status if possible
	if (cmd == ARDUINO_CMD_READ){
		if (read_error_flag == TRUE) { 
			SPI_I2S_SendData(SPI_INTERFACE, ARDUINO_CMD_STATUS_NO_DATA_AVAILABLE );
		}
		else {
			SPI_I2S_SendData(SPI_INTERFACE, ARDUINO_CMD_STATUS_OK );
		}
	}
	else if (cmd == ARDUINO_CMD_AVAILABLE){
		SPI_I2S_SendData(SPI_INTERFACE, ARDUINO_CMD_STATUS_OK );
	}
	else {
		process_poll(&arduino_cmd_process);

		// Preload No status for last command, set_last_cmd_status must be called at the end of the command to set the status 	
		SPI_I2S_SendData(SPI_INTERFACE, ARDUINO_CMD_STATUS_NO_STATUS );
	}

	/* Enable interrupt */
	SPI_I2S_ITConfig(SPI_INTERFACE, SPI_I2S_IT_RXNE, ENABLE);
} 


void set_arduino_read_buf(u8 * buf, u16 len) {
	arduino_read_buf = buf;
	arduino_read_buf_len = lora_data_available = len;
}

void set_last_cmd_status(u8 status){
	SPI_I2S_SendData(SPI_INTERFACE, status );


}


