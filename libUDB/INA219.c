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
#include "I2C.h"
#include "INA219.h"

#if (USE_INA219 == 1)

#define INA219_ADDRESS 0x80         // I2C address of INA219
/*
 Como shunt tengo 2 resistencias de 1 mOhm en paralelo! Y la corriente máxima que quiero medir son unos 40A. Con lo que me dará unos 0.8W en ellas

VbusMax = 16V - ya que voy a trabajar con 3S (3*4.2 = 12.6V)
VshuntMax = 0,04V  - Puede ser 40/80/160/320 mV. Esto es porque 40A * 0.0005 = 20mV
Rshunt = 0.0005 ohm

MaxPosibleI = 0.04V/0.0005 ohm = 80A
MaxExpectedI = 40A - Esto es lo que yo considero como máxima corriente esperada en mi sistema.

MinimumLSB = 40A/32767 = 1.22mA
MaximumLSB = 40A/4096 = 9.76mA

De esos dos valores elijo uno que este entre los dos, lo más chico posible para tener presicion 
y en lo posible redondo. Por ejemplo 2mA

CurrentLSB = 2mA
Calibracion = 0.04096 / (2 x10-3 * 0.0005) = 40960
PotLSB = 20 x CurrentLSB = 40mW
 */

#define USE_INA219_ON_I2C 2

static const unsigned char INA219_CalibReg[]  = { 0x05 };   // Address of the Calibration register
static const unsigned char INA219_ConfigReg[]  = { 0x00 };  // Address of the Cofiguration register

static const unsigned char INA219_VBusReg[]  = { 0x02 };    // Address of the V bus register
static const unsigned char INA219_PowerReg[]  = { 0x03 };    // Address of the Power register
static const unsigned char INA219_CurrentReg[]  = { 0x04 };    // Address of the Current register

static unsigned char INA219write_CalibValue[] = { 0xA0, 0x00 };   // Calibration value
static unsigned char INA219write_ConfigValue[] = { 0x29, 0x9F };  // Configuration value

static unsigned char INAData[2];        // Buffer for reister reading

static int INAMessage = 0;      // message type, state machine counter
static int INACalibPause = 0;

void ReadVBus_callback(boolean I2CtrxOK);
void ReadPower_callback(boolean I2CtrxOK);
void ReadCurrent_callback(boolean I2CtrxOK);

#if (USE_INA219_ON_I2C == 1)
	#define I2C_Normal      I2C1_Normal
	#define I2C_Read        I2C1_Read
	#define I2C_Write       I2C1_Write
	#define I2C_Reset       I2C1_Reset
#elif (USE_INA219_ON_I2C == 2)
	#define I2C_Normal      I2C2_Normal
	#define I2C_Read        I2C2_Read
	#define I2C_Write       I2C2_Write
	#define I2C_Reset       I2C2_Reset
#endif


ina219_callback_funcptr ina219_callback = NULL;

void rxINA219(ina219_callback_funcptr callback)  // service the INA219 sensor
{
	ina219_callback = callback;

	if (I2C_Normal() == false)  // if I2C is not ok
	{
		INAMessage = 0;         // start over again
		I2C_Reset();            // reset the I2C
		return;
	}

	if (INACalibPause == 0)
	{
		INAMessage++;
		if (INAMessage > 7)
		{
			INAMessage = 4;
		}
		switch (INAMessage)
		{ 
		case 1:
			I2C_Write(INA219_ADDRESS, INA219_CalibReg, 1, INA219write_CalibValue, 2, NULL);
			break;
		case 2:
			I2C_Write(INA219_ADDRESS, INA219_ConfigReg, 1, INA219write_ConfigValue, 2, NULL);
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			I2C_Read(INA219_ADDRESS, INA219_VBusReg, 1, INAData, 2, &ReadVBus_callback, I2C_MODE_WRITE_ADDR_READ);
			break;
		case 6:
			I2C_Read(INA219_ADDRESS, INA219_PowerReg, 1, INAData, 2, &ReadPower_callback, I2C_MODE_WRITE_ADDR_READ);
			break;
		case 7:
			I2C_Read(INA219_ADDRESS, INA219_CurrentReg, 1, INAData, 2, &ReadCurrent_callback, I2C_MODE_WRITE_ADDR_READ);
			break;
		default:
			INAMessage = 0;
			break;
		}
	}
	else
	{
		INACalibPause--;
	}
}

void ReadVBus_callback(boolean I2CtrxOK)
{
	unsigned int VBus;

	if (I2CtrxOK == true)
	{
		VBus = (INAData[0] << 8 | INAData[1]);
	}
	else
	{
		// the operation failed - we should probably do something about it...
	}
}

void ReadPower_callback(boolean I2CtrxOK)
{
	unsigned int Power;

	if (I2CtrxOK == true)
	{
		Power = (INAData[0] << 8 | INAData[1]);
	}
	else
	{
		// the operation failed - we should probably do something about it...
	}
}

void ReadCurrent_callback(boolean I2CtrxOK)
{
	unsigned int Current;

	if (I2CtrxOK == true)
	{
		Current = (INAData[0] << 8 | INAData[1]);
	}
	else
	{
		// the operation failed - we should probably do something about it...
	}
}
#endif // INA219
