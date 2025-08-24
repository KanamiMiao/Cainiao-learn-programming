//头文件
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "stdio.h"

//全局变量
u8 LCD_Flag,PA7_Change;

u8 buff[30];
u16 DMA_value[2];
float IC2_Frep,IC3_Frep,V1,V2;
u8 VD[2]={1,1},FD[2]={2,2};//指示灯  x[0]为真实值，x[1]为存储值

//函数
void ADC_Get()
{
	HAL_ADC_Start(&hadc2);
	static u32 tick;
	if(uwTick-tick>200)
	{
		tick=uwTick;
		V1=DMA_value[0]*3.3f/4095.0f;
		V2=DMA_value[1]*3.3f/4095.0f;
	}
}
void LCD_Proc();
void KEY1();
void KEY2();
void KEY3();
void KEY4();
void KEY_Proc();
void PWM_Proc();
void LED_Proc();
	
void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
	LCD_Init();
	LED_Init();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_3);
	HAL_ADC_Start_DMA(&hadc2,(u32*)DMA_value,2);
  while (1)
  {
		ADC_Get();
		LCD_Proc();
		KEY_Proc();
		PWM_Proc();
		LED_Proc();
			
		//验证区
		sprintf((char*)buff,"   VD:%d  FD:%d       ",VD[0],FD[0]);
		LCD_DisplayStringLine(Line8,buff);	
		sprintf((char*)buff,"%d   PA7:%d       ",PA7_Change,TIM3->ARR);
		LCD_DisplayStringLine(Line9,buff);	
  }

}
void LED_Proc()
{
	if(V1>V2)
		LED_On_Off(1,0x01<<(VD[0]-1));
	else
		LED_On_Off(0,0x01<<(VD[0]-1));
	
	if(IC2_Frep>IC3_Frep)
		LED_On_Off(1,0x01<<(FD[0]-1));
	else
		LED_On_Off(0,0x01<<(FD[0]-1));	
}
void LCD_Proc()
{
	if(LCD_Flag==0)
	{
		LED_On_Off(0,0x01<<(FD[0]-1));	
		LED_On_Off(0,0x01<<(VD[0]-1));	
		VD[0]=VD[1];
		FD[0]=FD[1];

		LCD_DisplayStringLine(Line1,(unsigned char *)"    DATA                    ");	
		sprintf((char*)buff,"    V1:%.1fV        ",V1);
		LCD_DisplayStringLine(Line3,buff);	
		sprintf((char*)buff,"    V2:%.1fV        ",V2);
		LCD_DisplayStringLine(Line4,buff);	
		sprintf((char*)buff,"    F1:%.0fHz      ",IC2_Frep);
		LCD_DisplayStringLine(Line5,buff);	
		sprintf((char*)buff,"    F2:%.0fHz      ",IC3_Frep);
		LCD_DisplayStringLine(Line6,buff);	
	}
	else
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"    PARA                    ");	
		sprintf((char*)buff,"    VD:%d        ",VD[1]);
		LCD_DisplayStringLine(Line3,buff);	
		sprintf((char*)buff,"    FD:%d        ",FD[1]);
		LCD_DisplayStringLine(Line4,buff);	
		LCD_DisplayStringLine(Line5,(unsigned char *)"                    ");	
		LCD_DisplayStringLine(Line6,(unsigned char *)"                    ");	
	}
}
void KEY_Proc()
{
	KEY1();
	KEY4();
	if(LCD_Flag)
	{
		KEY2();
		KEY3();
	}
}
void KEY1()
{
	static u32 tick;
	static u8 count;
	if(uwTick-tick>100)
	{
		tick=uwTick;
		if(B1==0)	count++;
		else if(count)
		{
			count=0;
			LCD_Flag++;
			LCD_Flag%=2;
		}
	}
}

void KEY2()
{
	static u32 tick;
	static u8 count;
	if(uwTick-tick>100)
	{
		tick=uwTick;
		if(B2==0)	count++;
		else if(count)
		{
			count=0;
			if(FD[1]-VD[1]==1) VD[1]+=2;
			else VD[1]++;
			if(VD[1]>8) VD[1]=1;
		}
	}
}

void KEY3()
{
	static u32 tick;
	static u8 count;
	if(uwTick-tick>100)
	{
		tick=uwTick;
		if(B3==0)	count++;
		else if(count)
		{
			count=0;
			if(VD[1]-FD[1]==1) FD[1]+=2;
			else FD[1]++;
			if(FD[1]>8) FD[1]=1;
		}
	}
}

void KEY4()
{
	static u32 tick;
	static u8 count;
	if(uwTick-tick>100)
	{
		tick=uwTick;
		if(B4==0)	count++;
		else if(count)
		{
			count=0;
			PA7_Change++;
			PA7_Change%=2;
		}
	}
}

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

void PWM_Proc()
{
	if(PA7_Change==0)
		TIM3->ARR=1000*1000/IC2_Frep-1;
	else
		TIM3->ARR=1000*1000/IC3_Frep-1;
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static u16 IC2_Num,IC3_Num;
	static u16 IC2_Num1,IC3_Num1,IC2_Num2,IC3_Num2;
	static u8 flag1,flag2;

	if(htim->Instance==TIM2)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		{
			if(flag1==0)			
			{
				IC2_Num1=TIM2->CCR2;
				flag1=1;
			}
			else if(flag1==1)
			{
				IC2_Num2=TIM2->CCR2;
				
				if(IC2_Num2>IC2_Num1)
					IC2_Num=(IC2_Num2-IC2_Num1);
				else
					IC2_Num=((0xFFFF-IC2_Num1)+IC2_Num2);
				
				static u32 tick; 					
				if(uwTick-tick>200)
				{
					tick=uwTick;
					IC2_Frep=1000*1000/IC2_Num;
				}
				flag1=0;
			}
		}
		
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_3)
		{
			if(flag2==0)
			{
				IC3_Num1=TIM2->CCR3;
				flag2=1;
			}
			else if(flag2==1)
			{
				IC3_Num2=TIM2->CCR3;
				if(IC3_Num2>IC3_Num1)
					IC3_Num=(IC3_Num2-IC3_Num1);
				else
					IC3_Num=((0xFFFF-IC3_Num1)+IC3_Num2);
				
				static u32 tick; 					
				if(uwTick-tick>200)
				{
					tick=uwTick;
					IC3_Frep=1000*1000/IC3_Num;
				}
				flag2=0;
			}
		}
	}
}

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
