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


//头文件

#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "i2c_hal.h"
#include "led.h"

void SystemClock_Config(void);



//全局变量
unsigned char jiemian=1;
unsigned char value[20];
unsigned int x_shop=0,y_shop=0;
float x_price=1.0,y_price=1.0;
unsigned char x_rep=10,y_rep=10;
unsigned char buf[2];
u8 LED1=0;//LED1的亮灭
//定义函数

//1.界面相关
void Section_Change()//切换界面
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0)
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0);
		HAL_Delay(20);
		jiemian++	;
		if(jiemian>3){jiemian=1;}
	}
}
void Section1()//界面1
{	
	sprintf((char*)value,"        SHOP      ");
	LCD_DisplayStringLine(Line1,value);
	sprintf((char*)value,"     X:%d         ",x_shop);      //界面1初始值
	LCD_DisplayStringLine(Line3,value);
	sprintf((char*)value,"     Y:%d         ",y_shop);
	LCD_DisplayStringLine(Line4,value);
	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0)              //按B2，x_shop+1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0);
		HAL_Delay(20);
		x_shop++;	
		if(x_shop==x_rep+1){x_shop=0;}                        //x_shop不超过x_rep
	}
	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)              //按B3，y_shop+1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
		HAL_Delay(20);
		y_shop++;	
		if(y_shop==y_rep+1){y_shop=0;}                        //y_shop不超过y_rep
	}
	
	if(LED1==1)
	{
		LED1=0;
		LED_Init();
		LED_On(1);
		HAL_Delay(5000);
		LED_Init();
	}
	
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)              //按B4，rep-shop,shop归零
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);
		HAL_Delay(20);
		x_rep-=x_shop;
		y_rep-=y_shop;
		printf("X:%.1f,Y:%.1f,Z:%.1f",x_price,y_price,x_price*x_shop+y_price*y_shop);		
		x_shop=0;
		y_shop=0;
		LED1=1;
	}
}


void Section2()//界面2
{
	sprintf((char*)value,"        PRICE      ");
	LCD_DisplayStringLine(Line1,value);
	sprintf((char*)value,"     X:%.1f         ",x_price);      //界面2初始值
	LCD_DisplayStringLine(Line3,value);
	sprintf((char*)value,"     Y:%.1f         ",y_price);
	LCD_DisplayStringLine(Line4,value);
	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0)              //按B2，x_price+0.1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0);
		HAL_Delay(20);
		x_price+=0.1;
		if(x_price==2.1){x_price=1.0;}
	}
	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)              //按B3，y_price+0.1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
		HAL_Delay(20);
		y_price+=0.1;	
		if(y_price==2.1){y_price=1.0;}
	}
	
	
}

void Section3()//界面3
{
	sprintf((char*)value,"        REP      ");
	LCD_DisplayStringLine(Line1,value);
	sprintf((char*)value,"     X:%d        ",x_rep);      //界面3初始值
	LCD_DisplayStringLine(Line3,value);
	sprintf((char*)value,"     Y:%d         ",y_rep);
	LCD_DisplayStringLine(Line4,value);
	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0)              //按B2，x_rep+1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0);
		HAL_Delay(20);
		x_rep++;
	}
	
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)              //按B3，y_rep+1
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
		HAL_Delay(20);
		y_rep++;	
	}
	
}




//2.串口
int fputc(int ch, FILE *f) 
{
	HAL_UART_Transmit(&huart1,(unsigned char*)&ch,1,50);//1字符，50ms延迟
	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)//回调函数
{
	buf[0]=HAL_UART_Receive_IT(&huart1,buf,1);
	if(buf[0]=='?')
	{
		printf("X:%.1f,Y:%.1f",x_price,y_price);
	}
}
//3.E2PROM
void E2PROM_Write(u8 add,u8 date)//写入
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
u8 E2PROM_Read(u8 add)//读取
{
	u8 data;
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	data=I2CReceiveByte();
	I2CSendNotAck();
	I2CStop();
	return data;
}
u8 rep_num;
float price_num;
//主程序

int main(void)
{
	//初始化：
  MX_GPIO_Init();
	SystemClock_Config();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
	LCD_Init();
	HAL_UART_Receive_IT(&huart1,buf,1);//串口接受1字符
	I2CInit();
	LED_Init();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	
	printf("X:%.1f,Y:%.1f",x_price,y_price);
	
  while (1)
  {
		Section_Change();//切换界面
		if(jiemian==1){Section1();}//进入界面1
		if(jiemian==2){Section2();}//进入界面2
		if(jiemian==3){Section3();}//进入界面3
			
		if((x_rep==0)&&(y_rep==0))
		{
			LED_Init();
			LED_On(2);
			HAL_Delay(100);
			LED_Init();
			HAL_Delay(100);
		}
		
		
		E2PROM_Write(1,x_rep);//读取EEPROM中x_rep
		E2PROM_Read(1);
		rep_num=E2PROM_Read(1);
		sprintf((char*)value,"x_rep:%d   ",rep_num);
		LCD_DisplayStringLine(Line6,value);
		
		E2PROM_Write(2,y_rep);
		E2PROM_Read(2);
		rep_num=E2PROM_Read(2);
		sprintf((char*)value,"y_rep:%d   ",rep_num);
		LCD_DisplayStringLine(Line7,value);
		
		E2PROM_Write(3,x_price*10);//读取EEPROM中x_price
		E2PROM_Read(3);
		price_num=E2PROM_Read(3)/10.f;
		sprintf((char*)value,"x_price:%.1f   ",price_num);
		LCD_DisplayStringLine(Line8,value);
		
		E2PROM_Write(4,y_price*10);
		E2PROM_Read(4);
		price_num=E2PROM_Read(4)/10.f;
		sprintf((char*)value,"y_price:%.1f   ",price_num);
		LCD_DisplayStringLine(Line9,value);
		
  }
  /* USER CODE END 3 */
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
