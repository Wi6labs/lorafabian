#include <stdio.h>
#include <debug-uart.h>
#include "uart1.h"
#include <string.h>
#include <strformat.h>
#include "debug_lprintf.h" 

#ifdef USE_UART1_TRACE
#define OUTPUT_ID 1
#else
	#ifdef USE_UART2_TRACE
		#define OUTPUT_ID 2
	#else
  #ifdef USE_USBCDC_TRACE
  #define OUTPUT_ID 4
  #else
  #define OUTPUT_ID 0 
  #endif /*USE_USBCDC_TRACE*/  
  #endif /*USE_UART2_TRACE*/
#endif /*USE_UART1_TRACE*/


#ifndef STDOUT_USART
#define STDOUT_USART OUTPUT_ID
#endif

#ifndef STDERR_USART
#define STDERR_USART OUTPUT_ID
#endif

#ifndef STDIN_USART
#define STDIN_USART OUTPUT_ID
#endif


/* light printf routine*/
int lprintf(int level, const char *ptr, int len)
{

    switch (level) {
    case OUT_STD: /*stdout*/
#if STDOUT_USART == 1
        uart1_write(ptr, len);
#elif STDOUT_USART == 2
        uart2_write(ptr, len);
#elif  STDOUT_USART == 4
        usb_write_trace(ptr, len);
#endif

//				SendMsgViaITM(ptr, len);

        break;
    case OUT_ERR: /* stderr */
#ifdef USE_CFS_SERVER
				cfs_server_printf(ptr, len);
#endif /*USE_CFS_SERVER*/

#if STDERR_USART == 1
        uart1_write(ptr, len);
#elif STDERR_USART == 2
        uart2_write(ptr, len);
#elif  STDERR_USART == 4
        usb_write_trace(ptr, len);
#endif

//				SendMsgViaITM(ptr, len);

        break;
    default:
        return -1;
    }
    return len;


}

unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len) {

#if STDOUT_USART == 1
        uart1_write(seq, len);
#elif STDERR_USART == 2
        uart2_write(seq, len);
#elif  STDOUT_USART == 4
        usb_write_trace(seq, len);
#endif
  return len;

}


void
dbg_putchar(const char ch){

  lprintf(OUT_STD, &ch, 1);

} 

