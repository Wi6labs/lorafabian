#ifndef __DEBUG_UART_H__
#define __DEBUG_UART_H__

int lprintf(int level, const char *ptr, int len);

void
dbg_setup_uart(void);

unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len);

#endif /* __DEBUG_UART_H__*/
