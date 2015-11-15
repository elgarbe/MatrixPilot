//
//  syscalls.h
//  MatrixPilot
//
//  Created by Robert Dickenson on 16/11/2015.
//  Copyright (c) 2015 MatrixPilot. All rights reserved.
//

#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

//#if (WIN == 1 || NIX == 1)
//#else
//#include <dsp.h>
//#endif

#if (PX4 == 1)
#include <sys/fcntl.h>
#include <unistd.h>
#else

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   4
#define O_CREAT  8

int open(const char name, int flags, ...);
int close(int fh);

#endif

#endif // _SYSCALLS_H_

