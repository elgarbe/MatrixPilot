/* Support files for GNU libc.  Files in the system namespace go here.
   Files in the C namespace (ie those that do not start with an
   underscore) go in .c.  */

#include <_ansi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h> // for malloc

#include "stm32f4xx_hal.h"
#include "fatfs.h"
#include "usart.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h" // ../libUDB/uart.h

#include "filesys.h"
#include "assert.h"

#undef errno
extern int errno;

#define FreeRTOS
#define MAX_STACK_SIZE 0x200

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

#ifndef FreeRTOS
  register char * stack_ptr asm("sp");
#endif

caddr_t _sbrk(int incr)
{
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end,*min_stack_ptr;

	if (heap_end == 0)
		heap_end = &end;
	prev_heap_end = heap_end;
#ifdef FreeRTOS
	/* Use the NVIC offset register to locate the main stack pointer. */
	min_stack_ptr = (char*)(*(unsigned int *)*(unsigned int *)0xE000ED08);
	/* Locate the STACK bottom address */
	min_stack_ptr -= MAX_STACK_SIZE;
	if (heap_end + incr > min_stack_ptr)
#else
	if (heap_end + incr > stack_ptr)
#endif
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
 * _gettimeofday primitive (Stub function)
 * */
int _gettimeofday (struct timeval * tp, struct timezone * tzp)
{
  /* Return fixed data for the timezone.  */
  if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }

  return 0;
}

//void initialise_monitor_handles()
//{
//}

int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void _exit(int status)
{
	_kill(status, -1);
	while (1) {}
}

int trap_handling = 0;

