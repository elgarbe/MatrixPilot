// This file is part of MatrixPilot.
//
//    http://code.google.com/p/gentlenav/
//
// Copyright 2009-2012 MatrixPilot Team
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

// Internal MPU6000 axis definition
// X axis pointing to right, Y axis pointing forward and Z axis pointing up


#include "../libUDB/libUDB.h"
#include "../libUDB/ADchannel.h"
#include "../libUDB/mpu6000.h"
#include "mpu_spi.h"

#if (BOARD_TYPE != UDB4_BOARD)

///////////////////////////////////////////////////////////////////////////////

#define MPU6000_ACCEL_DEFAULT_RANGE_G               8
#define MPU6000_ACCEL_DEFAULT_RATE                  1000
#define MPU6000_ACCEL_MAX_OUTPUT_RATE               280
#define MPU6000_ACCEL_DEFAULT_DRIVER_FILTER_FREQ    30

#define MPU6000_GYRO_DEFAULT_RANGE_G                8
#define MPU6000_GYRO_DEFAULT_RATE                   1000
/* rates need to be the same between accel and gyro */
#define MPU6000_GYRO_MAX_OUTPUT_RATE                MPU6000_ACCEL_MAX_OUTPUT_RATE
#define MPU6000_GYRO_DEFAULT_DRIVER_FILTER_FREQ     30

#define MPU6000_DEFAULT_ONCHIP_FILTER_FREQ          42

#define MPU6000_ONE_G                               9.80665f

unsigned _sample_rate = 200; // default MPU-6000 sampling rate (Hz)
uint8_t _product;
float _accel_range_scale;
float _accel_range_m_s2;

///////////////////////////////////////////////////////////////////////////////

uint16_t mpuCnt = 0;
boolean mpuDAV = false;
struct ADchannel mpu_temp;
int16_t vref_adj;

static callback_fptr_t callback = NULL;
static int initialised = 0;

static int set_accel_range(unsigned max_g_in)
{
	// workaround for bugged versions of MPU6k (rev C)

	switch (_product) {
		case MPU6000ES_REV_C4:
		case MPU6000ES_REV_C5:
		case MPU6000_REV_C4:
		case MPU6000_REV_C5:
			mpu_spi_write_checked_reg(MPUREG_ACCEL_CONFIG, 1 << 3);
			_accel_range_scale = (MPU6000_ONE_G / 4096.0f);
			_accel_range_m_s2 = 8.0f * MPU6000_ONE_G;
			return 0;
	}

	uint8_t afs_sel;
	float lsb_per_g;
	float max_accel_g;

	if (max_g_in > 8) { // 16g - AFS_SEL = 3
		afs_sel = 3;
		lsb_per_g = 2048;
		max_accel_g = 16;
	} else if (max_g_in > 4) { //  8g - AFS_SEL = 2
		afs_sel = 2;
		lsb_per_g = 4096;
		max_accel_g = 8;
	} else if (max_g_in > 2) { //  4g - AFS_SEL = 1
		afs_sel = 1;
		lsb_per_g = 8192;
		max_accel_g = 4;
	} else {                //  2g - AFS_SEL = 0
		afs_sel = 0;
		lsb_per_g = 16384;
		max_accel_g = 2;
	}

	mpu_spi_write_checked_reg(MPUREG_ACCEL_CONFIG, afs_sel << 3);
//	_accel_range_scale = (MPU6000_ONE_G / lsb_per_g);
//	_accel_range_m_s2 = max_accel_g * MPU6000_ONE_G;

	return 0;
}

/*
  set sample rate (approximate) - 1kHz to 5Hz, for both accel and gyro
*/
static void _set_sample_rate(unsigned desired_sample_rate_hz)
{
//	if (desired_sample_rate_hz == 0 ||
//			desired_sample_rate_hz == GYRO_SAMPLERATE_DEFAULT ||
//			desired_sample_rate_hz == ACCEL_SAMPLERATE_DEFAULT) {
//		desired_sample_rate_hz = MPU6000_GYRO_DEFAULT_RATE;
//	}

	uint8_t div = 1000 / desired_sample_rate_hz;
	if(div>200) div=200;
	if(div<1) div=1;
	mpu_spi_write_checked_reg(MPUREG_SMPLRT_DIV, div-1);
	_sample_rate = 1000 / div;
}

