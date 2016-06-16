/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX1272 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "stm32f10x.h"
#include "sx1272_radio.h"
#include <misc.h>
#include "clock.h"
#include "sx1272.h"
#include "sx1272-board.h"
#include "system_stm32f10x.h" 
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = FALSE;

/*!
 * Radio driver structure initialization,

 */
const struct Radio_s Radio =
{
    SX1272Init,
    SX1272GetStatus,
    SX1272SetModem,
    SX1272SetChannel,
    SX1272IsChannelFree,
    SX1272Random,
    SX1272SetRxConfig,
    SX1272SetTxConfig,
    SX1272CheckRfFrequency,
    SX1272GetTimeOnAir,
    SX1272Send,
    SX1272SetSleep,
    SX1272SetStby, 
    SX1272SetRx,
    SX1272ReadRssi,
    SX1272Write,
    SX1272Read,
    SX1272WriteBuffer,
    SX1272ReadBuffer,
    SX1272TurnOff
};

/*!
 * SX1272 RESET I/O definitions
 */
#define RESET_IOPORT                                GPIOC
#define RESET_PIN                                   GPIO_Pin_7

/*!
 * SX1272 SPI NSS I/O definitions
 */
#define NSS_IOPORT                                  GPIOA
#define NSS_PIN                                     GPIO_Pin_4

/*!
 * SX1272 DIO pins  I/O definitions
 */
#define DIO0_IOPORT                                 GPIOB
#define DIO0_PIN                                    GPIO_Pin_10
#define DIO0_PIN_ID                                 10
#define DIO0_EXTI_Line                              EXTI_Line10
#define DIO0_EXTI_PinSource                         GPIO_PinSource10
#define DIO0_EXTI_Port                              GPIO_PortSourceGPIOB
#define DIO0_IRQn                                   EXTI15_10_IRQn

#define DIO1_IOPORT                                 GPIOB
#define DIO1_PIN                                    GPIO_Pin_4
#define DIO1_PIN_ID                                 4
#define DIO1_EXTI_Line                              EXTI_Line4
#define DIO1_EXTI_PinSource                         GPIO_PinSource4
#define DIO1_EXTI_Port                              GPIO_PortSourceGPIOB
#define DIO1_IRQn                                   EXTI4_IRQn

#define DIO2_IOPORT                                 GPIOB
#define DIO2_PIN                                    GPIO_Pin_5
#define DIO2_PIN_ID                                 5
#define DIO2_EXTI_Line                              EXTI_Line5
#define DIO2_EXTI_PinSource                         GPIO_PinSource5
#define DIO2_EXTI_Port                              GPIO_PortSourceGPIOB
#define DIO2_IRQn                                   EXTI9_5_IRQn


#define DIO3_IOPORT                                 GPIOA
#define DIO3_PIN                                    GPIO_Pin_2
#define DIO3_PIN_ID                                 2
#define DIO3_EXTI_Line                              EXTI_Line2
#define DIO3_EXTI_PinSource                         GPIO_PinSource2
#define DIO3_EXTI_Port                              GPIO_PortSourceGPIOA
#define DIO3_IRQn                                   EXTI2_IRQn


#define DIO4_IOPORT                                 GPIOA
#define DIO4_PIN                                    GPIO_Pin_3
#define DIO4_PIN_ID                                 3
#define DIO4_EXTI_Line                              EXTI_Line3
#define DIO4_EXTI_PinSource                         GPIO_PinSource3
#define DIO4_EXTI_Port                              GPIO_PortSourceGPIOA
#define DIO4_IRQn                                   EXTI3_IRQn


#define DIO5_IOPORT                                 GPIOA
#define DIO5_PIN                                    GPIO_Pin_1
#define DIO5_PIN_ID                                 1
#define DIO5_EXTI_Line                              EXTI_Line1
#define DIO5_EXTI_PinSource                         GPIO_PinSource1
#define DIO5_EXTI_Port                              GPIO_PortSourceGPIOA
#define DIO5_IRQn                                   EXTI1_IRQn

#define RADIO_ANT_SWITCH_HF_PORT                    GPIOB  //CTX
#define RADIO_ANT_SWITCH_HF_PIN                     GPIO_Pin_6

#define RADIO_ANT_SWITCH_LF_PORT                    GPIOA  //CPS
#define RADIO_ANT_SWITCH_LF_PIN                     GPIO_Pin_8

