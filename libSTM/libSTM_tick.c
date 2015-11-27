//
//  libSTM_tick.c
//  MatrixPilot
//
//  Created by Robert Dickenson on 08/11/2015.
//  Copyright (c) 2014 MatrixPilot. All rights reserved.
//

#include "../libUDB/libUDB.h"
#include "../libUDB/heartbeat.h"

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "tim.h"

uint8_t udb_cpu_load(void)
{
	return 5; // sounds reasonable for a fake cpu%
}

void cpu_load_calc(void)
{
}

int setjmp(void)
{
	return 0;
}

void __delay32(unsigned long cycles)
{
    /* Insert delay in ms */
    HAL_Delay(cycles);
}

void vApplicationTickHook(void)
{
}


//! Test if in interrupt mode
inline int isInterrupt(void)
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0 ;
}

static __IO uint32_t uwTick;

/**
  * @brief This function is called to increment  a global variable "uwTick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each 1ms
  *       in Systick ISR.
 * @note This function is declared as __weak to be overwritten in case of other
  *      implementations in user file.
  * @retval None
  */
void HAL_IncTick(void)
{
  uwTick++;
}

/**
  * @brief Provides a tick value in millisecond.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
uint32_t HAL_GetTick(void)
{
	if (isInterrupt())
	{
//		uwTick++;
	}
	return uwTick;
}
