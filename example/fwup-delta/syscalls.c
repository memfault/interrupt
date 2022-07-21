#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libopencm3/stm32/usart.h>

// LIBC SYSCALLS
/////////////////////

extern int end;

caddr_t _sbrk(int incr) {
  static unsigned char *heap = NULL;
  unsigned char *prev_heap;

  if (heap == NULL) {
    heap = (unsigned char *)&end;
  }
  prev_heap = heap;

  heap += incr;

  return (caddr_t) prev_heap;
}

int _close(int file) {
  return -1;
}

int _fstat(int file, struct stat *st) {
  st->st_mode = S_IFCHR;

  return 0;
}

int _isatty(int file) {
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  return 0;
}

void _exit(int status) {
  __asm("BKPT #0");
  while (1) {}
}

void _kill(int pid, int sig) {
  return;
}

int _getpid(void) {
  return -1;
}

int _write(int file, char *ptr, int len)
{
    int i;

    if (file == STDOUT_FILENO || file == STDERR_FILENO) {
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n') {
                usart_send_blocking(USART2, '\r');
            }
            usart_send_blocking(USART2, ptr[i]);
        }
        return i;
    }
    errno = EIO;
    return -1;
}

int _read (int file, char * ptr, int len) {
  return -1;
}


