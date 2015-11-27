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


#include "libUDB.h"
#include "heartbeat.h"
#include "servoOut.h"
#include "radioIn.h"
#include "mpu6000.h"

union udb_fbts_byte udb_flags;

// Functions only included with nv memory.
#if (USE_NV_MEMORY == 1)
UDB_SKIP_FLAGS udb_skip_flags = {0, 0, 0};

void udb_skip_radio_trim(boolean b)
{
	udb_skip_flags.skip_radio_trim = 1;
}

void udb_skip_imu_calibration(boolean b)
{
	udb_skip_flags.skip_imu_cal = 1;
}
#endif // (USE_NV_MEMORY == 1)

void udb_init(void)
{
	udb_heartbeat_counter = 0;
	udb_flags.B = 0;
	sil_radio_on = 1;
//	sil_ui_init(mp_rcon);
//	MPU6000_init(&heartbeat); // initialise mpu from udb_init() from matrixpilot_init() - NOTE: now already called in hardware initialisation

////////////////////////////////////////////////////////////
// THIS BLOCK WAS BEING CALLED BEFORE SCHEDULER WAS STARTED
	led_off(LED_RED);
	led_off(LED_BLUE);
	led_off(LED_GREEN);
	led_off(LED_ORANGE);

	radioIn_init(); // was udb_init_capture();
	servoOut_init(); // was udb_init_pwm()
	MPU6000_init(&heartbeat); // initialise mpu in STM32 builds
////////////////////////////////////////////////////////////
}

int tsirq = 0;

void TAMP_STAMP_IRQHandler(void)
{
	tsirq = 1;
}

void udb_run(void) // traditionally only idled or stopped the clock
{
//			udb_flags._.radio_on = (sil_radio_on &&
//			    udb_pwIn[3] >= FAILSAFE_INPUT_MIN &&
//			    udb_pwIn[3] <= FAILSAFE_INPUT_MAX);
////			    udb_pwIn[FAILSAFE_INPUT_CHANNEL] >= FAILSAFE_INPUT_MIN &&
////			    udb_pwIn[FAILSAFE_INPUT_CHANNEL] <= FAILSAFE_INPUT_MAX);

//			udb_heartbeat_40hz_callback(); // Run at 40Hz
//			udb_heartbeat_callback(); // Run at HEARTBEAT_HZ
//			udb_heartbeat_counter++;

//		HAL_Delay(25); // should give us very roughly 40Hz

		if (tsirq)
		{
			tsirq = 0;
			printf("#");
//			fflush(stdout);
		}
}
