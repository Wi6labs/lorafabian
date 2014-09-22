#include "leds.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define LED1_PIN  GPIO_Pin_5
#define LED1_PORT GPIOA
#define LED1_RCC  RCC_APB2Periph_GPIOA


//BitAction reverse = Bit_SET, reverse2 = Bit_SET;

void leds_arch_init(void)
{
    /* Enable GPIOA clock */
    RCC_APB2PeriphClockCmd(LED1_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    /* Configure PA.5 as Output push-pull SB42 is set and SB29 released*/
    GPIO_InitStructure.GPIO_Pin = LED1_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

unsigned char leds_arch_get(void)
{
    return (unsigned char) GPIO_ReadOutputDataBit(LED1_PORT, LED1_PIN);
}

void leds_arch_set(unsigned char leds)
{
	if(leds & LEDS_GREEN) {
		GPIO_WriteBit(LED1_PORT, LED1_PIN, 0x01); 
	}  else {
		GPIO_WriteBit(LED1_PORT, LED1_PIN, 0x00);   
	}
}
