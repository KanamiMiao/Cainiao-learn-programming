/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "led.h"
#include "stdio.h"
#include "i2c_hal.h"



//全局变量
unsigned char value[30];                                     //要显示的字符
unsigned char B1_value=255,B2_value=255,B3_value=255;        //B1，B2，B3初始值
unsigned char Flag=0;
unsigned char Section_Num=1;                                 //页面1、2
unsigned int Passward;                                       //密码
unsigned int Passward_Ture=123;                              //真实密码
unsigned char Count=0;
unsigned char LED_Count=0;                                   //LED2闪烁计数
//定义函数
void Section1()//界面1的函数
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0)         //按下B1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0);
		HAL_Delay(20);
		B1_value++;
		B1_value%=10;
		Flag=1;
	}	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0)           //按下B2
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0);
		HAL_Delay(20);
		B2_value++;
		B2_value%=10;
		Flag=1;
	}	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)             //按下B3
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
		HAL_Delay(20);
		B3_value++;
		B3_value%=10;
		Flag=1;
	}	

	if(Flag==0)
	{
		sprintf((char*)value,"       PSD           ");       //初始界面1
		LCD_DisplayStringLine(Line1 ,value);
		sprintf((char*)value,"    B1:@             ");
		LCD_DisplayStringLine(Line3 ,value);
		sprintf((char*)value,"    B2:@             ");
		LCD_DisplayStringLine(Line4 ,value);
		sprintf((char*)value,"    B3:@             ");
		LCD_DisplayStringLine(Line5 ,value);
	}
	else
	{
		if(B1_value!=255)                                     //界面1显示数据
		{
			sprintf((char*)value,"    B1:%d     ",B1_value);
			LCD_DisplayStringLine(Line3 ,value);
		}
		if(B2_value!=255)
		{
			sprintf((char*)value,"    B2:%d     ",B2_value);
	  	LCD_DisplayStringLine(Line4 ,value);
		}
		if(B3_value!=255)
		{
			sprintf((char*)value,"    B3:%d     ",B3_value);
			LCD_DisplayStringLine(Line5 ,value);	
		}
	}
		
	Passward=B1_value*100+B2_value*10+B3_value;           //算密码
}

void Section2()//界面2的函数
{
	
	sprintf((char*)value,"       STR           ");      //界面2
	LCD_DisplayStringLine(Line1 ,value);
	sprintf((char*)value,"    F:2000Hz");
	LCD_DisplayStringLine(Line3 ,value);
	sprintf((char*)value,"    D:10%%"  );
	LCD_DisplayStringLine(Line4 ,value);
	sprintf((char*)value,"            ");
	LCD_DisplayStringLine(Line5 ,value);
	
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)          //切换为初始界面1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);    //记得延时
		HAL_Delay(20);

		Flag=0;
		B1_value=255;                                     //显示初始界面1的条件
		B2_value=255;
		B3_value=255;
		Section_Num=1;
	}
}











/* USER CODE END 0 */
void EEPROM_Write(u8 add,u8 date)
{
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CSendByte(date);
	I2CWaitAck();
	I2CStop();
	HAL_Delay(5);
}
u8 EEPROM_Read(u8 add)
{
	u8 data;
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CStop();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	data=I2CReceiveByte();
	I2CSendNotAck();
	I2CWaitAck();
	I2CStop();
	return data;
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  
	
	//初始化
	HAL_Init();
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
	LCD_Init();
	LED_Init();
	I2CInit();
	/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	//主函数
//	
//	unsigned char i;
//	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
//	__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,i);
//	
	u16 num;
	while(1)
	{
		if(Section_Num==1)//显示界面1
		{
			Section1();
			if(Count>=3)//错3次以上，LED2闪
			{
				LED_Init();LED_On(2);HAL_Delay(100);LED_Init();HAL_Delay(100);LED_Count++;
			}
			if(LED_Count==25){LED_Count=0;Count--;}
		}
		EEPROM_Write(1,Passward/10);
		EEPROM_Write(2,Passward%10);
		EEPROM_Read(1);
		EEPROM_Read(2);
		num=EEPROM_Read(1)*10+EEPROM_Read(2);
		sprintf((char*)value,"EEPROM:%d      ",num);
		LCD_DisplayStringLine(Line9,value);
		
		sprintf((char*)value,"      %d       ",Passward);//显示密码
		LCD_DisplayStringLine(Line7 ,value);

		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)&&(Section_Num==2))//界面2时按下B4
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);//延时一会
			HAL_Delay(20);
			Section_Num=1;//密码正确时，发出切换信号
			Flag=0;B1_value=255;B2_value=255;B3_value=255;//配置初始界面1的参数
		}
		
		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)&&(Section_Num==1))//界面1时按下B4
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);//延时一会
			HAL_Delay(20);
			if(Passward==Passward_Ture){Count=0;Section_Num=2;}//密码正确时，发出切换信号，计数归零
			else{Flag=0;B1_value=255;B2_value=255;B3_value=255;Count++;}
			//密码错误时，配置初始界面1的参数，计数+1
						
			if(Section_Num==2){Section2();LED_Init();LED_On(1);HAL_Delay(5000);LED_Init();}
			//收到切换信号，显示界面2，LED1亮5s
		}

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV3;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
