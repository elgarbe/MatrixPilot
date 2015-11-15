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
#include "osd_config.h"
#include "../libUDB/osd.h"

#if (USE_OSD == OSD_NATIVE && USE_OSD_SPI == 1)

#warning "Using OSD_NATIVE in SPI hardware mode"


//#include "stm32f4xx_hal.h"
//#include "cmsis_os.h"
//#include "dma.h"
//#include "spi.h"
//#include "gpio.h"


void osd_spi_init(void)
{
}

void osd_spi_write_byte(int8_t byte)
{
}

void osd_spi_write(int8_t addr, int8_t byte)
{
}

#else

#endif // (USE_OSD == OSD_NATIVE && USE_OSD_SPI == 1)
