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

#if (USE_OSD == OSD_NATIVE)

void osd_reset(void)
{
}

void osd_init(void)
{
	osd_reset();
}

void osd_spi_write_location(int16_t loc)
{
}

void osd_spi_write_string(const uint8_t* str)
{
}

void osd_spi_write_vertical_string_at_location(int16_t loc, const uint8_t* str)
{
}

void osd_spi_erase_chars(uint8_t n)
{
}

void osd_spi_write_number(int32_t val, int8_t num_digits, int8_t decimal_places, int8_t num_flags, int8_t header, int8_t footer)
{
}

#else

void osd_init(void) {}

#endif // USE_OSD
