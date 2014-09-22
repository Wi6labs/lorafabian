/**
  ******************************************************************************
  * @file    EXTI/EXTI_Config/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <stdio.h>

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup EXTI_Config
  * @{
  */


#ifdef STMIT_DEB
#define PRINTD(...) printf(__VA_ARGS__)
#define PRINTW(...) printf(__VA_ARGS__)
#else
#define PRINTD(...)
#ifdef STMIT_WARN
#define PRINTW(...) printf(__VA_ARGS__)
#else
#define PRINTW(...)
#endif /*STMIT_WARN*/
#endif /*STMIT_DEB*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSV_Handler exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/* void SysTick_Handler(void) */
/* { */
/*     (void)SysTick->CTRL; */
/*     SCB->ICSR = SCB_ICSR_PENDSTCLR; */
/*     led1_toggle(); */
/*     printf("fuck!\n\r"); */
/* } */

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles External line 0 interrupt request.
  * @param  None
  * @retval None
  */

/*void EXTI0_IRQHandler(void)
{
}
*/
/**
  * @brief  This function handles External lines 9 to 5 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
}

void EXTI9_5_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line9) != RESET)
  { 
    /* Clear the Key Button EXTI line pending bit */  
    EXTI_ClearITPendingBit(EXTI_Line9);
  }
}
void EXTI15_10_IRQHandler(void)
{
    /* delay_ms(30); */
    /* led2_toggle(); */
    /* while(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_13)); */
    /* while(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15)); */
    /* EXTI_ClearITPendingBit(EXTI_Line13); */
    /* EXTI_ClearITPendingBit(EXTI_Line15);  //清除EXTI15线路挂起位 */
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */
#include "stm32f10x_tim.h"
/*void TIM3_IRQHandler(void)*/
/*{*/
/*    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)*/
/*    {*/
/*        TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );*/
/*        PRINTD("timer irq\n\r");*/
/*    }*/
/*}*/
void WWDG_IRQHandler(void)
{
    WWDG_SetCounter(0x7F);
    WWDG_ClearFlag();
    PRINTD ("wdg irq\n\r");
    /* led1_toggle(); */
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
