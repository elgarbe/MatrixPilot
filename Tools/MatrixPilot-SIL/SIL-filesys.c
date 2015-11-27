//
//  SIL-udb.c
//  MatrixPilot-SIL
//
//  Created by Ben Levitt on 2/1/13.
//  Copyright (c) 2013 MatrixPilot. All rights reserved.
//

#if (WIN == 1 || NIX == 1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../libUDB/libUDB.h"
#include "SIL-config.h"

void filesys_chkdsk(void)
{
	printf("filesys_chkdsk() - WIN32 TODO\r\n");
}

void filesys_dir(char* arg)
{
	if (arg == NULL) {
		arg = "*.*";
	}
	printf("filesys_dir %s - WIN32 TODO\r\n", arg);
}

//size_t FSfread(void *ptr, size_t size, size_t n, FSFILE *stream);
void filesys_cat(char* arg)
{
	printf("filesys_cat(%s) - WIN32 TODO\r\n", arg);
}

void filesys_format(void)
{
}

int filesys_init(void)
{
	printf("filesys_init() - nothing to do on WIN32\r\n");
	return 1;
}

#endif // (WIN == 1 || NIX == 1)
