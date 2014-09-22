#include <stm32f10x.h>
#include <stm32f10x_dbgmcu.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <sys/process.h>
#include <sys/procinit.h>
#include <etimer.h>
#include <sys/autostart.h>
#include <clock.h>
#include <misc.h>
#include "contiki-conf.h"
#include "leds.h"
#include "cfs-coffee.h"
#include "mmem.h"
#include <usb-api.h>
#include "uart1.h"
#include "serial-line.h"

#ifdef USE_NETSTACK
#include "contiki-net.h"
#endif /*USE_NETSTACK*/

/* #include "lcd.h" */

/*Value to be updated function of tests*/
#define STOP_MODE_MINI_TIME CLOCK_SECOND
#define STOP_MODE_MAX_TIME CLOCK_SECOND*60
#define STBYE_MODE_MINI_TIME STOP_MODE_MAX_TIME
#define STBYE_MODE_MAX_TIME CLOCK_SECOND*120

PROCESS_NAME(usb_cdc_process);

unsigned int idle_count = 0;

//#define MAIN_DEB
#ifdef MAIN_DEB
#define PRINTD(...) printf(__VA_ARGS__)
#define PRINTW(...) printf(__VA_ARGS__)
#else
#define PRINTD(...)
#ifdef MAIN_WARN
#define PRINTW(...) printf(__VA_ARGS__)
#else
#define PRINTW(...)
#endif /*MAIN_WARN*/
#endif /*MAIN_DEB*/


#ifdef USE_JTAG_DEBUGGER
void dbg_init() {
  /* Keep debugger connection during SLEEP (debugging) */
  DBGMCU_Config(DBGMCU_SLEEP, ENABLE);
  /* Keep debugger connection during STOP (debugging) */
  DBGMCU_Config(DBGMCU_STOP, ENABLE);
  /* IWDG stopped when core is halted (debugging) */
  DBGMCU_Config(DBGMCU_IWDG_STOP, ENABLE);
}
#endif /*USE_JTAG_DEBUGGER*/

