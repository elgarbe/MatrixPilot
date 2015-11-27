//
//  libSTM.c
//  MatrixPilot
//
//  Created by Robert Dickenson on 17/6/2014.
//  Copyright (c) 2014 MatrixPilot. All rights reserved.
//

#include "../libUDB/libUDB.h"
#include "../libUDB/events.h"
//#include "../libUDB/heartbeat.h"

#include "stm32f4xx_hal.h"
//#include "cmsis_os.h"
//#include "serialIO.h"

//
// mostly everything in the various libUDB/DCM/STM modules will
// ideally be moved to functionality specific modules.
//
// in the beta branch development has started on abstracting
// the various potential input control sources via module selectIn
// which complicates the answer in this branch
//

uint16_t mp_rcon = 3;                           // default RCON state at normal powerup

uint8_t sil_radio_on;

union longww battery_current;                   // battery_current._.W1 is in tenths of Amps
union longww battery_mAh_used;                  // battery_mAh_used._.W1 is in mAh
union longww battery_voltage;                   // battery_voltage._.W1 is in tenths of Volts
uint8_t rc_signal_strength;                     // rc_signal_strength is 0-100 as percent of full signal
int16_t vref_adj;

volatile uint16_t trap_flags;
volatile uint32_t trap_source;
volatile uint16_t osc_fail_count;

int FSInit(void) { return 1; }

//NOTE: This is implemented on servoOut
//int16_t udb_servo_pulsesat(int32_t pw)
//{
//	if (pw > SERVOMAX) pw = SERVOMAX;
//	if (pw < SERVOMIN) pw = SERVOMIN;
//	return (int16_t)pw;
//}

void udb_servo_record_trims(void)
{
	int16_t i;

	for (i = 1; i <= NUM_INPUTS; i++)
	{
		udb_pwTrim[i] = udb_pwIn[i];
//		DPRINT("udb_pwTrim[%i] = %u\r\n", i, udb_pwTrim[i]);
	}
}

uint16_t get_reset_flags(void)
{
	return mp_rcon;
}

//uint16_t register_event_p(void (*event_callback)(void), eventPriority priority) { return 0; }
//void trigger_event(uint16_t hEvent) {}


void _StartDefaultTask(void const * argument)
{
	setvbuf(stdout, NULL, _IONBF, 0);

	printf("MatrixPilot STM32-nucleo\r\n");

	matrixpilot_init(); // initialise MP from default CMSOS task in STM32 build

	for(;;)
	{
		matrixpilot_loop(); // calls udb_run() as implemented above

void _C1Interrupt(void); //_EVENTM_INTERUPT
void _C2Interrupt(void); //_EVENTL_INTERUPT
		_C1Interrupt();
		_C2Interrupt();
	}
}
