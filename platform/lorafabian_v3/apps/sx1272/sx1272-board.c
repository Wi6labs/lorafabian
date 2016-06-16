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
#include "stm32l1xx.h"
#include "sx1272_radio.h"
#include <misc.h>
#include "clock.h"
#include "sx1272.h"
#include "sx1272-board.h"
#include "system_stm32l1xx.h" 
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "stm32l1xx_exti.h"
#include "stm32l1xx_syscfg.h"
#include "misc.h"

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
 * Hardware IO IRQ callback function definition
 */
typedef void ( EXTIIrqHandler )( void );

static EXTIIrqHandler *ExtiIRQ[15]; 


// -----------------------------------------------------------------------------
// I/O


// output lines
// output lines
#define NSS_PORT           GPIOA // NSS: PA4
#define NSS_PIN            GPIO_Pin_4

#define L_FEM_CTX_PORT            GPIOB// CTX:  PB6  
#define L_FEM_CTX_PIN             GPIO_Pin_6
#define L_FEM_CPS_PORT            GPIOB // CPS:  PB0  Not used
#define L_FEM_CPS_PIN             GPIO_Pin_0
#define RST_PORT           GPIOC // RST: PC7
#define RST_PIN            GPIO_Pin_7

#define ONOFF_LORA_PORT    GPIOC // PC4
#define ONOFF_LORA_PIN     GPIO_Pin_4

// input lines
#define DIO0_PORT           GPIOB // DIO0: PB10   
#define DIO0_PIN            GPIO_Pin_10
#define DIO0_EXTI_Line      EXTI_Line10
#define DIO0_EXTI_PinSource EXTI_PinSource10
#define DIO0_EXTI_Port      EXTI_PortSourceGPIOB
#define DIO0_IRQn           EXTI15_10_IRQn

#define DIO1_PORT           GPIOB // DIO1: PB4  
#define DIO1_PIN            GPIO_Pin_4
#define DIO1_EXTI_Line      EXTI_Line4
#define DIO1_EXTI_PinSource EXTI_PinSource4
#define DIO1_EXTI_Port      EXTI_PortSourceGPIOB
#define DIO1_IRQn           EXTI4_IRQn

#define DIO2_PORT           GPIOB // DIO2: PB5 
#define DIO2_PIN            GPIO_Pin_5
#define DIO2_EXTI_Line      EXTI_Line5
#define DIO2_EXTI_PinSource EXTI_PinSource5
#define DIO2_EXTI_Port      EXTI_PortSourceGPIOB
#define DIO2_IRQn           EXTI9_5_IRQn

#define DIO3_PORT           GPIOA // DIO3: PA2 
#define DIO3_PIN            GPIO_Pin_2
#define DIO3_EXTI_Line      EXTI_Line2
#define DIO3_EXTI_PinSource EXTI_PinSource2
#define DIO3_EXTI_Port      EXTI_PortSourceGPIOA
#define DIO3_IRQn           EXTI2_IRQn

#define DIO4_PORT           GPIOA // DIO4: PA3 
#define DIO4_PIN            GPIO_Pin_3
#define DIO4_EXTI_Line      EXTI_Line3
#define DIO4_EXTI_PinSource EXTI_PinSource3
#define DIO4_EXTI_Port      EXTI_PortSourceGPIOA
#define DIO4_IRQn           EXTI3_IRQn

#define DIO5_PORT           GPIOA // DIO5: PA1 
#define DIO5_PIN            GPIO_Pin_1
#define DIO5_EXTI_Line      EXTI_Line1
#define DIO5_EXTI_PinSource EXTI_PinSource1
#define DIO5_EXTI_Port      EXTI_PortSourceGPIOA
#define DIO5_IRQn           EXTI1_IRQn