#define GPIOA_NB 15
GPIO_InitTypeDef GPIO_InitStructure_GPIOA[GPIOA_NB] = {

	/*GPIO_Pin, GPIO_Speed, GPIO_Mode*/
	{GPIO_Pin_0, GPIO_Speed_2MHz, GPIO_Mode_IPU},	 		/*WKPUP*/ 						/*0*/
	{GPIO_Pin_1, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING}, 	/*USART2_RTS - not connected*/
	{GPIO_Pin_2, GPIO_Speed_2MHz, GPIO_Mode_IPU}, 		/*USART2_TX*/
	{GPIO_Pin_3, GPIO_Speed_2MHz, GPIO_Mode_IPU}, 		/*USART2_RX*/
	{GPIO_Pin_4, GPIO_Speed_2MHz, GPIO_Mode_IPU}, 		/*SPI1_NSS*/
	{GPIO_Pin_5, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING}, 	/*SPI1_SCK - LED 1*/	/*5*/
	{GPIO_Pin_6, GPIO_Speed_2MHz, GPIO_Mode_IPU},			/*SPI1_MISO*/
	{GPIO_Pin_7, GPIO_Speed_2MHz, GPIO_Mode_IPU},			/*SPI1_MOSI*/
	{GPIO_Pin_8, GPIO_Speed_2MHz, GPIO_Mode_IPU},			/*USART1_OK*/
	{GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_IPU},			/*USART1_TX*/
	{GPIO_Pin_10, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*USART1_RX*/					/*10*/
	{GPIO_Pin_11, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*USART1_CTS/USBDM*/
	{GPIO_Pin_12, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*USART1_RTS/USBDP*/
	{GPIO_Pin_13, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*JTMS*/
	{GPIO_Pin_14, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*JTCK*/
	{GPIO_Pin_15, GPIO_Speed_2MHz, GPIO_Mode_IPU}			/*JTDI*/							/*15*/
};
#define GPIOB_NB 15
GPIO_InitTypeDef GPIO_InitStructure_GPIOB[GPIOB_NB]= {

	/*GPIO_Pin, GPIO_Speed, GPIO_Mode*/
	{GPIO_Pin_0, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*ADC8*/					/*0*/
	{GPIO_Pin_1, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},		/*ADC9*/
	//{GPIO_Pin_2, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},		/*BOOT1*/
	{GPIO_Pin_3, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*JTD0*/
	{GPIO_Pin_4, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*JTRST*/
	{GPIO_Pin_5, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*I2C1_SMBA*/			/*5*/
	{GPIO_Pin_6, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*I2C1_SCL*/
	{GPIO_Pin_7, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*I2C1_SDA*/
	{GPIO_Pin_8, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*TIM4_CH3*/
	{GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_IPU},						/*TIM4_CH4*/
	{GPIO_Pin_10, GPIO_Speed_2MHz, GPIO_Mode_IPU},					/*I2C2_SCL*/		/*10*/
	{GPIO_Pin_11, GPIO_Speed_2MHz, GPIO_Mode_IPU},					/*I2C2_SDA*/
	{GPIO_Pin_12, GPIO_Speed_2MHz, GPIO_Mode_IPU},					/*SPI2_NSS*/
	{GPIO_Pin_13, GPIO_Speed_2MHz, GPIO_Mode_IPU},					/*SPI2_SCK*/
	{GPIO_Pin_14, GPIO_Speed_2MHz, GPIO_Mode_IPU},					/*SPI2_MSO*/
	{GPIO_Pin_15, GPIO_Speed_2MHz, GPIO_Mode_IPU}						/*SPI2_MOSI*/		/*15*/
};
#define GPIOC_NB 14
GPIO_InitTypeDef GPIO_InitStructure_GPIOC[GPIOC_NB]= {

	/*GPIO_Pin, GPIO_Speed, GPIO_Mode*/
	{GPIO_Pin_0, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*ADC10*/							/*0*/
	{GPIO_Pin_1, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*ADC11*/
	{GPIO_Pin_2, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*ADC12*/
	{GPIO_Pin_3, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*ADC13*/
	{GPIO_Pin_4, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*ADC14*/
	{GPIO_Pin_5, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*ADC15*/							/*5*/
	{GPIO_Pin_6, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*TIM3_CH1*/
	{GPIO_Pin_7, GPIO_Speed_2MHz, GPIO_Mode_Out_OD},		/*TIM3_CH2*/
	{GPIO_Pin_8, GPIO_Speed_2MHz, GPIO_Mode_IPD},		/*TIM3_CH3 - BOOT0*/
	{GPIO_Pin_9, GPIO_Speed_2MHz, GPIO_Mode_IPU},		/*TIM3_CH4*/
	{GPIO_Pin_10, GPIO_Speed_2MHz, GPIO_Mode_IPU},	/*USART3_TX */	/*10*/
	{GPIO_Pin_11, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},	/*USART3_RX USB_P*/					
	{GPIO_Pin_12, GPIO_Speed_2MHz, GPIO_Mode_IPU},					/*USART3_CK DISC*/
	{GPIO_Pin_13, GPIO_Speed_2MHz, GPIO_Mode_IPD},					/*ANTI_TAMP --> USER blue button*/
	//{GPIO_Pin_14, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},	/*OSC32_IN*/
	//{GPIO_Pin_15, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},	/*OSC32_OUT*/					/*15*/
};
#define GPIOD_NB 1
GPIO_InitTypeDef GPIO_InitStructure_GPIOD[GPIOD_NB]= {

	/*GPIO_Pin, GPIO_Speed, GPIO_Mode*/
	//{GPIO_Pin_0, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},	/*OSC_IN*/						/*0*/
	//{GPIO_Pin_1, GPIO_Speed_2MHz, GPIO_Mode_IN_FLOATING},	/*OSC_OUT*/
	{GPIO_Pin_2, GPIO_Speed_2MHz, GPIO_Mode_IPD},						/*TIM3_ETR*/
};
/*
Set all IOs to a correct value
*/
void gpio_init_all()
{
	int i;

	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOC);
	GPIO_DeInit(GPIOD);
	GPIO_DeInit(GPIOE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	for(i = 0; i < GPIOA_NB; i++) {
		GPIO_Init(GPIOA, &GPIO_InitStructure_GPIOA[i]);
	}

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	for(i = 0; i < GPIOB_NB; i++) {
		GPIO_Init(GPIOB, &GPIO_InitStructure_GPIOB[i]);
	}

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	for(i = 0; i < GPIOC_NB; i++) {
		GPIO_Init(GPIOB, &GPIO_InitStructure_GPIOC[i]);
	}


  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	for(i = 0; i < GPIOD_NB; i++) {
		GPIO_Init(GPIOB, &GPIO_InitStructure_GPIOD[i]);
	}
}


int
main()
{
//  gpio_init_all();

#ifdef USE_UART1_TRACE
  uart1_init(115200);
  PRINTD("Starting contiki \n\r");
#endif /*USE_UART1_TRACE*/

#ifdef USE_JTAG_DEBUGGER
  dbg_init();
#endif /*USE_JTAG_DEBUGGER*/


  clock_init();
  process_init();

#ifdef USE_USBCDC_TRACE
  process_start(&usb_cdc_process, NULL);
  PRINTD("Starting contiki \n\r");
#endif /*USE_USBCDC_TRACE*/

  process_start(&etimer_process, NULL);

#ifdef USE_SHELL
  /*for shell application*/
  serial_line_init();
  uart1_set_input(uart1_read);
  cfs_server_init();
#endif /*USE_SHELL*/


#ifdef USE_NETSTACK
  queuebuf_init();
  netstack_init();
  process_start(&tcpip_process, NULL);
#endif /*USE_NETSTACK*/

  autostart_start(autostart_processes);

  PRINTD("Processes running\n\r");

  while(1) {
	  do {  
    } while(process_run() > 0);
    idle_count++;
	}
	
  return 0;
}


#if UIP_CONF_LOGGING
void uip_log(char *msg)
{
    PRINTD("%u : %s\n\r", clock_time(), msg);
}
#endif

void assert_failed(u8 * file, u32 line)
{

	printf("assert_failed at file %s, line %d \n\r", file, line);


}
