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


// TODO: consider renaming this module, ie. pilot.c / autopilot.c

#include "defines.h"
#include "navigate.h"
#include "behaviour.h"
#include "mode_switch.h"
#include "servoMix.h"
#include "servoPrepare.h"
#include "MAVLink.h"
#include "telemetry.h"
#include "flightplan-waypoints.h"
#include "libCntrl.h"
#include "airspeedCntrl.h"
#include "cameraCntrl.h"
#include "../libUDB/heartbeat.h"
#include "../libUDB/servoOut.h"
#include "../libUDB/osd.h"
#include "osd_config.h"
#include "mp_osd.h"
#include "mavlink_options.h"

int16_t pitch_control;
int16_t roll_control;
int16_t yaw_control;
int16_t throttle_control;
uint16_t wind_gain;

void manualPassthrough(void);

void init_servoPrepare(void) // initialize the PWM
{
	int16_t i;

#if (USE_NV_MEMORY == 1)
	if (udb_skip_flags.skip_radio_trim == 1)
		return;
#endif

	for (i = 0; i <= NUM_INPUTS; i++)
	{
#if (FIXED_TRIMPOINT == 1)
		udb_pwTrim[i] = udb_pwIn[i] = ((i == THROTTLE_INPUT_CHANNEL) ? THROTTLE_TRIMPOINT : CHANNEL_TRIMPOINT);
#else
		udb_pwIn[i] = udb_pwTrim[i] = ((i == THROTTLE_INPUT_CHANNEL) ? 0 : 3000);
#endif
	}

	for (i = 0; i <= NUM_OUTPUTS; i++)
	{
#if (FIXED_TRIMPOINT == 1)
		udb_pwOut[i] = ((i == THROTTLE_OUTPUT_CHANNEL) ? THROTTLE_TRIMPOINT : CHANNEL_TRIMPOINT);
#else
		udb_pwOut[i] = ((i == THROTTLE_OUTPUT_CHANNEL) ? 0 : 3000);
#endif
	}

#if (NORADIO == 1)
	udb_pwIn[MODE_SWITCH_INPUT_CHANNEL] = udb_pwTrim[MODE_SWITCH_INPUT_CHANNEL] = 4000;
#endif
}

// Called at HEARTBEAT_HZ
//void dcm_servo_callback_prepare_outputs(void)
void dcm_heartbeat_callback(void)
{
	if (dcm_flags._.calib_finished)
	{
		if (udb_heartbeat_counter % (HEARTBEAT_HZ/40) == 0)
		{
			flight_mode_switch_2pos_poll(); // we always want this called at 40Hz
		}
#if (DEADRECKONING == 1)
		navigate_process_flightplan();
#endif
#if (ALTITUDE_GAINS_VARIABLE == 1)
		airspeedCntrl();
#endif // ALTITUDE_GAINS_VARIABLE
		updateBehavior();
		wind_gain = wind_gain_adjustment();
		rollCntrl();
		yawCntrl();
		altitudeCntrl();
		pitchCntrl();
		servoMix();
		cameraCntrl();
		cameraServoMix();
		updateTriggerAction();
	}
	else
	{
		// otherwise, there is not anything to do
		manualPassthrough();                // Allow manual control while starting up
	}
	
	if (dcm_flags._.calib_finished)         // start telemetry after calibration
	{
#if (USE_MAVLINK == 1)
		// Poll the MAVLink subsystem at 40hz
		if (udb_heartbeat_counter % (HEARTBEAT_HZ/40) == 0)
		{
			mavlink_output_40hz();
		}
#else
		// Send telemetry updates at 8hz
		if (udb_heartbeat_counter % (HEARTBEAT_HZ/8) == 0)
		{
// RobD			flight_state_8hz();
			telemetry_output_8hz();
		}
#endif // (USE_MAVLINK == 1)
	}

	// Poll the OSD subsystem at 8hz
	if (udb_heartbeat_counter % (HEARTBEAT_HZ/8) == 0)
	{
#if (USE_OSD == OSD_NATIVE)
		mp_osd_run_step(); // TODO: this was being called at HEARTBEAT_HZ (investigate) - RobD
//		mp_osd_run_step(udb_heartbeat_counter); // TODO: this was being called at HEARTBEAT_HZ (investigate) - RobD
#elif (USE_OSD == OSD_REMZIBI)
void remzibi_osd_8hz(void);
		remzibi_osd_8hz();
#elif (USE_OSD == OSD_MINIM)
void minim_osd_8hz(void);
		minim_osd_8hz();
#endif // USE_OSD
	}
}

void manualPassthrough(void)
{
	roll_control = pitch_control = yaw_control = throttle_control = 0;
	servoMix();
}