void SX1272IoInit( void )
{

  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  EXTI_InitTypeDef   EXTI_InitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | RCC_AHBPeriph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  // OUT config
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

  GPIO_InitStructure.GPIO_Pin = NSS_PIN;
  GPIO_Init(NSS_PORT , &GPIO_InitStructure );
  GPIO_WriteBit(NSS_PORT, NSS_PIN, 0);

  GPIO_InitStructure.GPIO_Pin = ONOFF_LORA_PIN;
  GPIO_Init(ONOFF_LORA_PORT , &GPIO_InitStructure );
  GPIO_WriteBit(ONOFF_LORA_PORT, ONOFF_LORA_PIN, 0);

  // FEM 
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

  GPIO_InitStructure.GPIO_Pin = L_FEM_CTX_PIN;
  GPIO_Init(L_FEM_CTX_PORT , &GPIO_InitStructure );
  GPIO_WriteBit(L_FEM_CTX_PORT, L_FEM_CTX_PIN, 0);

  // Not used keep default config for the moment
  //    GPIO_InitStructure.GPIO_Pin = L_FEM_CPS_PIN;
  //    GPIO_Init(L_FEM_CPS_PORT , &GPIO_InitStructure );

  // IN config
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  GPIO_InitStructure.GPIO_Pin = DIO0_PIN;
  GPIO_Init(DIO0_PORT , &GPIO_InitStructure );
  SYSCFG_EXTILineConfig(DIO0_EXTI_Port, DIO0_EXTI_PinSource);

  EXTI_ClearFlag(DIO0_EXTI_Line);
  EXTI_InitStructure.EXTI_Line = DIO0_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO0_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DIO1_PIN;
  GPIO_Init(DIO1_PORT , &GPIO_InitStructure );
  SYSCFG_EXTILineConfig(DIO1_EXTI_Port, DIO1_EXTI_PinSource);

  EXTI_ClearFlag(DIO1_EXTI_Line);
  EXTI_InitStructure.EXTI_Line = DIO1_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO1_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DIO2_PIN;
  GPIO_Init(DIO2_PORT , &GPIO_InitStructure );
  SYSCFG_EXTILineConfig(DIO2_EXTI_Port, DIO2_EXTI_PinSource);
  EXTI_ClearFlag(DIO2_EXTI_Line);
  EXTI_InitStructure.EXTI_Line = DIO2_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO2_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DIO3_PIN;
  GPIO_Init(DIO3_PORT , &GPIO_InitStructure );
  SYSCFG_EXTILineConfig(DIO3_EXTI_Port, DIO3_EXTI_PinSource);
  EXTI_ClearFlag(DIO3_EXTI_Line);
  EXTI_InitStructure.EXTI_Line = DIO3_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO3_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DIO4_PIN;
  GPIO_Init(DIO4_PORT , &GPIO_InitStructure );
  SYSCFG_EXTILineConfig(DIO4_EXTI_Port, DIO4_EXTI_PinSource);
  EXTI_ClearFlag(DIO4_EXTI_Line);
  EXTI_InitStructure.EXTI_Line = DIO4_EXTI_Line;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_InitStructure.NVIC_IRQChannel = DIO4_IRQn;
  NVIC_Init(&NVIC_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DIO5_PIN;
  GPIO_Init(DIO5_PORT , &GPIO_InitStructure );

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
}



void SX1272IoDeInit( void )
{

}

// Done in SX1272IoInit
void SX1272IoIrqEnable( void )
{
}

// Done in SX1272IoInit
void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{
  ExtiIRQ[DIO0_EXTI_PinSource] = irqHandlers[0];
  ExtiIRQ[DIO1_EXTI_PinSource] = irqHandlers[1];
  ExtiIRQ[DIO2_EXTI_PinSource] = irqHandlers[2];
  ExtiIRQ[DIO3_EXTI_PinSource] = irqHandlers[3];
  ExtiIRQ[DIO4_EXTI_PinSource] = irqHandlers[4];
  ExtiIRQ[DIO5_EXTI_PinSource] = irqHandlers[5];

}


void SX1272SetReset( uint8_t state )
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // OUT config
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;

  GPIO_InitStructure.GPIO_Pin = RST_PIN;
  GPIO_Init(RST_PORT , &GPIO_InitStructure );

  if( state == RADIO_RESET_ON )
  {
    GPIO_WriteBit(RST_PORT, RST_PIN, 1); 
  }
  else
  {
    GPIO_WriteBit(RST_PORT, RST_PIN, 0); 
  }
}

uint8_t SX1272GetPaSelect( uint32_t channel )
{
  return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1272SetAntSwLowPower( bool status )
{
}

void SX1272AntSwInit( void )
{
}

void SX1272AntSwDeInit( void )
{

}

void SX1272SetAntSw( uint8_t rxTx )
{
  SX1272.RxTx = rxTx;

 // 1: TX, 0: RX
 GPIO_WriteBit(L_FEM_CTX_PORT, L_FEM_CTX_PIN, rxTx);
}

bool SX1272CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supportted
    return TRUE;
}

/*set to 1 (ON) to get switch ON LoRa module*/
inline void SX1272SetPower( uint8_t onoff )
{
  GPIO_WriteBit(ONOFF_LORA_PORT, ONOFF_LORA_PIN, onoff);
}

/*set NSS for SPI*/
inline void SX1272SetNSS( uint8_t value )
{
  GPIO_WriteBit(NSS_PORT, NSS_PIN, value);
}

/*perform a delay in ms*/
void DelayMs( uint32_t time_in_ms )
{
	  uint32_t  counter_ms = 8000*time_in_ms;
	  while(counter_ms-- > 0) { asm(""); }

}


