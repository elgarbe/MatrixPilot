/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "libUDB.h"
#include "heartbeat.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Variables -----------------------------------------------------------------*/
osThreadId defaultTaskHandle;
osThreadId TaskGPSHandle;
osThreadId TaskIMUHandle;
osSemaphoreId osSemaphoreIMUHandle;
osSemaphoreId osSemaphoreGPSHandle;

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Function prototypes -------------------------------------------------------*/
void StartDefaultTask(void const * argument);
void StartTaskGPS(void const * argument);
void StartTaskIMU(void const * argument);

extern void MX_FATFS_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN FunctionPrototypes */
int matrixpilot_loop(void);

/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

/* USER CODE BEGIN 2 */

void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */

//	matrixpilot_loop();
}

/* USER CODE END 2 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/* USER CODE BEGIN 5 */
void vApplicationMallocFailedHook(void)
{
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
	printf("vApplicationMallocFailedHook()\r\n");
//	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/* USER CODE END 5 */

/* Init FreeRTOS */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of osSemaphoreIMU */
  osSemaphoreDef(osSemaphoreIMU);
  osSemaphoreIMUHandle = osSemaphoreCreate(osSemaphore(osSemaphoreIMU), 1);

  /* definition and creation of osSemaphoreGPS */
  osSemaphoreDef(osSemaphoreGPS);
  osSemaphoreGPSHandle = osSemaphoreCreate(osSemaphore(osSemaphoreGPS), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 1024);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of TaskGPS */
  osThreadDef(TaskGPS, StartTaskGPS, osPriorityAboveNormal, 0, 1024);
  TaskGPSHandle = osThreadCreate(osThread(TaskGPS), NULL);

  /* definition and creation of TaskIMU */
  osThreadDef(TaskIMU, StartTaskIMU, osPriorityHigh, 0, 1024);
  TaskIMUHandle = osThreadCreate(osThread(TaskIMU), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
}

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{
  /* init code for FATFS */
  MX_FATFS_Init();

  /* USER CODE BEGIN StartDefaultTask */
	DPRINT("StartDefaultTask: %p\r\n", argument);
void _StartDefaultTask(void const * argument);
	_StartDefaultTask(argument);
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN 6 */
void RunTaskGPS(void);
void RunTaskIMU(void);

/* USER CODE END 6 */

/* StartTaskGPS function */
void StartTaskGPS(void const * argument)
{
  /* USER CODE BEGIN StartTaskGPS */
	(void)argument;
	DPRINT("StartTaskGPS\r\n");
	for(;;)
	{
		osSemaphoreWait(osSemaphoreGPSHandle, osWaitForever);
		RunTaskGPS();
//		udb_led_toggle(LED_RED);
	}
  /* USER CODE END StartTaskGPS */
}

/* StartTaskIMU function */
void StartTaskIMU(void const * argument)
{
  /* USER CODE BEGIN StartTaskIMU */
	led_on(LED_ORANGE);
	DPRINT("StartTaskIMU\r\n");
	for(;;)
	{
		static int i = 0;

//		osSemaphoreWait(osSemaphoreIMUHandle, osWaitForever);
		if (osSemaphoreWait(osSemaphoreIMUHandle, 500) == osOK)
		{
//			pulse(); // now called from RunTaskIMU below:
			RunTaskIMU();
		}
		else
		{
			// timeout
		}
		if (i++ > 0)
		{
			i = 0;
//			DPRINT("#");
		}
		udb_led_toggle(LED_ORANGE);
//		udb_led_toggle(LED_BLUE);  // blue led not working?
	}
  /* USER CODE END StartTaskIMU */
}

/* USER CODE BEGIN Application */

void TriggerTaskGPS(void)
{
	osSemaphoreRelease(osSemaphoreGPSHandle);
}

void TriggerTaskIMU(void)
{
	osSemaphoreRelease(osSemaphoreIMUHandle);
}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