#define RXTX_IOPORT                                 GPIOB
#define RXTX_PIN                                    GPIO_Pin_0

#define ONOFF_LORA_IOPORT                           GPIOC
#define ONOFF_LORA_PIN                              GPIO_Pin_4

/*!
 * Hardware IO IRQ callback function definition
 */
typedef void ( EXTIIrqHandler )( void );

static EXTIIrqHandler *ExtiIRQ[15]; 

void SX1272IoInit( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
			RCC_APB2Periph_GPIOC , ENABLE ); 

	/*ONOFF settings*/
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = ONOFF_LORA_PIN;
	GPIO_Init( ONOFF_LORA_IOPORT, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	// Configure NSS as output
	GPIO_WriteBit( NSS_IOPORT, NSS_PIN, Bit_SET );
	GPIO_InitStructure.GPIO_Pin = NSS_PIN;
	GPIO_Init( NSS_IOPORT, &GPIO_InitStructure );

	// Configure RESET as output
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = RESET_PIN;
	GPIO_Init( RESET_IOPORT, &GPIO_InitStructure );


	// Configure radio DIO as inputs
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	// Configure DIO0
	GPIO_InitStructure.GPIO_Pin =  DIO0_PIN;
	GPIO_Init( DIO0_IOPORT, &GPIO_InitStructure );

	// Configure DIO1
	GPIO_InitStructure.GPIO_Pin =  DIO1_PIN;
	GPIO_Init( DIO1_IOPORT, &GPIO_InitStructure );

	// Configure DIO2
	GPIO_InitStructure.GPIO_Pin =  DIO2_PIN;
	GPIO_Init( DIO2_IOPORT, &GPIO_InitStructure );

	// Configure DIO3 as input
	GPIO_InitStructure.GPIO_Pin =  DIO3_PIN;
	GPIO_Init( DIO2_IOPORT, &GPIO_InitStructure );    

	// Configure DIO4 as input
	GPIO_InitStructure.GPIO_Pin =  DIO4_PIN;
	GPIO_Init( DIO2_IOPORT, &GPIO_InitStructure );    

	// Configure DIO5 as input
	GPIO_InitStructure.GPIO_Pin =  DIO5_PIN;
	GPIO_Init( DIO2_IOPORT, &GPIO_InitStructure );

	SX1272SetPower(1);
}


/*INTERRUPT routine*/
void
EXTI1_IRQHandler(void) __attribute__ ((interrupt));
/**
  * @brief  This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
    /* Clear the EXTI line 1 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line1);
    ExtiIRQ[1]();
  }
}

void
EXTI2_IRQHandler(void) __attribute__ ((interrupt));
/**
  * @brief  This function handles External line 2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line2) != RESET)
  {
    /* Clear the EXTI line 2 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line2);
    ExtiIRQ[2]();
  }
}

void
EXTI3_IRQHandler(void) __attribute__ ((interrupt));
/**
  * @brief  This function handles External line 3 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI3_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line3) != RESET)
  {
    /* Clear the EXTI line 3 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line3);
    ExtiIRQ[3]();
  }
}

void
EXTI4_IRQHandler(void) __attribute__ ((interrupt));
/**
  * @brief  This function handles External line 4 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI4_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line4) != RESET)
  {
    /* Clear the EXTI line 4 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line4);
    ExtiIRQ[4]();
  }
}


void
EXTI9_5_IRQHandler(void) __attribute__ ((interrupt));
/**
  * @brief  This function handles External line 5 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line5) != RESET)
  {
    /* Clear the EXTI line 5 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line5);

    ExtiIRQ[5]();
  }
}


void
EXTI15_10_IRQHandler(void) __attribute__ ((interrupt));
/**
  * @brief  This function handles External line 10-15 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line10) != RESET)
  {
    /* Clear the EXTI line 2 pending bit */
    EXTI_ClearITPendingBit(EXTI_Line10);
    ExtiIRQ[10]();
  }
}

