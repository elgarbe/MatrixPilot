// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2011 MatrixPilot Team
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


#include "../libUDB/libUDB.h"
#include "../libUDB/heartbeat.h"
#include "../libDCM/rmat.h"
#include "servoOut.h"
#include "radioIn.h"

int one_hertz_flag = 0;
uint16_t udb_heartbeat_counter = 0;
uint16_t udb_heartbeat_40hz_counter = 0;
#define HEARTBEAT_MAX 57600 // Evenly divisible by many common values: 2^8 * 3^2 * 5^2

int16_t pw[8];

void heartbeat(void) // called from MPU6000 ISR
{
	// Start the sequential servo pulses at frequency SERVO_HZ
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/SERVO_HZ) == 0)
	{
//		start_pwm_outputs();
        radioIn_getInput(pw, 8); // Read the inputs
        set_pwm_outputs(pw);     // Update the servos
	}

	// Capture cpu_timer once per second.
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/1) == 0)
	{
		cpu_load_calc();
		one_hertz_flag = 1;
	}

	// This calls the state machine implemented in MatrixPilot/states.c
	// it is called at high priority to ensure manual control takeover can
	// occur, even if the lower priority tasks hang
	// Call the periodic callback at 40 Hz
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/40) == 0)
	{
		// call the FSM. implemented in states.c
		udb_heartbeat_40hz_callback(); // this was called udb_background_callback_periodic()
		udb_heartbeat_40hz_counter = (udb_heartbeat_40hz_counter+1) % HEARTBEAT_MAX;
	}

	udb_heartbeat_counter = (udb_heartbeat_counter+1) % HEARTBEAT_MAX;
	TriggerTaskIMU();
}

//void pulse(void) // called from TaskIMU
void RunTaskIMU(void) // called from OS TaskIMU
{
//	led_off(LED_BLUE);     // indicates logfile activity

#if (NORADIO != 1)
	// 20Hz testing of radio link
	if ((udb_heartbeat_counter % (HEARTBEAT_HZ/20)) == 1)
	{
//		radioIn_failsafe_check();
/////////////////////////////////////////////////////////////////////
// called from heartbeat pulse at 20Hz
		if (radioIn_isRadioOn())
		{
			udb_flags._.radio_on = 1;
			led_on(LED_GREEN);
		}
		else
		{
			if (udb_flags._.radio_on == 1)
			{
				udb_flags._.radio_on = 0;
				udb_callback_radio_did_turn_off();
			}
			led_off(LED_GREEN);
		}
/////////////////////////////////////////////////////////////////////
	}
	// Computation of noise rate
	// Noise pulses are counted when they are detected, and reset once a second
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/1) == 1)
	{
		radioIn_failsafe_reset();
	}
#endif // (NORADIO != 1)

#ifdef VREF
	vref_adj = (udb_vref.offset>>1) - (udb_vref.value>>1);
#else
	vref_adj = 0;
#endif // VREF

//	calculate_analog_sensor_values();
	udb_callback_read_sensors();
	udb_flags._.a2d_read = 1; // signal the A/D to start the next summation

	// process sensor data, run flight controller, generate outputs. implemented in libDCM.c
	udb_heartbeat_callback(); // this was called udb_servo_callback_prepare_outputs()
}
