//
//  filesys.c
//  MatrixPilot
//
//  Created by Robert Dickenson on 17/6/2014.
//  Copyright (c) 2014 MatrixPilot. All rights reserved.
//

//#include <_ansi.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <sys/fcntl.h>
//#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <reent.h>
#include <unistd.h>
#include <stdlib.h> // for malloc

#include "../libUDB/libUDB.h"

//#if (BOARD_TYPE == PX4_BOARD)

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "filesys.h"

#undef errno
extern int errno;

FATFS SDFatFs;  /* File system object for SD card logical drive */

int filesys_test(void)
{
	int fh;
	uint8_t wtext[] = "MatrixPilot low level file support";

//	int oflags = O_RDONLY;
	int oflags = O_WRONLY | O_CREAT;
//	int oflags = O_RDWR | O_CREAT;

    DPRINT("filesys_test()\r\n");

	fh = open("test.txt", oflags);
	if (fh != -1)
	{
//		size_t byteswritten = write(fh, wtext, sizeof(wtext)); // sizeof returns 32
		size_t byteswritten = write(fh, wtext, strlen(wtext));
        if ((byteswritten == 0))
        {
			printf("write() - FAILED\r\n");
        }
		close(fh);
	} else {
		printf("open() - FAILED\r\n");
	}
	return 0;
}

int filesys_ftest(void)
{
	FILE* fp;
	uint8_t wtext[] = "MatrixPilot stream support";

    DPRINT("filesys_ftest()\r\n");

	fp = fopen("ftest.txt", "w+");
	if (fp)
	{
		size_t byteswritten = fwrite(wtext, 1, sizeof(wtext), fp);
        if ((byteswritten == 0))
        {
			printf("fwrite() - FAILED\r\n");
        }
		fclose(fp);
	} else {
		printf("fopen() - FAILED\r\n");
	}
	return 0;
}

int filesys_f_test(void)
{
    FRESULT res;                                            /* FatFs function common result code */
	FIL fil;                                             /* File object */
	uint8_t wtext[] = "MatrixPilot FatFs library support";  /* File write buffer */
    uint32_t byteswritten;                                  /* File write counts */

    DPRINT("filesys_f_test()\r\n");

	res = f_open(&fil, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if (res == FR_OK)
	{
		res = f_write(&fil, wtext, sizeof(wtext), (void*)&byteswritten);
        if ((byteswritten == 0) || (res == FR_OK))
        {
			f_close(&fil);
			return 1;
        } else {
			printf("f_write() - FAILED\r\n");
		}
	} else {
		printf("f_open() - FAILED\r\n");
	}
	return 0;
}

void filesys_format(void)
{
	filesys_test();
	filesys_ftest();
}

void filesys_chkdsk(void)
{
//	printf("filesys_chkdsk() - TODO\r\n");
	filesys_f_test();
}

void filesys_dir(char* arg)
{
	FRESULT fr;
	DIR dp;
	FILINFO fno;

	if (arg == NULL) {
		arg = "*.*";
	}
	fr = f_findfirst(&dp, &fno, "", arg);
	while (fr == FR_OK && fno.fname[0]) {
		printf("%s\t%lu\r\n", fno.fname, fno.fsize);
		fr = f_findnext(&dp, &fno);
	}
	f_closedir(&dp);
}

void filesys_cat(char* arg)
{
	FRESULT fr;
	FIL fil;
	char buf[2];
	UINT br;

	if (arg == NULL) {
		printf("argument required\r\n");
		return;
	}
//	mode &= FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW;
	fr = f_open(&fil, arg, FA_READ);
	if (fr == FR_OK) {
		while ((f_read(&fil, buf, 1, &br) == FR_OK) && br) {
//			if (!br) break;
			printf("%c", buf[0]);
		}
		f_close(&fil);
	} else {
		printf("failed to open %s\r\n", arg);
	}
}

int filesys_init(void)
{
    FRESULT res;

    DPRINT("filesys_init()\r\n");

    /*## FatFS: Link the SD driver ###########################*/
//    if (FATFS_LinkDriver(&SD_Driver, SD_Path) == 0)
	if (1)
    {
        /*##-2- Register the file system object to the FatFs module ##############*/
        res = f_mount(&SDFatFs, (TCHAR const*)SD_Path, 0);
        if (res == FR_OK)
        {
			return 1;
        } else {
			printf("f_mount() - FAILED\r\n");
		}
    } else {
		printf("FATFS_LinkDriver() - FAILED\r\n");
    }
    /*##-11- Unlink the micro SD disk I/O driver ###############################*/
//    FATFS_UnLinkDriver(SD_Path);
	return 0;
}

void* fil_open_r(const char* path, int flags, int _mode)
{
	FRESULT res = -1;
//	int i;
    //FRESULT f_open(FIL* fp, const TCHAR* path, BYTE mode);

	BYTE mode = FA_READ; // 0

	if (flags & O_RDWR) mode |= FA_WRITE;
	if (flags & O_WRONLY) mode |= FA_WRITE;
	if (flags & O_CREAT) mode |= FA_CREATE_ALWAYS;
//	if (flags & O_RDONLY) mode |= ;
//	if (flags & ) mode |= ;
//	mode &= FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW;

	FIL* fil = (FIL*)malloc(sizeof(FIL));
	if (fil)
	{
//		res = f_open(fil, path, FA_CREATE_ALWAYS | FA_WRITE);
		res = f_open(fil, path, mode);
		if (res == FR_OK) {
		} else {
			free(fil);
			fil = NULL;
		}
	}
	return fil;
}

/*
 The normal return value from close is 0; a value of -1 is returned in case of failure.
 The following errno error conditions are defined for this function:
  EBADF: The argument is not a valid file descriptor.
 */
int fil_close_r(void* dev)
{
	FRESULT res = -1;

	if (dev != NULL)
	{
		res = f_close((FIL*)dev);
		if (res == FR_OK) {
			res = 0;
		} else {
			res = -1;
		}
		free(dev);
	}
	return res;
}

long fil_write_r(const void* dev, const char* ptr, int len)
{
	FRESULT res = -1;

	if (dev != NULL)
	{
		int byteswritten;
		res = f_write((FIL*)dev, ptr, len, (void*)&byteswritten);
		if (res == FR_OK)
		{
			return byteswritten;
		}
	}
	errno = EBADF;
	return res;
}

long fil_read_r(const void* dev, char* ptr, int len)
{
	FRESULT res = -1;

	if (dev != NULL)
	{
		int bytesread;
		res = f_read((FIL*)dev, ptr, len, (void*)&bytesread);
		if (res == FR_OK)
		{
			return bytesread;
		}
	}
	errno = EBADF;
	return 0;
}
