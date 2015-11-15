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


#include "libUDB.h"
#include "oscillator.h"
#include "interrupt.h"
#include "heartbeat.h"
#include "ADchannel.h"
#include "mpu_spi.h"
#include "mpu6000.h"

#if (BOARD_TYPE != UDB4_BOARD)

///////////////////////////////////////////////////////////////////////////////

#define MPU6000_ONE_G                               9.80665f

unsigned _sample_rate = 200; // default MPU-6000 sampling rate (Hz)
uint8_t _product;
float _accel_range_scale;
float _accel_range_m_s2;

///////////////////////////////////////////////////////////////////////////////

boolean mpuDAV = false;
struct ADchannel mpu_temp;
int16_t vref_adj;

static callback_fptr_t callback = NULL;

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
	mpu_spi_write(MPUREG_PWR_MGMT_1, BIT_H_RESET);

	// 10msec delay seems to be needed for AUAV3 (MW's prototype)
	delay_ms(10);

	// Wake up device and select GyroZ clock (better performance)
	mpu_spi_write(MPUREG_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);

	// Disable I2C bus (recommended on datasheet)
	mpu_spi_write(MPUREG_USER_CTRL, BIT_I2C_IF_DIS);

	// SAMPLE RATE
	mpu_spi_write(MPUREG_SMPLRT_DIV, 4); // Sample rate = 200Hz  Fsample= 1Khz/(N+1) = 200Hz

	// scaling & DLPF
	mpu_spi_write(MPUREG_CONFIG, BITS_DLPF_CFG_42HZ);

//	mpu_spi_write(MPUREG_GYRO_CONFIG, BITS_FS_2000DPS);  // Gyro scale 2000º/s
	mpu_spi_write(MPUREG_GYRO_CONFIG, BITS_FS_500DPS); // Gyro scale 500º/s

#if (ACCEL_RANGE == 2)
	mpu_spi_write(MPUREG_ACCEL_CONFIG, BITS_FS_2G); // Accel scele 2g, g = 8192
#elif (ACCEL_RANGE == 4)
	mpu_spi_write(MPUREG_ACCEL_CONFIG, BITS_FS_4G); // Accel scale g = 4096
#elif (ACCEL_RANGE == 8)
	mpu_spi_write(MPUREG_ACCEL_CONFIG, BITS_FS_8G); // Accel scale g = 2048
#else
#error "Invalid ACCEL_RANGE"
#endif

#if 0
	// Legacy from Mark Whitehorn's testing, we might need it some day.
	// SAMPLE RATE
	mpu_spi_write(MPUREG_SMPLRT_DIV, 7); // Sample rate = 1KHz  Fsample= 8Khz/(N+1)

	// no DLPF, gyro sample rate 8KHz
	mpu_spi_write(MPUREG_CONFIG, BITS_DLPF_CFG_256HZ_NOLPF2);

	mpu_spi_write(MPUREG_GYRO_CONFIG, BITS_FS_500DPS); // Gyro scale 500º/s

//	mpu_spi_write(MPUREG_ACCEL_CONFIG, BITS_FS_2G); // Accel scale 2g, g = 16384
	mpu_spi_write(MPUREG_ACCEL_CONFIG, BITS_FS_4G); // Accel scale g = 8192
//	mpu_spi_write(MPUREG_ACCEL_CONFIG, BITS_FS_8G); // Accel scale g = 4096
#endif

	// INT CFG => Interrupt on Data Ready, totem-pole (push-pull) output
	mpu_spi_write(MPUREG_INT_PIN_CFG, BIT_INT_LEVEL | BIT_INT_RD_CLEAR); // INT: Clear on any read
	mpu_spi_write(MPUREG_INT_ENABLE, BIT_DATA_RDY_EN); // INT: Raw data ready
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

#if (BOARD_TYPE != UDB4_BOARD && HEARTBEAT_HZ == 200)
	//  trigger synchronous processing of sensor data
	if (callback) callback();   // was directly calling heartbeat()
#else
#warning mpu6000: no callback mechanism defined
#endif // (BOARD_TYPE != UDB4_BOARD && HEARTBEAT_HZ == 200)
}

void MPU6000_init(callback_fptr_t fptr)
{
	callback = fptr;

	mpu_spi_init(NULL); // open SPI in low speed for register access

	// need at least 60 msec delay here
	delay_ms(60);

	MPU6000_probe();
	while (MPU6000_reset()) { }
	mpu_spi_init(&process_MPU_data); // open the SPI in high speed for ongoing callback operation
}

#endif // (BOARD_TYPE != UDB4_BOARD)
