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
#include "mpu_spi.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

extern SPI_HandleTypeDef hspi2;

#ifdef MPU_SPI_DEBUG
volatile double tempC;
volatile double X_accel, Y_accel, Z_accel;
volatile double X_gyro, Y_gyro, Z_gyro;
#endif // MPU_SPI_DEBUG
uint8_t mpu_dma[15]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static void no_call_back(uint16_t* mpu_data) {}
static void (*mpu_call_back)(uint16_t*) = &no_call_back;

static void mpu_spi_error(int err)
{
	DPRINT("ERROR: mpu_spi_error: ");
	switch (err) {
		case HAL_ERROR:
			DPRINT("HAL_ERROR");
			break;
		case HAL_TIMEOUT:
			DPRINT("HAL_TIMEOUT");
			break;
		case HAL_BUSY:
			DPRINT("HAL_BUSY");
			break;
		case HAL_OK:
			DPRINT("HAL_OK\r\n");
			break;
		default:
			break;
	}
	DPRINT("\r\n");
}

void mpu_spi_init(void (*call_back)(uint16_t*))
{
	if (call_back)
	{
		mpu_call_back = call_back;
		// Bump the SPI clock up towards 10.5 MHz for ongoing sensor and interrupt register reads
		// 20 MHz is the maximum specified for the MPU-6000
		//
		//TODO: Check if hspi is initialized ok and just need to change BaudRatePrescaler

//		hspi2.Instance = SPI2;
//		hspi2.Init.Mode = SPI_MODE_MASTER;
//		hspi2.Init.Direction = SPI_DIRECTION_2LINES;
//		hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
//		hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
//		hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
//		hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT;
//		hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
//		hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
//		hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
//		hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
//		HAL_SPI_Init(&hspi2);

		HAL_NVIC_EnableIRQ(EXTI0_IRQn); //NOTE: Should I enable INT here? It's already enabled, but maybe I should move here.
		// RobD: on the other hand, the driver does not need to know what priority the system is running it at.
		//  perhaps an assert here on the assumption that it is already enabled?
	}
	else
	{
		mpu_call_back = &no_call_back;
// MPU-6000 maximum SPI clock is specified as 1 MHz for all registers
//    however the datasheet states that the sensor and interrupt registers
//    may be read using an SPI clock of 20 Mhz
// As these register accesses are one time only during initial setup lets be
//    conservative and only run the SPI bus at half the maximum specified speed
		HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	}
}

#define DIR_READ    0x80
#define DIR_WRITE   0x00

uint8_t mpu_spi_read(uint8_t reg)
{
	HAL_StatusTypeDef err;
	uint8_t data[2];

	data[0] = reg | DIR_READ;
	data[1] = 0;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	err = HAL_SPI_Receive(&hspi2, data, 2, 10); // transfer two bytes in 8 bit mode
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	if (err != HAL_OK){
		mpu_spi_error(err);
//		while(1);
	}
	// this delay is necessary; it appears that SS must be de-asserted for one or
	// more SPI clock cycles between writes.
	// NOTE: From original MP code.
	// TODO: It's 1ms delay, it would be 2us. Check on MPU6000 data sheet this issue
	HAL_Delay(1);
	return data[1];
}

void mpu_spi_write(uint8_t reg, uint8_t value)
{
	HAL_StatusTypeDef err;
	uint8_t data[2];

	data[0] = reg | DIR_WRITE;
	data[1] = value;
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	err = HAL_SPI_Transmit(&hspi2, data, 2, 10);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	if(err != HAL_OK){
		//TODO: Do something with possible error here
		mpu_spi_error(err);
//		while(1);
	}
	// this delay is necessary; it appears that SS must be deasserted for one or
	// more SPI clock cycles between writes
	// NOTE: From original MP code.
	// TODO: It's 1 mseg delay, it would be 2useg. Check on MPU6000 data sheet this issue
	HAL_Delay(1);
}

void mpu_spi_write_checked_reg(uint8_t reg, uint8_t value)
{
	mpu_spi_write(reg, value);
}

// DMA Receive Complete ISR Callback
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // CS goes HIGH
	// Process MPU register data
#ifdef MPU_SPI_DEBUG
//	int16_t tmp = 0;
	X_accel = (int16_t)((mpu_dma[1]<<8) | mpu_dma[2])/16384.0;
	Y_accel = (int16_t)((mpu_dma[3]<<8) | mpu_dma[4])/16384.0;
	Z_accel = (int16_t)((mpu_dma[5]<<8) | mpu_dma[6])/16384.0;
//    tmp=(mpu_dma[7]<<8) | mpu_dma[8];
//    tempC = (double)(tmp)/340.0 + 36.53;
	tempC = (double)(((int16_t)(mpu_dma[7]<<8) | mpu_dma[8]))/340.0 + 36.53;
	X_gyro = (int16_t)((mpu_dma[9]<<8)  | mpu_dma[10])/16384.0;
	Y_gyro = (int16_t)((mpu_dma[11]<<8) | mpu_dma[12])/16384.0;
	Z_gyro = (int16_t)((mpu_dma[13]<<8) | mpu_dma[14])/16384.0;
#endif // MPU_SPI_DEBUG
	mpu_dma[0] = mpu_dma[2];    //X_accel
	mpu_dma[2] = mpu_dma[4];    //Y_accel
	mpu_dma[4] = mpu_dma[6];    //Z_accel
	mpu_dma[6] = mpu_dma[8];    //Temperature
	mpu_dma[8] = mpu_dma[10];   //X_gyro
	mpu_dma[10] = mpu_dma[12];  //Y_gyro
	mpu_dma[12] = mpu_dma[14];  //Z_gyro
	mpu_call_back((uint16_t*)&mpu_dma); //	process_MPU_data(mpu_dma);
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_PIN_0 == GPIO_Pin) // MPU-6000
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //CS goes LOW
#define MPUREG_ACCEL_XOUT_H         0x3B
		mpu_dma[0] = MPUREG_ACCEL_XOUT_H | 0x80;
		HAL_SPI_Receive_DMA(&hspi2, mpu_dma, sizeof(mpu_dma));
	}
	if (GPIO_PIN_13 == GPIO_Pin) // Push button
	{
	}
}
