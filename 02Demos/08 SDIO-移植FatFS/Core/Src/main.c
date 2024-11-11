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
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

/*****************************************************************************************/

FATFS 	SD_FatFs; 		// �ļ�ϵͳ����
FRESULT 	MyFile_Res;    // ������� 
char SDPath[4];			// SD���߼�����·��

/*****************************************************************************************/

void FatFs_Check(void);				// �ж�FatFs�Ƿ���سɹ�����û�д���FatFs���ʽ��SD��
void FatFs_GetVolume(void);		// �����豸����
uint8_t  FatFs_FileTest(void);	// �ļ�������д�����
void SystemClock_Config(void);	// ϵͳʱ������

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
	
	FatFs_Check();			//�ж�FatFs�Ƿ���سɹ�����û�д���FatFs���ʽ��SD��
	FatFs_GetVolume();	// �����豸����
	FatFs_FileTest();		//�ļ�������д�����
  while (1)
  {	
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
			HAL_Delay(1000);	// ��ʱ	
		  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
			HAL_Delay(1000);	// ��ʱ	
  }
}
/**************************************************************************************************/

//	������FatFs_Check
//	���ܣ�����FatFs�ļ�ϵͳ�Ĺ���
//
void FatFs_Check(void)	//�ж�FatFs�Ƿ���سɹ�����û�д���FatFs���ʽ��SD��
{
	BYTE work[FF_MAX_SS]; 
	
	FATFS_LinkDriver(&SD_Driver, SDPath);		// ��ʼ������
	MyFile_Res = f_mount(&SD_FatFs,"0:",1);	//	����SD��
	
	if (MyFile_Res == FR_OK)	//�ж��Ƿ���سɹ�
	{
		printf("\r\nSD�ļ�ϵͳ���سɹ�\r\n");
	}
	else		
	{
		printf("SD����δ�����ļ�ϵͳ��������ʽ��\r\n");
		
		MyFile_Res = f_mkfs("0:",FM_FAT32,0,work,sizeof work);		//��ʽ��SD����FAT32����Ĭ�ϴ�С16K
		
		if (MyFile_Res == FR_OK)		//�ж��Ƿ��ʽ���ɹ�
			printf("SD����ʽ���ɹ���\r\n");
		else
			printf("��ʽ��ʧ�ܣ���������SD����\r\n");
	}
}
//	������FatFs_GetVolume
//	���ܣ������豸��������������������ʣ������
//
void FatFs_GetVolume(void)	// �����豸����
{
	FATFS *fs;		//����ṹ��ָ��
	uint32_t SD_CardCapacity = 0;		//SD����������
	uint32_t SD_FreeCapacity = 0;		//SD����������
	DWORD fre_clust, fre_sect, tot_sect; 	//���дأ���������������������

	f_getfree("0:",&fre_clust,&fs);			//��ȡSD��ʣ��Ĵ�

	tot_sect = (fs->n_fatent-2) * fs->csize;	//���������� = �ܵĴ� * ÿ���ذ�����������
	fre_sect = fre_clust * fs->csize;			//����ʣ��Ŀ���������	   

	SD_CardCapacity = tot_sect / 2048 ;	// SD�������� = �������� * 512( ÿ�������ֽ��� ) / 1048576(�����MB)
	SD_FreeCapacity = fre_sect / 2048 ;	//����ʣ�����������λΪM
	printf("-------------------��ȡ�豸������Ϣ-----------------\r\n");		
	printf("SD������%dMB\r\n",SD_CardCapacity);	
	printf("SDʣ�ࣺ%dMB\r\n",SD_FreeCapacity);
}

//	������FatFs_FileTest
//	���ܣ������ļ�д��Ͷ�ȡ����
//
uint8_t  FatFs_FileTest(void)	//�ļ�������д�����
{
	uint8_t i = 0;
	uint16_t BufferSize = 0;	
	FIL	MyFile;			// �ļ�����
	UINT 	MyFile_Num;		//	���ݳ���
	BYTE 	MyFile_WriteBuffer[] = "STM32 SD�� �ļ�ϵͳ����";	//Ҫд�������
	BYTE 	MyFile_ReadBuffer[1024];	//Ҫ����������
	
	printf("-------------FatFs �ļ�������д�����---------------\r\n");
	
	MyFile_Res = f_open(&MyFile,"0:FatFs Test.txt",FA_CREATE_ALWAYS | FA_WRITE);	//���ļ������������򴴽����ļ�
	if(MyFile_Res == FR_OK)
	{
		printf("�ļ���/�����ɹ���׼��д������...\r\n");
		
		MyFile_Res = f_write(&MyFile,MyFile_WriteBuffer,sizeof(MyFile_WriteBuffer),&MyFile_Num);	//���ļ�д������
		if (MyFile_Res == FR_OK)	
		{
			printf("д��ɹ���д������Ϊ��\r\n");
			printf("%s\r\n",MyFile_WriteBuffer);
		}
		else
		{
			printf("�ļ�д��ʧ�ܣ�����SD�������¸�ʽ��!\r\n");
			f_close(&MyFile);	  //�ر��ļ�	
			return ERROR;			
		}
		f_close(&MyFile);	  //�ر��ļ�			
	}
	else
	{
		printf("�޷���/�����ļ�������SD�������¸�ʽ��!\r\n");
		f_close(&MyFile);	  //�ر��ļ�	
		return ERROR;		
	}
	
	printf("-------------FatFs �ļ���ȡ����---------------\r\n");	
	
	BufferSize = sizeof(MyFile_WriteBuffer)/sizeof(BYTE);									// ����д������ݳ���
	MyFile_Res = f_open(&MyFile,"0:FatFs Test.txt",FA_OPEN_EXISTING | FA_READ);	//���ļ������������򴴽����ļ�
	MyFile_Res = f_read(&MyFile,MyFile_ReadBuffer,BufferSize,&MyFile_Num);			// ��ȡ�ļ�
	if(MyFile_Res == FR_OK)
	{
		printf("�ļ���ȡ�ɹ�������У������...\r\n");
		
		for(i=0;i<BufferSize;i++)
		{
			if(MyFile_WriteBuffer[i] != MyFile_ReadBuffer[i])		// У������
			{
				printf("У��ʧ�ܣ�����SD�������¸�ʽ��!\r\n");
				f_close(&MyFile);	  //�ر��ļ�	
				return ERROR;
			}
		}
		printf("У��ɹ�������������Ϊ��\r\n");
		printf("%s\r\n",MyFile_ReadBuffer);
	}	
	else
	{
		printf("�޷���ȡ�ļ�������SD�������¸�ʽ��!\r\n");
		f_close(&MyFile);	  //�ر��ļ�	
		return ERROR;		
	}	
	
	f_close(&MyFile);	  //�ر��ļ�	
	return SUCCESS;
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