void SX1272IoIrqDisable( void )
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;

  EXTI_InitStructure.EXTI_Line = DIO0_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO0_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  EXTI_InitStructure.EXTI_Line = DIO1_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  EXTI_InitStructure.EXTI_Line = DIO2_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  EXTI_InitStructure.EXTI_Line = DIO3_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  EXTI_InitStructure.EXTI_Line = DIO4_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO4_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  EXTI_InitStructure.EXTI_Line = DIO5_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO5_IRQn;
  NVIC_Init(&NVIC_InitStructure);

}
void SX1272IoIrqEnable( void )
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;

	// Clear All interrupte pending bits
    EXTI_ClearITPendingBit(EXTI_Line1);
    EXTI_ClearITPendingBit(EXTI_Line2);
    EXTI_ClearITPendingBit(EXTI_Line3);
    EXTI_ClearITPendingBit(EXTI_Line4);
    EXTI_ClearITPendingBit(EXTI_Line5);
    EXTI_ClearITPendingBit(EXTI_Line10);


  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;


  EXTI_InitStructure.EXTI_Line = DIO0_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
	GPIO_EXTILineConfig(DIO0_EXTI_Port, DIO0_EXTI_PinSource);
  NVIC_InitStructure.NVIC_IRQChannel = DIO0_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  EXTI_InitStructure.EXTI_Line = DIO1_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
	GPIO_EXTILineConfig(DIO1_EXTI_Port, DIO1_EXTI_PinSource);
  NVIC_InitStructure.NVIC_IRQChannel = DIO1_IRQn;
  NVIC_Init(&NVIC_InitStructure);
/*  
	EXTI_InitStructure.EXTI_Line = DIO2_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);	
	GPIO_EXTILineConfig(DIO2_EXTI_Port, DIO2_EXTI_PinSource);
  NVIC_InitStructure.NVIC_IRQChannel = DIO2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
	EXTI_InitStructure.EXTI_Line = DIO3_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
	GPIO_EXTILineConfig(DIO3_EXTI_Port, DIO3_EXTI_PinSource);
  NVIC_InitStructure.NVIC_IRQChannel = DIO3_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
	EXTI_InitStructure.EXTI_Line = DIO4_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
	GPIO_EXTILineConfig(DIO4_EXTI_Port, DIO4_EXTI_PinSource);
  NVIC_InitStructure.NVIC_IRQChannel = DIO4_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
	EXTI_InitStructure.EXTI_Line = DIO5_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
	GPIO_EXTILineConfig(DIO5_EXTI_Port, DIO5_EXTI_PinSource);
  NVIC_InitStructure.NVIC_IRQChannel = DIO5_IRQn;
  NVIC_Init(&NVIC_InitStructure);
*/
}

void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{

//  EXTI_InitTypeDef   EXTI_InitStructure;
//  NVIC_InitTypeDef   NVIC_InitStructure;

  /* Enable SYSCFG clock */
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
													RCC_APB2Periph_GPIOC , ENABLE ); 

  ExtiIRQ[DIO0_PIN_ID] = irqHandlers[0];
  ExtiIRQ[DIO1_PIN_ID] = irqHandlers[1];
  ExtiIRQ[DIO2_PIN_ID] = irqHandlers[2];
  ExtiIRQ[DIO3_PIN_ID] = irqHandlers[3];
  ExtiIRQ[DIO4_PIN_ID] = irqHandlers[4];
  ExtiIRQ[DIO5_PIN_ID] = irqHandlers[5];

	// Clear All interrupte pending bits
    EXTI_ClearITPendingBit(EXTI_Line1);
    EXTI_ClearITPendingBit(EXTI_Line2);
    EXTI_ClearITPendingBit(EXTI_Line3);
    EXTI_ClearITPendingBit(EXTI_Line4);
    EXTI_ClearITPendingBit(EXTI_Line5);
    EXTI_ClearITPendingBit(EXTI_Line10);

  /*DIO0*/
  /* Connect EXTI Line to GPIO pin */
//  SYSCFG_EXTILineConfig(DIO0_EXTI_Port, DIO0_EXTI_PinSource);

#if 0
  /* Configure EXTI line */
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_Line = DIO0_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set EXTI Interrupt to the high priority */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannel = DIO0_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  /*DIO1*/
//  SYSCFG_EXTILineConfig(DIO1_EXTI_Port, DIO1_EXTI_PinSource);
  EXTI_InitStructure.EXTI_Line = DIO1_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO1_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  /*DIO2*/
//  SYSCFG_EXTILineConfig(DIO2_EXTI_Port, DIO2_EXTI_PinSource);
  EXTI_InitStructure.EXTI_Line = DIO2_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO2_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  /*DIO3*/
