//头文件

#include "main.h"
#include "adc.h"
#include "tim.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"

//全局变量
unsigned char buf[30];
u8 LCD_Mode,PWM_Mode;

float V37;
u8 PA6_zhankonbi,PA7_zhankonbi;


//定义函数
void KEY_Proc()
{
	static u32 B1_Tick,B2_Tick,B3_Tick,B4_Tick;
	static u32 B1_Count,B2_Count,B3_Count,B4_Count;
	if(B1==0)//B1按下
	{
		if(uwTick-B1_Tick<20) return;
		B1_Tick=uwTick;
		B1_Count++;
	}
	if(B1==1 & B1_Count!=0)//松手
	{
		B1_Count=0;
		LCD_Mode++;
		LCD_Mode%=2;
	}
	
	if(PWM_Mode==1)
	{
		if(B2==0)//B2按下
		{
			if(uwTick-B2_Tick<20) return;
			B2_Tick=uwTick;
			B2_Count++;
		}
		if(B2==1 & B2_Count!=0)//松手
		{
			B2_Count=0;
			PA6_zhankonbi+=10;
			if(PA6_zhankonbi>90){PA6_zhankonbi=10;}
		}
		
		
		if(B3==0)//B3按下
		{
			if(uwTick-B3_Tick<20) return;
			B3_Tick=uwTick;
			B3_Count++;
		}
		if(B3==1 & B3_Count!=0)//松手
		{
			B3_Count=0;
			PA7_zhankonbi+=10;
			if(PA7_zhankonbi>90){PA7_zhankonbi=10;}
		}
	}
	
	if(B4==0)//B4按下
	{
		if(uwTick-B4_Tick<20) return;
		B4_Tick=uwTick;
		B4_Count++;
	}
	if(B4==1 & B4_Count!=0)//松手
	{
		B4_Count=0;
		PA6_zhankonbi=10;
		PA7_zhankonbi=10;
		PWM_Mode++;
		PWM_Mode%=2;
}
	
	
	
}

void PWM_Proc()
{
	if(PWM_Mode==0)
	{
		if(V37==3.3)
		{
			PA6_zhankonbi=100;
			PA7_zhankonbi=100;
		}
		else if(V37==0)
		{
			PA6_zhankonbi=0;
			PA7_zhankonbi=0;
		}
		else
		{
			PA6_zhankonbi=V37/3.3f*100;
			PA7_zhankonbi=V37/3.3f*100;
		}
	}
	TIM16->CCR1=PA6_zhankonbi*(TIM16->ARR+1)/100;
	TIM17->CCR1=PA7_zhankonbi*(TIM17->ARR+1)/100;
}
void ADC_GET()
{
	static u32 ADC_Tick;
	if(uwTick-ADC_Tick<99) return;
	ADC_Tick=uwTick;
	HAL_ADC_Start(&hadc2);
	V37=HAL_ADC_GetValue(&hadc2)*3.3f/4095.0f;	
}
void LCD_Proc()
{
	if(LCD_Mode==0)
	{
		sprintf((char*)buf,"      Date          ");
		LCD_DisplayStringLine(Line0,buf);	
		sprintf((char*)buf,"    V:%.2fV          ",V37);
		LCD_DisplayStringLine(Line2,buf);	
		
		if(PWM_Mode==0){LCD_DisplayStringLine(Line4,(unsigned char *)"    Mode:AUTO     ");}
		if(PWM_Mode==1){LCD_DisplayStringLine(Line4,(unsigned char *)"    Mode:MANU     ");}
		
	}
	
	if(LCD_Mode==1)
	{
		
		sprintf((char*)buf,"      Para          ");
		LCD_DisplayStringLine(Line0,buf);	
		sprintf((char*)buf,"    PA6:%d%%          ",PA6_zhankonbi);
		LCD_DisplayStringLine(Line2,buf);	
		sprintf((char*)buf,"    PA7:%d%%          ",PA7_zhankonbi);
		LCD_DisplayStringLine(Line4,buf);	
	}
}

struct __FILE
{
  int handle;
};

FILE __stdout;
int fputc(int ch, FILE *f) 
{
	
  return ch;
}



void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC2_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
	LCD_Init();
	LED_Init();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	HAL_TIM_PWM_Start(&htim16,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	
	while (1)
  {
		KEY_Proc();
		PWM_Proc();
    ADC_GET();
		LCD_Proc();
		if(LCD_Mode==0){LED_ON(0x02,1);}
		else{LED_ON(0x02,0);}
		if(PWM_Mode==0){LED_ON(0x01,1);}
		else{LED_ON(0x01,0);}
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
