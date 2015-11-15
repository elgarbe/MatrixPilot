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


#ifndef MPU_SPI_H
#define MPU_SPI_H


// define which SPI port the MPU is using by defining MPU_SPI to be 1 or 2
// on UDB4, either SPI port can be used to connect MPU.
// on UDB5, SPI2 connects to MPU, SPI1 is for off board.
// on AUAV3, SPI1 connects to MPU, SPI2 is for off board.
// On UDB4/5:
// SPI1 interface uses INT1, RA12 for MPU interrupt
// SPI2 interface uses INT3, RA14 for MPU interrupt
// On AUAV3:
// SPI1 interface uses INT1, RG12 for MPU interrupt


#if (BOARD_TYPE == UDB4_BOARD)
#define MPU_SPI 1
#define _TRISMPUINT _TRISA12

#elif (BOARD_TYPE == UDB5_BOARD)
#define MPU_SPI 2
#define _TRISMPUINT _TRISA14

#elif (BOARD_TYPE == AUAV2_BOARD)
#define MPU_SPI 1
#define _TRISMPUINT _TRISE8

#elif (BOARD_TYPE == AUAV3_BOARD)
#define MPU_SPI 1
#define _TRISMPUINT _TRISG12

#elif (BOARD_TYPE == AUAV4_BOARD)
#define MPU_SPI 1
#define _TRISMPUINT _TRISG12

#elif (BOARD_TYPE == PX4_BOARD)
#define MPU_SPI 2   //I'm using SPI2. But I don't think that I will use MPU_SPI definition
#else
#error "Only BOARD_TYPEs UDB5, UDB4 and AUAV3 supported"
#endif


void mpu_spi_init(void (*call_back)(uint16_t*));
uint8_t mpu_spi_read(uint8_t addr);
void mpu_spi_write(uint8_t addr, uint8_t data);
void mpu_spi_write_checked_reg(uint8_t addr, uint8_t data);

#endif // MPU_SPI_H