//  SYSCFG_EXTILineConfig(DIO3_EXTI_Port, DIO3_EXTI_PinSource);
  EXTI_InitStructure.EXTI_Line = DIO3_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO3_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  /*DIO4*/
//  SYSCFG_EXTILineConfig(DIO4_EXTI_Port, DIO4_EXTI_PinSource);
  EXTI_InitStructure.EXTI_Line = DIO4_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO4_IRQn;
  NVIC_Init(&NVIC_InitStructure);


  /*DIO5*/
//  SYSCFG_EXTILineConfig(DIO5_EXTI_Port, DIO5_EXTI_PinSource);
  EXTI_InitStructure.EXTI_Line = DIO5_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO5_IRQn;
  NVIC_Init(&NVIC_InitStructure);
#endif
}

void SX1272IoDeInit( void )
{

}

void SX1272SetReset( uint8_t state )
{

    if( state == RADIO_RESET_ON )
    {
			// Set RESET pin to 0

			GPIO_ResetBits(RESET_IOPORT, RESET_PIN);
    }
    else
    {
			GPIO_SetBits(RESET_IOPORT, RESET_PIN);

    }
}

uint8_t SX1272GetPaSelect( uint32_t channel )
{
  return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1272SetAntSwLowPower( bool status )
{
    if( RadioIsActive != status )
    {
        RadioIsActive = status;
    
        if( status == FALSE )
        {
            SX1272AntSwInit( );
        }
        else
        {
            SX1272AntSwDeInit( );
        }
    }
}

void SX1272AntSwInit( void )
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  // Configure RADIO_ANT_SWITCH_HF
  GPIO_InitStructure.GPIO_Pin =  RADIO_ANT_SWITCH_HF_PIN;
  GPIO_Init( RADIO_ANT_SWITCH_HF_PORT, &GPIO_InitStructure );    

  // Configure RADIO_ANT_SWITCH_LF
  GPIO_InitStructure.GPIO_Pin =  RADIO_ANT_SWITCH_LF_PIN;
  GPIO_Init( RADIO_ANT_SWITCH_LF_PORT, &GPIO_InitStructure );  

  GPIO_WriteBit( RADIO_ANT_SWITCH_LF_PORT, RADIO_ANT_SWITCH_LF_PIN,  Bit_SET);
  GPIO_WriteBit( RADIO_ANT_SWITCH_HF_PORT, RADIO_ANT_SWITCH_HF_PIN,  Bit_RESET);
}

void SX1272AntSwDeInit( void )
{

}

void SX1272SetAntSw( uint8_t rxTx )
{
  if( SX1272.RxTx == rxTx )
  {
      return;
  }

  SX1272.RxTx = rxTx;

  if( rxTx != 0 ) // 1: TX, 0: RX
  {
    GPIO_ResetBits( RADIO_ANT_SWITCH_LF_PORT, RADIO_ANT_SWITCH_LF_PIN );
    GPIO_SetBits( RADIO_ANT_SWITCH_HF_PORT, RADIO_ANT_SWITCH_HF_PIN );
  }
  else
  {
    GPIO_SetBits( RADIO_ANT_SWITCH_LF_PORT, RADIO_ANT_SWITCH_LF_PIN);
    GPIO_ResetBits( RADIO_ANT_SWITCH_HF_PORT, RADIO_ANT_SWITCH_HF_PIN);
  }
}

bool SX1272CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supportted
    return TRUE;
}

/*set to 1 (ON) to get switch ON LoRa module*/
inline void SX1272SetPower( uint8_t onoff )
{
  if(onoff == 1) {
    GPIO_WriteBit( ONOFF_LORA_IOPORT, ONOFF_LORA_PIN,  Bit_SET);
  } else {
    GPIO_WriteBit( ONOFF_LORA_IOPORT, ONOFF_LORA_PIN,  Bit_RESET);
  }
}

/*set NSS for SPI*/
inline void SX1272SetNSS( uint8_t value )
{
  if(value == 1) {
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN,  Bit_SET);
  } else {
    GPIO_WriteBit( NSS_IOPORT, NSS_PIN,  Bit_RESET);
  }
}

/*perform a delay in ms*/
void DelayMs( uint32_t time_in_ms )
{
	  int  counter_ms = 1600*time_in_ms;
	  while(counter_ms-- > 0) { asm(""); }

}


