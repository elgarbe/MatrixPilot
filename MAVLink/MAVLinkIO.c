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

#include "../MatrixPilot/telemetry_log.h"
#include "../MatrixPilot/MAVLink.h"
#include "../libUDB/syscalls.h"
#include "../libUDB/serialIO.h"
#include "../libUDB/events.h"
#include "../MAVLinkIO.h"
#include <string.h>
#include <stdarg.h>

int mavlink_fh = 0;
mavlink_status_t r_mavlink_status;
uint8_t handling_of_message_completed = true;

static mavlink_message_t msg[2];
static uint8_t mavlink_message_index = 0;
static uint16_t mavlink_process_message_handle = INVALID_HANDLE;
static uint16_t mavlink_process_output_handle = INVALID_HANDLE;

#if (MAVLINK_TEST_ENCODE_DECODE != 1)
void mp_mavlink_transmit(uint8_t ch)
// routine to send a single character used by MAVlink standard include routines.
// We forward to multi-byte sending routine so that firmware can interleave
// ascii debug messages with MAVLink binary messages without them overwriting the buffer.
{
//printf("mp_mavlink_transmit(%u)\r\n", ch);
	mavlink_serial_send(MAVLINK_COMM_0, &ch, 1);
}

void mavlink_output_40hz(void)
{
	trigger_event(mavlink_process_output_handle);
}
#endif // (MAVLINK_TEST_ENCODE_DECODE == 1)

void mav_printf(const char* format, ...)
{
	char buf[200];
	va_list arglist;

	va_start(arglist, format);
	vsnprintf(buf, sizeof(buf), format, arglist);
	// mavlink_msg_statustext_send(MAVLINK_COMM_1, severity, text);
	// severity: Severity of status, 0 = info message, 255 = critical fault (uint8_t)
	mavlink_msg_statustext_send(MAVLINK_COMM_0, 0, buf);
	va_end(arglist);
}

#if (PX4 == 1)

#include "usart.h"
#include "stm32f4xx_hal.h"

//int16_t mavlink_serial_send(mavlink_channel_t UNUSED(chan), const uint8_t buf[], uint16_t len)
int16_t mavlink_serial_send(mavlink_channel_t chan, const uint8_t buf[], uint16_t len)
{
	HAL_StatusTypeDef status;
	status = HAL_UART_Transmit(&UART_MAV, buf, len, 100);
	if (status == HAL_OK)
	{
		return 1;
	}

//int _write(int file, char *ptr, int len);
//	return _write(mavlink_fh, buf, len);
	return -1;
}

static int16_t mavlink_callback_get_byte_to_send(void)
{
	return -1;
}


void mavlink_poll(void)
{
	static mavlink_message_t msg;
	int chan = 0;
//	mavlink_status_t r_mavlink_status;
	char byte;

	while (read(mavlink_fh, &byte, 1) > 0)
	{
//		uint8_t byte = serial.getNextByte();
#if 0
//		mavlink_callback_received_byte(byte);
#else
		if (mavlink_parse_char(chan, byte, &msg, &r_mavlink_status))
		{
			mavlink_input_msg(&msg);
			printf("Received message with ID %d, sequence: %d from component %d of system %d", msg.msgid, msg.seq, msg.compid, msg.sysid);
		}
#endif
	}
}

#else

#define SERIAL_BUFFER_SIZE  MAVLINK_MAX_PACKET_LEN

static int16_t sb_index = 0;
static int16_t end_index = 0;
static char serial_interrupt_stopped = 1;
static uint8_t serial_buffer[SERIAL_BUFFER_SIZE];

//int16_t udb_serial_callback_get_byte_to_send(void)
int16_t mavlink_callback_get_byte_to_send(void)
{
	if (sb_index < end_index && sb_index < SERIAL_BUFFER_SIZE) // ensure never end up racing thru memory.
	{
		uint8_t txchar = serial_buffer[sb_index++];
		return txchar;
	}
	else
	{
		serial_interrupt_stopped = 1;
	}
	return -1;
}

