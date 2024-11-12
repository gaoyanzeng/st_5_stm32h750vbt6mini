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
#include "key.h"
#include <string.h>
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "usbd_cdc_if.h"
#include "lcd_model.h"
#include "key.h"


/*****************************************************************************************/

#define Camera_Buffer	0x24000000    // ����ͷͼ�񻺳���


/******************************************************************************/

void SystemClock_Config(void);
void MPU_Config(void);					// MPU����

/******************************************************************************/

#define	NumOf_Blocks	64

/******************************************************************************/

int32_t SD_Status ; 		 //SD������־λ


/***************************************************************************************************
*	�� �� ��: main
*	��ڲ���: ��
*	�� �� ֵ: ��
*	˵    ��: ��
****************************************************************************************************/
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	MPU_Config();				// MPU����
  SCB_EnableICache();
  SCB_EnableDCache();
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
	MX_USART1_UART_Init();
	
	KEY_Init();
	
	SPI_LCD_Init();      // Һ�����Լ�SPI��ʼ��
	
	DCMI_OV2640_Init();     // DCMI�Լ�OV2640��ʼ��	
	
	OV2640_DMA_Transmit_Continuous(Camera_Buffer, OV2640_BufferSize);  // ����DMA��������
	
//	OV2640_DMA_Transmit_Snapshot(Camera_Buffer, OV2640_BufferSize);	 // ����DMA���δ���
	
	
  while (1)
  {	
		uint8_t keyPressCount = KEY_Scan1();

        // ���ݰ������µĴ����ı�LED��״̬
        if (keyPressCount % 5 == 1) 
				{
						LED1_Toggle;
					  HAL_Delay(200);
        }
				if (keyPressCount % 5 == 2) 
				{
					if (DCMI_FrameState == 1)	// �ɼ�����һ֡ͼ��
						{		
							DCMI_FrameState = 0;		// �����־λ
							LCD_CopyBuffer(0,0,Display_Width,Display_Height, (uint16_t *)Camera_Buffer);	// ��ͼ�����ݸ��Ƶ���Ļ		
							LCD_DisplayString( 84 ,240,"FPS:");
							LCD_DisplayNumber( 132,240, OV2640_FPS,2) ;	// ��ʾ֡��						
						}	
        }
				if (keyPressCount % 5 == 3) 
				{
					LCD_SetBackColor(LCD_BLACK); 			//	���ñ���ɫ
					LCD_Clear(); 											// ����
					LCD_SetColor(LIGHT_GREEN);				// ���û�����ɫ 				
					SD_Status = BSP_SD_Init(SD_Instance);	//SD����ʼ��											
					if( SD_Status == BSP_ERROR_NONE )	//����Ƿ��ʼ���ɹ�
					{		
						LCD_SetAsciiFont(&ASCII_Font32);		
						LCD_DisplayString(20,25,"SD Success");
						HAL_Delay(200);
					}
					else
					{
						LCD_SetAsciiFont(&ASCII_Font32);		
						LCD_DisplayString(20,25,"SD Error");
						HAL_Delay(200);
					}
        }
				if (keyPressCount % 5 == 4) 
				{						
						LCD_Test();//�ı���ʾ
						LCD_Picture();//ͼƬ							
						LCD_Line();//����		
						LCD_Rectangle();//����
						LCD_RouRectangle();//Բ�Ǿ���
						LCD_Ellipse();//��Բ	
						LCD_Circle();//Բ
						LCD_Triangle();//������
						LCD_Arc();//Բ��
						LCD_Polygon();//�����
						LCD_Clock();//ʱ��
					  break;
				}
  }
}
/****************************************************************************************************/

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


//	����MPU
//
void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	HAL_MPU_Disable();		// �Ƚ�ֹMPU

	MPU_InitStruct.Enable 				= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0x24000000;
	MPU_InitStruct.Size 					= MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission 	= MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable 		= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number 				= MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField 		= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable 	= 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);	

	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);	// ʹ��MPU
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