/*
  set the DLPF filter frequency. This affects both accel and gyro.
 */
static void _set_dlpf_filter(uint16_t frequency_hz)
{
	uint8_t filter;

//	   choose next highest filter frequency available
	if (frequency_hz == 0) {
		filter = BITS_DLPF_CFG_2100HZ_NOLPF;
	} else if (frequency_hz <= 5) {
		filter = BITS_DLPF_CFG_5HZ;
	} else if (frequency_hz <= 10) {
		filter = BITS_DLPF_CFG_10HZ;
	} else if (frequency_hz <= 20) {
		filter = BITS_DLPF_CFG_20HZ;
	} else if (frequency_hz <= 42) {
		filter = BITS_DLPF_CFG_42HZ;
	} else if (frequency_hz <= 98) {
		filter = BITS_DLPF_CFG_98HZ;
	} else if (frequency_hz <= 188) {
		filter = BITS_DLPF_CFG_188HZ;
	} else if (frequency_hz <= 256) {
		filter = BITS_DLPF_CFG_256HZ_NOLPF2;
	} else {
		filter = BITS_DLPF_CFG_2100HZ_NOLPF;
	}
	mpu_spi_write_checked_reg(MPUREG_CONFIG, filter);
}

static int MPU6000_reset(void)
{
	// mpu6000 frequently comes up in a bad state with all transfers being zero
	uint8_t tries = 5;
	while (--tries != 0) {
		mpu_spi_write(MPUREG_PWR_MGMT_1, BIT_H_RESET);
		HAL_Delay(10);

		// Wake up device and select GyroZ clock. Note that the
		// MPU6000 starts up in sleep mode, and it can take some time
		// for it to come out of sleep
		mpu_spi_write_checked_reg(MPUREG_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
		HAL_Delay(2);

		// Disable I2C bus (recommended on datasheet)
		mpu_spi_write_checked_reg(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);

		if (mpu_spi_read(MPUREG_PWR_MGMT_1) == MPU_CLK_SEL_PLLGYROZ) {
			break;
		}
		HAL_Delay(3);
	}
	if (mpu_spi_read(MPUREG_PWR_MGMT_1) != MPU_CLK_SEL_PLLGYROZ) {
		printf("MPUREG_PWR_MGMT_1 != MPU_CLK_SEL_PLLGYROZ:0x%02x\r\n", mpu_spi_read(MPUREG_PWR_MGMT_1));
		HAL_Delay(100);
		return -1;
	}
	HAL_Delay(2);
	_set_sample_rate(_sample_rate);
	HAL_Delay(2);

	// FS & DLPF   FS=2000 deg/s, DLPF = 20Hz (low pass filter)
	// was 90 Hz, but this ruins quality and does not improve the
	// system response
	_set_dlpf_filter(MPU6000_DEFAULT_ONCHIP_FILTER_FREQ);
	HAL_Delay(2);
	// Gyro scale 2000 deg/s ()
	mpu_spi_write_checked_reg(MPUREG_GYRO_CONFIG, BITS_FS_2000DPS);
	HAL_Delay(2);

	// correct gyro scale factors
	// scale to rad/s in SI units
	// 2000 deg/s = (2000/180)*PI = 34.906585 rad/s
	// scaling factor:
	// 1/(2^15)*(2000/180)*PI
//	_gyro_range_scale = (0.0174532 / 16.4);//1.0f / (32768.0f * (2000.0f / 180.0f) * M_PI_F);
//	_gyro_range_rad_s = (2000.0f / 180.0f) * M_PI_F;
	set_accel_range(8);
	HAL_Delay(2);

///	// INT CFG => Interrupt on Data Ready, totem-pole (push-pull) output
///	mpu_spi_write(MPUREG_INT_PIN_CFG, BIT_INT_LEVEL | BIT_INT_RD_CLEAR); // INT: Clear on any read
///	mpu_spi_write(MPUREG_INT_ENABLE, BIT_DATA_RDY_EN); // INT: Raw data ready

#define BIT_INT_ANYRD_2CLEAR (BIT_INT_LEVEL | BIT_INT_RD_CLEAR)

	// INT CFG => Interrupt on Data Ready
	mpu_spi_write_checked_reg(MPUREG_INT_ENABLE, BIT_DATA_RDY_EN);        // INT: Raw data ready
	HAL_Delay(2);
	mpu_spi_write_checked_reg(MPUREG_INT_PIN_CFG, BIT_INT_ANYRD_2CLEAR); // INT: Clear on any read
	HAL_Delay(2);

	// Oscillator set
	// mpu_spi_write(MPUREG_PWR_MGMT_1,MPU_CLK_SEL_PLLGYROZ);
	HAL_Delay(2);
	return 0;
}

int MPU6000_probe(void)
{
	uint8_t whoami;
	whoami = mpu_spi_read(MPUREG_WHOAMI);
	if (whoami != MPU_WHOAMI_6000) {
		printf("unexpected WHOAMI 0x%02x\r\n", whoami);
		return -1;
	}
	_product = mpu_spi_read(MPUREG_PRODUCT_ID); // verify product revision
	switch (_product) {
	case MPU6000ES_REV_C4:
	case MPU6000ES_REV_C5:
	case MPU6000_REV_C4:
	case MPU6000_REV_C5:
	case MPU6000ES_REV_D6:
	case MPU6000ES_REV_D7:
	case MPU6000ES_REV_D8:
	case MPU6000_REV_D6:
	case MPU6000_REV_D7:
	case MPU6000_REV_D8:
	case MPU6000_REV_D9:
	case MPU6000_REV_D10:
		printf("MPU-6000 ID 0x%02x\r\n", _product);
		return 0;
	}
	printf("unexpected ID 0x%02x\r\n", _product);
	return -2;
}

void process_MPU_data(uint16_t* mpu_data)
{
	mpuCnt++;
	mpuDAV = true;

	udb_xaccel.value = mpu_data[xaccel_MPU_channel];
	udb_yaccel.value = mpu_data[yaccel_MPU_channel];
	udb_zaccel.value = mpu_data[zaccel_MPU_channel];

	mpu_temp.value = mpu_data[temp_MPU_channel];

	udb_xrate.value = mpu_data[xrate_MPU_channel];
	udb_yrate.value = mpu_data[yrate_MPU_channel];
	udb_zrate.value = mpu_data[zrate_MPU_channel];

//{
//	static int i = 0;
//	if (i++ > 10) {
//		i = 0;
//		printf("%u %u %u\r\n", udb_xaccel.value, udb_yaccel.value, udb_zaccel.value);
//	}
//}

//#if (BOARD_TYPE != UDB4_BOARD && HEARTBEAT_HZ == 200)
	//  trigger synchronous processing of sensor data
	if (callback) callback();   // was directly calling heartbeat()
//#else
//#warning mpu6000: no callback mechanism defined
//#endif // (BOARD_TYPE != UDB4_BOARD && HEARTBEAT_HZ == 200)
}

void MPU6000_init(callback_fptr_t fptr)
{
	uint8_t tries = 5;

	if (initialised != 0)
	{
		DPRINT("WARNING: MPU6000 already initilised\r\n");
		return;
	}
	initialised = 1;

	callback = fptr;

	mpu_spi_init(NULL); // open SPI in low speed for register access
	while (MPU6000_probe())
	{
		if (tries++ > 5) {
			printf("ERROR: MPU600_probe failed");
			break; // TODO: we are really in a critical fail condition here and possibly should not continue
		}
		HAL_Delay(10);
	}
	tries = 0;
	while (MPU6000_reset())
	{
		if (tries++ > 5) {
			printf("ERROR: MPU600_reset failed");
			break; // TODO: we are really in a critical fail condition here and possibly should not continue
		}
	}
	mpu_spi_init(&process_MPU_data); // open the SPI in high speed for ongoing callback operation
}

// Used for debugging:
void MPU6000_print(uint8_t* mpu_data)
{
	printf("%06u axyz %06i %06i %06i gxyz %06i %06i %06i t %u\r\n",
	    mpuCnt,      mpu_data[0], mpu_data[1], mpu_data[2],
	    mpu_data[4], mpu_data[5], mpu_data[6], mpu_data[3]);
}

#endif // (BOARD_TYPE != UDB4_BOARD)