//int16_t mavlink_serial_send(mavlink_channel_t UNUSED(chan), uint8_t buf[], uint16_t len)
int16_t mavlink_serial_send(mavlink_channel_t chan, const uint8_t buf[], uint16_t len) // RobD
// Note: Channel Number, chan, is currently ignored.
{
	int16_t start_index;
	int16_t remaining;

#if (USE_TELELOG == 1)
//printf("calling log_telemetry with %u bytes\r\n", len);
	log_telemetry(buf, len);
#endif // USE_TELELOG

	// Note at the moment, all channels lead to the one serial port
	if (serial_interrupt_stopped == 1)
	{
		sb_index = 0;
		end_index = 0;
	}
	start_index = end_index;
	remaining = SERIAL_BUFFER_SIZE - start_index;

//	printf("%u\r\n", remaining);

	if (len > remaining)
	{
		// Chuck away the entire packet, as sending partial packet
		// will break MAVLink CRC checks, and so receiver will throw it away anyway.
		return (-1);
	}
	if (remaining > 1)
	{
		memcpy(&serial_buffer[start_index], buf, len);
		end_index = start_index + len;
	}
	if (serial_interrupt_stopped == 1)
	{
		serial_interrupt_stopped = 0;
#if (SILSIM == 1) // TODO: RobD: why is this here?
		mavlink_start_sending_data();
#else
		udb_serial_start_sending_data();
#endif
	}
	return (1);
}

#endif // (PX4 == 1)

void send_text(uint8_t text[])
{
	uint16_t index = 0;

	while (text[index++] != 0 && index < 80)
	{
		; // Do nothing, just measuring the length of the text
	}
//printf("send_text(%s) %u\r\n", text, index);
	mavlink_serial_send(MAVLINK_COMM_0, text, index - 1);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// A simple routine for sending a uint8_t number as 2 bytes of hexadecimal text
//static void send_uint8(uint8_t value)
//{
//	uint8_t temp;
//	temp = value >> 4; // Take upper half of hex int.
//	if (temp < 10)
//	{
//		mp_mavlink_transmit(temp + 0x30); //1,2,3,4,5,6,7,8,9
//	}
//	else
//	{
//		mp_mavlink_transmit(temp - 10 + 0x41); // A,B,C,D,E,F
//	}
//	temp = value & 0x0f; // Take lower half of hex int
//	if (temp < 10)
//	{
//		mp_mavlink_transmit(temp + 0x30); //1,2,3,4,5,6,7,8,9
//	}
//	else
//	{
//		mp_mavlink_transmit(temp - 10 + 0x41); // A,B,C,D,E,F
//	}
//}


/*
typedef struct __mavlink_message {
	uint16_t checksum; /// sent at end of packet
	uint8_t magic;   ///< protocol magic marker
	uint8_t len;     ///< Length of payload
	uint8_t seq;     ///< Sequence of packet
	uint8_t sysid;   ///< ID of message sender system/aircraft
	uint8_t compid;  ///< ID of the message sender component
	uint8_t msgid;   ///< ID of message in payload
	uint64_t payload64[(MAVLINK_MAX_PAYLOAD_LEN+MAVLINK_NUM_CHECKSUM_BYTES+7)/8];
} mavlink_message_t;
 */
////////////////////////////////////////////////////////////////////////////////////////
//
// MAIN MATRIXPILOT MAVLINK CODE FOR RECEIVING COMMANDS FROM THE GROUND CONTROL STATION
//

static void handleMessage(void)
{
	mavlink_message_t* handle_msg;

	if (mavlink_message_index == 0)
	{
		handle_msg = &msg[1];
	}
	else
	{
		handle_msg = &msg[0];
	}
	mavlink_input_msg(handle_msg);
}

//void udb_serial_callback_received_byte(uint8_t rxchar)
void mavlink_callback_received_byte(uint8_t rxchar)
{
	if (mavlink_parse_char(0, rxchar, &msg[mavlink_message_index], &r_mavlink_status))
	{
		// Check that handling of previous message has completed before calling again
		if (handling_of_message_completed == true)
		{
			// Switch between incoming message buffers
			if (mavlink_message_index == 0) mavlink_message_index = 1;
			else mavlink_message_index = 0;
			handling_of_message_completed = false;
			trigger_event(mavlink_process_message_handle);
#if (PX4 == 1)
//			handleMessage(); // event mechanism not as yet ported, so call message handler directly
#endif
		}
	}
}

void MAVLinkIO_init(void)
{
#ifndef SERIAL_BAUDRATE
#define SERIAL_BAUDRATE 57600 // default
#pragma warning "SERIAL_BAUDRATE set to default value of 57600 bps for MAVLink"
#endif
	udb_serial_set_rate(SERIAL_BAUDRATE);
	mav_init_USART(&mavlink_callback_get_byte_to_send, &mavlink_callback_received_byte);

	mavlink_message_index = 0;
	mavlink_process_message_handle = register_event_p(&handleMessage, EVENT_PRIORITY_MEDIUM);
	mavlink_process_output_handle = register_event_p(&mavlink_output_40hz_handler, EVENT_PRIORITY_MEDIUM);

	mavlink_fh = open("com2", O_RDWR);
	if (mavlink_fh != -1) {
		close(mavlink_fh);
	}
}

#endif // (USE_MAVLINK == 1)
