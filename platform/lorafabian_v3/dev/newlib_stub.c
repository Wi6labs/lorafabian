/*
 * newlib_stubs.c
 *
 *  Created on: 2 Nov 2010
 *      Author: nanoage.co.uk
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32l1xx_usart.h"
#include <stdio.h>
#include "contiki.h"
#include <sys/types.h>
#include <usb-api.h>


#ifndef STDOUT_USART
#define STDOUT_USART OUTPUT_ID
#endif

#ifndef STDERR_USART
#define STDERR_USART OUTPUT_ID
#endif

#ifndef STDIN_USART
#define STDIN_USART OUTPUT_ID
#endif


#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EAGAIN
#define EAGAIN 11
#endif

#ifndef EBADF
#define EBADF 9
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

#ifndef EMLINK
#define EMLINK 31
#endif

#ifndef ENOENT
#define ENOENT 2
#endif

#ifndef ECHILD
#define ECHILD 10
#endif

#undef errno
extern int errno;

#define GET_TICK_COUNT( )                           ( get_current_clock() )
#define TICK_RATE_MS( ms )                          ( ms*1000/CLOCK_SECOND)

/*
 environ
 A pointer to a list of environment variables and their values.
 For a minimal environment, this empty list is adequate:
 */
char *__env[1] = { 0 };
char **environ = __env;

int _write(int file, char *ptr, int len);

void _exit (int status)
{
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}

int _close(int file) {
    return -1;
}
/*
 execve
 Transfer control to a new process. Minimal implementation (for a system without processes):
 */
int _execve(char *name, char **argv, char **env) {
    errno = ENOMEM;
    return -1;
}
/*
 fork
 Create a new process. Minimal implementation (for a system without processes):
 */

int _fork() {
    errno = EAGAIN;
    return -1;
}
/*
 fstat
 Status of an open file. For consistency with other minimal implementations in these examples,
 all files are regarded as character special devices.
 The `sys/stat.h' header file required is distributed in the `include' subdirectory for this C library.
 */
int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

/*
 getpid
 Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes. Minimal implementation, for a system without processes:
 */

int _getpid() {
    return 1;
}

/*
 isatty
 Query whether output stream is a terminal. For consistency with the other minimal implementations,
 */
int _isatty(int file) {
    switch (file){
    case STDOUT_FILENO:
    case STDERR_FILENO:
    case STDIN_FILENO:
        return 1;
    default:
        //errno = ENOTTY;
        errno = EBADF;
        return 0;
    }
}


/*
 kill
 Send a signal. Minimal implementation:
 */
int _kill(int pid, int sig) {
    errno = EINVAL;
    return (-1);
}

/*
 link
 Establish a new name for an existing file. Minimal implementation:
 */

int _link(char *old, char *new) {
    errno = EMLINK;
    return -1;
}

/*
 lseek
 Set position in a file. Minimal implementation:
 */
int _lseek(int file, int ptr, int dir) {
    return 0;
}


register char * stack_ptr asm("sp");

/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 */
caddr_t _sbrk(int incr)
{
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0)
		heap_end = &end;

	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr)
	{
//		write(1, "Heap and stack collision\n", 25);
//		abort();
		errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}

/*
 read
 Read a character to a file. `libc' subroutines will use this system routine for input from all files, including stdin
 Returns -1 on error or blocks until the number of characters have been read.
 */


int _read(int file, char *ptr, int len) {

    int n;
    int num = 0;
    char c;
    switch (file) {
    case STDIN_FILENO:
        for (n = 0; n < len; n++) {
#if   STDIN_USART == 1
            while ((USART1->SR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
            c = (char)(USART1->DR & (uint16_t)0x01FF);
#elif STDIN_USART == 4
            /*To be updated*/
           /* while ((USART3->SR & USART_FLAG_RXNE) == (uint16_t)RESET) {}
            char c = (char)(USART3->DR & (uint16_t)0x01FF);*/
#endif
            *ptr++ = c;
            num++;
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }
    return num;
}

/*
 stat
 Status of a file (by name). Minimal implementation:
 int    _EXFUN(stat,( const char *__path, struct stat *__sbuf ));
 */

int _stat(const char *filepath, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

/*
 times
 Timing information for current process. Minimal implementation:
 */

clock_t _times(struct tms *buf) {
    return -1;
}

/*
 unlink
 Remove a file's directory entry. Minimal implementation:
 */
int _unlink(char *name) {
    errno = ENOENT;
    return -1;
}

/*
 wait
 Wait for a child process. Minimal implementation:
 */
int _wait(int *status) {
    errno = ECHILD;
    return -1;
}

/*
 delay
 perform a delay in ms
 */
void _delay( uint32_t time_in_ms )
{
  uint32_t startTick = GET_TICK_COUNT( );
    while( ( GET_TICK_COUNT( ) - startTick ) < TICK_RATE_MS( time_in_ms ) );
}

