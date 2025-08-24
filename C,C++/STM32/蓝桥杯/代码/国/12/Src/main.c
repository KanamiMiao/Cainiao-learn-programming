//
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "gpio.h"
#include "stdio.h"

//
u8 LCD_Flag,Mode;//0->A,1->B

u8 buff[30];
static u32 tick;
static u8 count;
u16 dma_date[2];
float PA3,PA4,PA5;
u32 f,f2; u8 ax,bx;
float a[2],b[2];//0->new value
//角度数据α（a）、角度数据β（b）、频率数据（f）、角度数据变化量（ax、bx）
u8 Pax,Pbx; u16 Pf;//角度变化量参数 Pax 、Pbx 和频率参数（Pf）

//
void ADC_Get();
void LCD_Proc();
void KEY1();
void KEY2();
void KEY3();
void KEY4();
void KEY_Proc();

void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
	LCD_Init();

	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	HAL_ADC_Start_DMA(&hadc2,(u32*)dma_date,2);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);//PULS1
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_3);//PULS1
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);//PWM1
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_2);//PWM2
  while (1)
  {
		ADC_Get();
		LCD_Proc();
		KEY_Proc();
  }

}
void LCD_Proc()
{
	sprintf((char*)buff,"%.1f %.1f",PA4,PA5);
	LCD_DisplayStringLine(Line0,buff);	
	if(LCD_Flag==0)
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"        DATA                 ");
		sprintf((char*)buff,"   a:%.1f        ",a[0]);
		LCD_DisplayStringLine(Line2,buff);
		sprintf((char*)buff,"   b:%.1f        ",b[0]);
		LCD_DisplayStringLine(Line3,buff);
		sprintf((char*)buff,"   f:%d,f2:%d          ",f,f2);
		LCD_DisplayStringLine(Line4,buff);
		
		if(a[0]>a[1])ax=a[0]-a[1];
		else ax=a[1]-a[0];
		a[1]=a[0];
		if(b[0]>b[1])bx=b[0]-b[1];
		else bx=b[1]-b[0];
		b[1]=b[0];
		
		sprintf((char*)buff,"   ax:%d         ",ax);
		LCD_DisplayStringLine(Line6,buff);
		sprintf((char*)buff,"   bx:%d         ",bx);
		LCD_DisplayStringLine(Line7,buff);
		if(Mode==0)
			LCD_DisplayStringLine(Line8,(unsigned char *)"   mode:A                 ");
		else
			LCD_DisplayStringLine(Line8,(unsigned char *)"   mode:B                 ");
	}	
	
	else if(LCD_Flag==1)
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"        PARA                 ");
		sprintf((char*)buff,"   Pax:%d         ",Pax);
		LCD_DisplayStringLine(Line2,buff);
		sprintf((char*)buff,"   Pbx:%d         ",Pbx);
		LCD_DisplayStringLine(Line3,buff);
		sprintf((char*)buff,"   Pf:%d          ",Pf);
		LCD_DisplayStringLine(Line4,buff);
		
		LCD_DisplayStringLine(Line6,(unsigned char *)"                    ");	
		LCD_DisplayStringLine(Line7,(unsigned char *)"                    ");	
		LCD_DisplayStringLine(Line8,(unsigned char *)"                    ");	
	}		
}
void KEY_Proc()
{
	KEY1();
//	KEY2();
}
void KEY1()
{
	if(B1==0)//按下
	{
		if(uwTick-tick<100) return;
		tick=uwTick;
		count++;
	}
	else if(count!=0)
	{
		count=0;
		LCD_Flag++;
		LCD_Flag%=2;
	}
}
void KEY2()
{
	if(B2==0)//按下
	{
		if(uwTick-tick<100) return;
		tick=uwTick;
		count++;
	}
	else if(count!=0)
	{
		count=0;
	}
}
void ADC_Get()
{
	HAL_ADC_Start(&hadc1);
	HAL_ADC_Start(&hadc2);
	if(uwTick-tick<100) return;
	tick=uwTick;
	PA3=HAL_ADC_GetValue(&hadc1)*3.3/4095.0;
	PA4=dma_date[0]*3.3/4095.0;
	PA5=dma_date[1]*3.3/4095.0;
}
//u8 CaptureNum;
//u16 value1,value2,value_low,value_high;
//float zhan_kon_bi;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		{
			static u8 CaptureNum;
			static u16 value1,value2,value;
			if(CaptureNum==0)
			{
				value1=TIM2->CCR2;
				CaptureNum=1;
			}
			else if(CaptureNum==1)
			{
				value2=TIM2->CCR2;
				CaptureNum=0;
				if(value2>value1)
					value=value2-value1;
				else
					value=0xFF+value2-value1;
				f=1000*1000/value;
			}
		}
		
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_3)
		{
			static u8 CaptureNum;
			static u16 value1,value2,value;
			if(CaptureNum==0)
			{
				value1=TIM2->CCR3;
				CaptureNum=1;
			}
			else if(CaptureNum==1)
			{
				value2=TIM2->CCR3;
				CaptureNum=0;
				if(value2>value1)
					value=value2-value1;
				else
					value=0xFF+value2-value1;
				f2=1000*1000/value;
			}
		}			
	}
	
	
	if(htim->Instance==TIM3)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_1)
		{
			static u8 CaptureNum;
			static u16 value1,value2,value_low,value_high;
			static float zhan_kon_bi;
			if(CaptureNum==0)
			{
				value1=TIM3->CCR1;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim,TIM_CHANNEL_1,TIM_INPUTCHANNELPOLARITY_FALLING);
				CaptureNum=1;
			}
			else if(CaptureNum==1)
			{
				value2=TIM3->CCR1;
				if(value2>value1)
					value_high=value2-value1;
				else
					value_high=0xFFFF+value2-value1;
				value1=value2;
				CaptureNum=2;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim,TIM_CHANNEL_1,TIM_INPUTCHANNELPOLARITY_RISING);
			}
			else if(CaptureNum==2)
			{
				value2=TIM3->CCR1;
				if(value2>value1)
					value_low=value2-value1;
				else
					value_low=0xFFFF+value2-value1;
				CaptureNum=0;
				zhan_kon_bi=value_high*1.0/(value_high+value_low);
				if(zhan_kon_bi<0.1)
					a[0]=0;
				else if(zhan_kon_bi>0.9)
					a[0]=180;
				else
					a[0]=225*zhan_kon_bi-22.5;
			}
		}
		
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		{
			static u8 CaptureNum;
			static u16 value1,value2,value_low,value_high;
			static float zhan_kon_bi;
			if(CaptureNum==0)
			{
				value1=TIM3->CCR2;
				CaptureNum=1;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim,TIM_CHANNEL_2,TIM_INPUTCHANNELPOLARITY_FALLING);
			}
			else if(CaptureNum==1)
			{
				value2=TIM3->CCR2;
				CaptureNum=2;
				__HAL_TIM_SET_CAPTUREPOLARITY(htim,TIM_CHANNEL_2,TIM_INPUTCHANNELPOLARITY_RISING);
				if(value2>value1)
					value_high=value2-value1;
				else
					value_high=0xFFFF+value2-value1;
				value1=value2;
			}
			else if(CaptureNum==2)
			{
				value2=TIM3->CCR2;
				CaptureNum=0;
				if(value2>value1)
					value_low=value2-value1;
				else
					value_low=0xFFFF+value2-value1;
				zhan_kon_bi=value_high*1.0/(value_high+value_low);
				if(zhan_kon_bi<0.1)
					b[0]=0;
				else if(zhan_kon_bi>0.9)
					b[0]=90;
				else
					b[0]=112.5*zhan_kon_bi-11.25;
			}
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
