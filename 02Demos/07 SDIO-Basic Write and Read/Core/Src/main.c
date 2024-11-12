/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "quadspi.h"
#include "rtc.h"
#include "sdmmc.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include <string.h>


/******************************************************************************/

#define	NumOf_Blocks	64
#define 	Test_BlockSize  ((BLOCKSIZE * NumOf_Blocks) >> 2)	 //�������ݴ�С,SD���СΪ512�ֽڣ���Ϊ��32λ�����飬�����������4
#define	Test_Addr 	 0x00


int32_t SD_Status ; 		 //SD������־λ
uint32_t SD_WriteBuffer[Test_BlockSize];	//	д��������
uint32_t SD_ReadBuffer[Test_BlockSize];	//	����������


// ������SD������
//	���ܣ�SD�����д����
//	����ֵ��BSP_ERROR_NONE - ��д���Գɹ�
//
uint8_t SD_Test(void)
{
	uint32_t i = 0;	
	uint32_t ExecutionTime_Begin;		// ��ʼʱ��
	uint32_t ExecutionTime_End;		// ����ʱ��
	uint32_t ExecutionTime;				// ִ��ʱ��	
	float    ExecutionSpeed;			// ִ���ٶ�

// ���� >>>>>>>    
		
	ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms
	SD_Status = BSP_SD_Erase(SD_Instance,Test_Addr, NumOf_Blocks);
	while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//�ȴ�ͨ�Ž���	
	ExecutionTime_End		= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms
	
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin; // �������ʱ�䣬��λms
	
	if( SD_Status == BSP_ERROR_NONE )
	{
		printf ("\r\n �����ɹ�, ��������ʱ��: %d ms\r\n",ExecutionTime);		
	}
	else
	{
		printf ("\r\n ����ʧ��!!!!!  �������:%d\r\n",SD_Status);
		while (1);
	}		
	
// д�� >>>>>>>    
	for(i=0;i<Test_BlockSize;i++)	//��Ҫд��SD��������д������
	{
		SD_WriteBuffer[i] = i;
	}
	
	ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms	
	SD_Status = BSP_SD_WriteBlocks(SD_Instance,SD_WriteBuffer, Test_Addr, NumOf_Blocks);	//��д��	
	while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//�ȴ�ͨ�Ž���	
	ExecutionTime_End		= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms
	
	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 		// �������ʱ�䣬��λms
	ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime /1024 ; // ����д���ٶȣ���λ MB/S	
	if( SD_Status == BSP_ERROR_NONE )
	{
		printf ("\r\nд��ɹ�,���ݴ�С��%d KB, ��ʱ: %d ms, д���ٶȣ�%.2f MB/s\r\n",BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
	}
	else
	{
		printf ("\r\nд�����!!!!!  �������:%d\r\n",SD_Status);
		while (1);
	}		
	
	
// ��ȡ >>>>>>>    
	ExecutionTime_Begin 	= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms		
	SD_Status = BSP_SD_ReadBlocks(SD_Instance,SD_ReadBuffer, Test_Addr, NumOf_Blocks);	//���ȡ
	while(BSP_SD_GetCardState(SD_Instance) != SD_TRANSFER_OK);	//�ȴ�ͨ�Ž���	
	ExecutionTime_End		= HAL_GetTick();	// ��ȡ systick ��ǰʱ�䣬��λms

	ExecutionTime  = ExecutionTime_End - ExecutionTime_Begin; 						// �������ʱ�䣬��λms
	ExecutionSpeed = (float)BLOCKSIZE * NumOf_Blocks / ExecutionTime / 1024 ; 	// �����ȡ�ٶȣ���λ MB/S 
	
	if( SD_Status == BSP_ERROR_NONE )
	{
		printf ("\r\n��ȡ�ɹ�,���ݴ�С��%d KB, ��ʱ: %d ms, ��ȡ�ٶȣ�%.2f MB/s \r\n",BLOCKSIZE * NumOf_Blocks/1024,ExecutionTime,ExecutionSpeed);		
	}
	else
	{
		printf ("\r\n��ȡ����!!!!!  �������:%d\r\n",SD_Status);
		while (1);
	}		

// У�� >>>>>>>   
	for(i=0;i<Test_BlockSize;i++)	//��֤�����������Ƿ����д�������
	{
		if( SD_WriteBuffer[i] != SD_ReadBuffer[i] )	//������ݲ���ȣ��򷵻�0	
		{
			printf ("\r\n����У��ʧ��!!!!!\r\n");	
			while(1);
		}
	}		
	printf ("\r\nУ��ͨ��!!!!!SD����������\r\n");		
	return BSP_ERROR_NONE; 	//������ȷ����д����ͨ��
}


/******************************************************************************/

void SystemClock_Config(void);


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  SCB_EnableICache();

  SCB_EnableDCache();

  HAL_Init();

  SystemClock_Config();


  MX_GPIO_Init();
	MX_USART1_UART_Init();
//  MX_DCMI_Init();
//  MX_RTC_Init();
//  MX_SDMMC1_SD_Init();
//  MX_USB_DEVICE_Init();
	
	SD_Status = BSP_SD_Init(SD_Instance);	//SD����ʼ��
	
	if( SD_Status == BSP_ERROR_NONE )	//����Ƿ��ʼ���ɹ�
	{		
		printf("SD����ʼ���ɹ� \r\n");	
	}
	else
		printf("��ⲻ��SD����ERROR: %d\r\n",SD_Status);	

	SD_Test();
	printf("SD����д���Գɹ� \r\n");		
  while (1)
  {	
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		HAL_Delay(1000);	// ��ʱ	
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		HAL_Delay(1000);	// ��ʱ	
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
	
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_SDMMC
															|RCC_PERIPHCLK_CKPER;
  
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;  
  
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
