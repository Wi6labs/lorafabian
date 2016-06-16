#include <stm32l1xx.h>
#include <stm32l1xx_dbgmcu.h>
#include <stm32l1xx_gpio.h>
#include <stm32l1xx_rcc.h>
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


/*
Set all IOs to a correct value
*/
void gpio_init_all()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure all GPIO as analog to reduce current consumption on non used IOs */
  /* Enable GPIOs clock */
#ifdef STM32L1XX_HD
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD |
                        RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
                        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, ENABLE);
#else //STM32L1XX_HD
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD |
                        RCC_AHBPeriph_GPIOE, ENABLE);
#endif //STM32L1XX_HD

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);
#ifdef STM32L1XX_HD
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  GPIO_Init(GPIOF, &GPIO_InitStructure);
  GPIO_Init(GPIOG, &GPIO_InitStructure);
#endif //STM32L1XX_HD
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  GPIO_Init(GPIOB, &GPIO_InitStructure);   

#ifdef STM32L1XX_HD
  /* Disable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD |
                        RCC_AHBPeriph_GPIOE | RCC_AHBPeriph_GPIOH |
                        RCC_AHBPeriph_GPIOF | RCC_AHBPeriph_GPIOG, DISABLE);
#else //STM32L1XX_HD
  /* Disable GPIOs clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB |
                        RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD |
                        RCC_AHBPeriph_GPIOE, DISABLE);
#endif //STM32L1XX_HD
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

			SCB->SCR &=~SCB_SCR_SLEEPDEEP_Msk;
			__WFI();

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
