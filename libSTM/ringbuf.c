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


//#include "../libUDB/libUDB.h"
#include <string.h>
#include "ringbuf.h"

static unsigned int modulo_inc(const unsigned int value, const unsigned int modulus)
{
    unsigned int my_value = value + 1;
    if (my_value >= modulus)
    {
      my_value  = 0;
    }
    return (my_value);
}

static unsigned int modulo_dec(const unsigned int value, const unsigned int modulus)
{
    unsigned int my_value = (0 == value) ? (modulus - 1) : (value - 1);
    return (my_value);
}

void ringbuf_init(ringbuf_t *_this)
{
    /*****
      The following clears:
        -> buf
        -> head
        -> tail
        -> count
      and sets head = tail
    ***/
    memset(_this, 0, sizeof(*_this));
}

void ringbuf_put(ringbuf_t *_this, const unsigned char c)
{
    if (_this->count < RBUF_SIZE)
    {
      _this->buf[_this->head] = c;
      _this->head = modulo_inc(_this->head, RBUF_SIZE);
      ++_this->count;
    }
}

int ringbuf_get(ringbuf_t *_this)
{
    int c;
    if (_this->count>0)
    {
      c = _this->buf[_this->tail];
      _this->tail = modulo_inc(_this->tail, RBUF_SIZE);
      --_this->count;
    }
    else
    {
      c = -1;
    }
    return (c);
}

void ringbuf_flush(ringbuf_t *_this, const int clearBuffer)
{
  _this->count = 0;
  _this->head = 0;
  _this->tail = 0;
  if (clearBuffer)
  {
    memset(_this->buf, 0, sizeof(_this->buf));
  }
}

int ringbuf_full(ringbuf_t *_this)
{
    return (_this->count >= RBUF_SIZE);
}

int ringbuf_empty(ringbuf_t *_this)
{
    return (0 == _this->count);
}

#include  <stdio.h>

int ringbuf_test(int argc, char* argv[])
{
  int i;
  ringbuf_t my_ringBuffer;
  int my_status;
  ringbuf_init(&my_ringBuffer);

  printf("\nfifo empty status = %d", ringbuf_empty(&my_ringBuffer));

  puts ("\nThe following is the FIFO FULL status");
  for (i = 0; i < RBUF_SIZE; i++)
  {
    ringbuf_put(&my_ringBuffer, (unsigned char)i);
    my_status = ringbuf_full(&my_ringBuffer);
    if (0 == (i % 16))
    {
      putchar ('\n');
    }
    printf("%2d ", my_status);
  }

  printf("\nfifo empty status = %d", ringbuf_empty(&my_ringBuffer));
  puts("\nThe following is the contents of the FIFO");
  for (i = 0; i < RBUF_SIZE; i++)
  {
    int my_datum = ringbuf_get(&my_ringBuffer);
    if (0 == (i % 16))
    {
      putchar ('\n');
    }
    printf("%02X ", my_datum);
  }
  printf("\nfifo empty status = %d", ringbuf_empty(&my_ringBuffer));

  putchar ('\n');
  return 0;
}
