#include <stm32l1xx.h>
#include <system_stm32l1xx.h>
//#include <misc.h>
/* #include <nvic.h> */
#include <sys/clock.h>
#include <sys/cc.h>
#include <sys/etimer.h>

#include <core_cm3.h>
#include <stdio.h>


#include <dev/leds.h>

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;

void
SysTick_Handler(void) __attribute__ ((interrupt));

void
SysTick_Handler(void)
{
    (void)SysTick->CTRL;
    SCB->ICSR = SCB_ICSR_PENDSTCLR;

    current_clock++;


    if(etimer_pending() && etimer_next_expiration_time() <= current_clock) {
        etimer_request_poll();
     //   printf("%d,%d\n", clock_time(),etimer_next_expiration_time  	());
    }

    if (--second_countdown == 0) {
        current_seconds++;
        second_countdown = CLOCK_SECOND;
    }
}

void
clock_init()
{
    if (SysTick_Config(SystemCoreClock / CLOCK_SECOND))
    {
        while(1);
    }

	  NVIC_SetPriority(SysTick_IRQn, 0x0C);
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

////////////////////////////////////////////////////////////////////////////////
/// This function performs a delay in tick
///
/// @return - none
///
////////////////////////////////////////////////////////////////////////////
void clock_delay(unsigned int t)
{
  clock_time_t end_tick = get_current_clock() + t;

  while(get_current_clock() < end_tick);
}

unsigned long
clock_seconds(void)
{
  return get_current_seconds();
}
