// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009, 2015 MatrixPilot Team
// See the AUTHORS.TXT file for a list of authors of MatrixPilot.
//
// MatrixPilot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MatrixPilot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MatrixPilot.  If not, see <http://www.gnu.org/licenses/>.


#include "../MatrixPilot/defines.h"
#include "mavlink_options.h"

#if (USE_MAVLINK == 1)

#include "../MatrixPilot/MAVLink.h"
#include "../MAVLinkIO.h"
//#include <string.h>
//#include <stdarg.h>

#if (MAVLINK_TEST_ENCODE_DECODE == 1)
mavlink_message_t last_msg;
#define _ADDED_C_LIB 1 // Needed to get vsnprintf()
#include <stdio.h>
#include <stdarg.h>
#define MAVLINK_TEST_MESSAGE_SIZE 100
static uint8_t mavlink_test_message_buffer[MAVLINK_TEST_MESSAGE_SIZE];
static int16_t mavlink_tests_pass = 0;
static int16_t mavlink_tests_fail = 0;
static char mavlink_test_first_pass_flag = 1;

#define MAVLINK_ASSERT(exp) \
	if (!(exp)) \
	{ \
		printf("MAVLink Test Fail: " \
		       "at %s, line %d.\r\n", __FILE__, __LINE__); \
		mavlink_tests_fail++; \
	} else { \
		mavlink_tests_pass++; \
	}

#include "../MAVLink/include/matrixpilot/testsuite.h"

// add printf library when running tests to output ascii messages of test results
static void serial_output(const char* format, ...)
{
	int16_t remaining = 0;
	int16_t wrote = 0;
	va_list arglist;

	va_start(arglist, format);
	remaining = MAVLINK_TEST_MESSAGE_SIZE;
	wrote = vsnprintf((char*)(&mavlink_test_message_buffer[0]), (size_t)remaining, format, arglist);
	if (wrote > 0)
	{
		mavlink_serial_send(MAVLINK_COMM_0, &mavlink_test_message_buffer[0], (uint16_t)wrote);
//		printf("%s\r\n", mavlink_test_message_buffer);
	}
}

void mp_mavlink_transmit(uint8_t ch)
// This is a special version of the routine for testing MAVLink routines
// The incoming serial stream is parsed to reproduce a mavlink message.
// This will then be checked against the original message and results recorded
// using the MAVLINK_ASSERT macro.
{
	mavlink_status_t r_mavlink_status;

	mavlink_parse_char(0, ch, &last_msg, &r_mavlink_status);
}

void mavlink_output_40hz(void)
{
	if (mavlink_test_first_pass_flag == 1)
	{
		serial_output("\r\nRunning MAVLink encode / decode Tests.\r\n");
		// reset serial buffer in preparation for testing against buffer
		mavlink_tests_pass = 0;
		mavlink_tests_fail = 0;
		mavlink_test_all(mavlink_system.sysid, mavlink_system.compid, &last_msg);
		serial_output("\r\nMAVLink Tests Pass: %d\r\nMAVLink Tests Fail: %d\r\n", mavlink_tests_pass, mavlink_tests_fail);
		mavlink_test_first_pass_flag = 0;
	}
}

#endif // (MAVLINK_TEST_ENCODE_DECODE == 1)

#endif // (USE_MAVLINK == 1)