void _out(char ch)
{
	if (trap_handling)
	{
		PutChar(ch);
	}
	else
	{
		__io_putchar(ch);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

typedef struct {
	const char* name;
	void* (*open_r)(const char* path, int flags, int mode);
	int (*close_r)(void* dev);
	long (*write_r)(const void* dev, const char* ptr, int len);
	long (*read_r)(const void* dev, char* ptr, int len);
} devoptab_t;

const devoptab_t devoptab_com = { "com", com_open_r, com_close_r, com_write_r, com_read_r };
const devoptab_t devoptab_spi = { "spi", spi_open_r, spi_close_r, spi_write_r, spi_read_r };
const devoptab_t devoptab_i2c = { "i2c", i2c_open_r, i2c_close_r, i2c_write_r, i2c_read_r };
const devoptab_t devoptab_fil = { "fil", fil_open_r, fil_close_r, fil_write_r, fil_read_r };

typedef struct {
	const char* name;
	const devoptab_t* devoptab;
	void* deveopdata;
} devtab_t;

devtab_t devtab_list[] = {
	{ "stdin",  &devoptab_com, &UART_CON, }, // standard input
	{ "stdout", &devoptab_com, &UART_CON, }, // standard output
	{ "stderr", &devoptab_com, &UART_CON, }, // standard error
	{ "com1",   &devoptab_com, &huart1, }, // serial port com1
	{ "com2",   &devoptab_com, &huart2, }, // serial port com2
	{ "com6",   &devoptab_com, &huart6, }, // serial port com6
	{ "spi1",   &devoptab_spi, &hspi1, },  // spi bus 1
	{ "spi2",   &devoptab_spi, &hspi2, },  // spi bus 2
	{ "i2c1",   &devoptab_i2c, &hi2c1, },  // i2c bus 1
	{ "i2c3",   &devoptab_i2c, &hi2c3, },  // i2c bus 3
	{ NULL,     &devoptab_fil, NULL, },    // file handle 1
	{ NULL,     &devoptab_fil, NULL, },    // file handle 2
	{ NULL,     &devoptab_fil, NULL, },    // file handle 3
	{ NULL,     0,             NULL, }     // terminate list
};

int _open(char *path, int flags, int mode)
{
	int i = 0;
	int fh = -1;

	do {
		if (devtab_list[i].name != NULL) {
			if (strcmp(devtab_list[i].name, path) == 0) {
//				fh = i;
				break;
			}
		} else {
			if (devtab_list[i].deveopdata == NULL) {
//				fh = i;
				break;
			}
		}
	} while (devtab_list[++i].devoptab);

	if (devtab_list[i].devoptab) {
		devtab_list[i].deveopdata = devtab_list[i].devoptab->open_r(path, flags, mode);
		if (devtab_list[i].deveopdata) {
			fh = i;
		}
	} else {
		errno = ENODEV;
	}
/*
	if (fh != -1) {
		void* dev = devtab_list[fh].devoptab;
		if (dev != NULL) {
			devtab_list[fh].deveopdata = devtab_list[fh].devoptab->open_r(path, flags, mode);
			if (devtab_list[fh].deveopdata == NULL) {
				fh = -1;
			}
		} else {
			fh = -1;
		}
	} else {
		errno = ENODEV;
	}
 */
	return fh;
}

/*
 The normal return value from close is 0; a value of -1 is returned in case of failure.
 The following errno error conditions are defined for this function:
  EBADF: The argument is not a valid file descriptor.
 */
int _close(int fh)
{
	int res;
	assert(fh < sizeof(devtab_list)/sizeof(devtab_list[0]));
	res = devtab_list[fh].devoptab->close_r(devtab_list[fh].deveopdata);
	devtab_list[fh].deveopdata = NULL;
	return res;
}

int _write(int fh, char *ptr, int len)
{
	assert(fh < sizeof(devtab_list)/sizeof(devtab_list[0]));
	return devtab_list[fh].devoptab->write_r(devtab_list[fh].deveopdata, ptr, len);
}

int _read(int fh, char *ptr, int len)
{
	assert(fh < sizeof(devtab_list)/sizeof(devtab_list[0]));
	return devtab_list[fh].devoptab->read_r(devtab_list[fh].deveopdata, ptr, len);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int _fstat(int file, struct stat *st)
{
	FRESULT res = 0;
	st->st_mode = S_IFCHR;
	return res;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	FRESULT res = 0;
	return res;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int _times(struct tms *buf)
{
	return -1;
}

int _stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}


//void _init(void)
//{
//}

#if 0
/**
  * @brief  Update FIFO configuration
  * @param  hpcd: PCD handle
  * @retval status
  */
HAL_StatusTypeDef HAL_PCDEx_SetTxFiFo(PCD_HandleTypeDef *hpcd, uint8_t fifo, uint16_t size)
{
  uint8_t i = 0;
  uint32_t Tx_Offset = 0;


  /*  TXn min size = 16 words. (n  : Transmit FIFO index)
  *   When a TxFIFO is not used, the Configuration should be as follows:
  *       case 1 :  n > m    and Txn is not used    (n,m  : Transmit FIFO indexes)
  *       --> Txm can use the space allocated for Txn.
  *       case2  :  n < m    and Txn is not used    (n,m  : Transmit FIFO indexes)
  *       --> Txn should be configured with the minimum space of 16 words
  *  The FIFO is used optimally when used TxFIFOs are allocated in the top
  *       of the FIFO.Ex: use EP1 and EP2 as IN instead of EP1 and EP3 as IN ones.
  *   When DMA is used 3n * FIFO locations should be reserved for internal DMA registers */

  Tx_Offset = hpcd->Instance->GRXFSIZ;

  if(fifo == 0)
  {
    hpcd->Instance->DIEPTXF0_HNPTXFSIZ = (size << 16) | Tx_Offset;
  }
  else
  {
    Tx_Offset += (hpcd->Instance->DIEPTXF0_HNPTXFSIZ) >> 16;
    for (i = 0; i < (fifo - 1); i++)
    {
      Tx_Offset += (hpcd->Instance->DIEPTXF[i] >> 16);
    }

    /* Multiply Tx_Size by 2 to get higher performance */
    hpcd->Instance->DIEPTXF[fifo - 1] = (size << 16) | Tx_Offset;

  }

  return HAL_OK;
}

/**
  * @brief  Update FIFO configuration
  * @param  hpcd: PCD handle
  * @retval status
  */
HAL_StatusTypeDef HAL_PCDEx_SetRxFiFo(PCD_HandleTypeDef *hpcd, uint16_t size)
{

  hpcd->Instance->GRXFSIZ = size;

  return HAL_OK;
}
#endif
