#include <stdio.h>
#include "stm32f10x_usart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "uart2.h"
#include "xbee.h"


void uart2_init(unsigned int bpr)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  /* Configure USARTx_Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USARTx_Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  
  USART_InitStructure.USART_BaudRate = bpr;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART2, &USART_InitStructure);
  
    /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* Enable USARTy Receive and Transmit interrupts */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  
   /* Enable the USARTy */
  USART_Cmd(USART2, ENABLE);
  
}

void uart2_putc(unsigned char x)
{
  USART_SendData(USART2, x);
    /* Loop until the end of transmission */
  while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void uart2_print(const char *str){
    int i = 0;
    while(str[i]!='\0' && i<32){
        uart2_putc(str[i]);
        i++;
    }

}

static int (*input_call)(unsigned char )  = NULL;
void uart2_set_input(int(*fun)(unsigned char))
{
  input_call = fun;
}

void USART2_IRQHandler(void)
{
  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    unsigned char x;
    x = USART_ReceiveData(USART2);
    if(input_call != NULL) input_call(x);
  }
}

int uart2_read(unsigned char c){
    xbee_receive(c);

}
