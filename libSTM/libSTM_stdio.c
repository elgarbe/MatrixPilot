//
//  libSTM_stdio.c
//  MatrixPilot
//
//  Created by Robert Dickenson on 18/10/2015.
//  Copyright (c) 2014 MatrixPilot. All rights reserved.
//

#include "../libUDB/libUDB.h"
//#include "libUDB.h"
#include "../libUDB/uart.h"
#include "usart.h"

#include "stm32f4xx_hal.h"
//#include "cmsis_os.h"

#include <stdio.h>


uint8_t buffered_char = 0;
uint8_t buffered_full = 0;


#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE      //  __io_putchar()
{
// CONSOLE_UART
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&UART_CON, (uint8_t *)&ch, 1, 0xFFFF);
//  HAL_UART_Transmit(&UART_MAV, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
/*
int __io_getchar(void)
{
	return GetChar();
}
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  // we want to signal the process responsible for this uart
}

//void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	switch (huart->ErrorCode)
	{
	case HAL_UART_ERROR_NONE://         ((uint32_t)0x00000000)   // !< No error
		break;
	case HAL_UART_ERROR_PE://           ((uint32_t)0x00000001)   // !< Parity error
		break;
	case HAL_UART_ERROR_NE://           ((uint32_t)0x00000002)   // !< Noise error
		break;
	case HAL_UART_ERROR_FE://           ((uint32_t)0x00000004)   // !< Frame error
		break;
	case HAL_UART_ERROR_ORE://          ((uint32_t)0x00000008)   // !< Overrun error
		break;
	case HAL_UART_ERROR_DMA://          ((uint32_t)0x00000010)   // !< DMA transfer error
		break;
	default:
		break;
	}
	huart->ErrorCode = HAL_UART_ERROR_NONE;


	if (huart->ErrorCode != HAL_UART_ERROR_NONE)
	{
		huart->ErrorCode = HAL_UART_ERROR_NONE;
	}

}

char IsPressed(void)
{
	HAL_StatusTypeDef status;
	uint8_t Data[2];

// HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)

	if (buffered_full == 1)
	{
		return 1;
	}

	status = HAL_UART_Receive(&UART_CON, &Data[0], 1, 0);
	if (status == HAL_OK)
	{
		buffered_char = Data[0];
		buffered_full = 1;
		return 1;
	}

	return 0;
}

void ClrError(void)
{
//	if (U##x##STAbits.OERR) U##x##STAbits.OERR = 0;
}

char GetChar(void)
{
	HAL_StatusTypeDef status;
	uint8_t Data[2];
	char Temp;
//	while (!IsPressed());

	if (buffered_full == 1)
	{
		Temp = buffered_char;
		buffered_full = 0;
	}
	else
	{
		status = HAL_UART_Receive(&UART_CON, &Data[0], 1, 0);
		if (status == HAL_OK)
		{
			Temp = Data[0];
		}
	}
//	ClrError();
	return Temp;
}

/*
void UART_PutChar(UART_HandleTypeDef *huart, char ch);
void UART_PutChar(UART_HandleTypeDef *huart, char ch)
{
	if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TXE, RESET, 0xffff) == HAL_OK)
	{
		huart->Instance->DR = ch;
	}
}
static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status, uint32_t Timeout);
//HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
 */
void PutChar(char ch)
{
//	HAL_StatusTypeDef status;

//	status = HAL_UART_Transmit(&UART_CON, (uint8_t *)&ch, 1, 0xFFFF);
//	if (status == HAL_OK)
//	{
//	}

#if 1
	while(__HAL_UART_GET_FLAG(&UART_CON, UART_FLAG_TXE) == RESET)
	{
	}
	UART_CON.Instance->DR = ch;
//	UART_PutChar(&UART_CON, ch);
#else
	while(__HAL_UART_GET_FLAG(&UART_MAV, UART_FLAG_TXE) == RESET)
	{
	}
	UART_MAV.Instance->DR = ch;
#endif
}
