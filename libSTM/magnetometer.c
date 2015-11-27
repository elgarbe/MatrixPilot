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
//#include "../libUDB/heartbeat.h"
#include "../libUDB/magnetometer.h"
#include "magnetometerOptions.h"

//#include "stm32f4xx_hal.h"
//#include "cmsis_os.h"
//#include "dma.h"
//#include "i2c.h"

int16_t udb_magFieldBody[3];                    // magnetic field in the body frame of reference
int16_t udb_magOffset[3] = { 0 , 0 , 0 };       // magnetic offset in the body frame of reference
int16_t magGain[3] = { RMAX , RMAX , RMAX };    // magnetometer calibration gains
int16_t rawMagCalib[3] = { 0 , 0 , 0 };
uint8_t magreg[6];                              // magnetometer read-write buffer
int16_t magFieldRaw[3];
int16_t magMessage;

#if (MAG_YAW_DRIFT == 1)

static magnetometer_callback_funcptr magnetometer_callback = NULL;

void rxMagnetometer(magnetometer_callback_funcptr callback)
{
	magnetometer_callback = callback;
}

void I2C_doneReadMagData(void)
{
	magFieldRaw[0] = (magreg[0]<<8) + magreg[1];
	magFieldRaw[1] = (magreg[2]<<8) + magreg[3];
	magFieldRaw[2] = (magreg[4]<<8) + magreg[5];

	if (magMessage == 7)
	{
		udb_magFieldBody[0] = MAG_X_SIGN((__builtin_mulsu((magFieldRaw[MAG_X_AXIS]), magGain[MAG_X_AXIS]))>>14)-(udb_magOffset[0]>>1);
		udb_magFieldBody[1] = MAG_Y_SIGN((__builtin_mulsu((magFieldRaw[MAG_Y_AXIS]), magGain[MAG_Y_AXIS]))>>14)-(udb_magOffset[1]>>1);
		udb_magFieldBody[2] = MAG_Z_SIGN((__builtin_mulsu((magFieldRaw[MAG_Z_AXIS]), magGain[MAG_Z_AXIS]))>>14)-(udb_magOffset[2]>>1);

		if ((abs(udb_magFieldBody[0]) < MAGNETICMAXIMUM) &&
			(abs(udb_magFieldBody[1]) < MAGNETICMAXIMUM) &&
			(abs(udb_magFieldBody[2]) < MAGNETICMAXIMUM))
		{
//			udb_magnetometer_callback();
			if (magnetometer_callback != NULL)
			{
				magnetometer_callback();
			}
			else
			{
				printf("ERROR: magnetometer_callback function pointer not set\r\n");
			}
		}
		else
		{
			magMessage = 0;         // invalid reading, reset the magnetometer
		}
	}
}

void HILSIM_MagData(magnetometer_callback_funcptr callback)
{
	(void)callback;
//	magnetometer_callback = callback;
	magMessage = 7;                 // indicate valid magnetometer data
	I2C_doneReadMagData();          // run the magnetometer computations
}

#endif // MAG_YAW_DRIFT
