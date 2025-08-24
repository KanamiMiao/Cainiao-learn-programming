//头文件
#include "main.h"
#include "adc.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"

//全局变量
u32 B1_Count,B2_Count,B3_Count,B4_Count;
u32 B1_Tick,B2_Tick,B3_Tick,B4_Tick;     
u8 xiaochu_gaoliang;//1为需要消除


unsigned char buf[30];
u8 LCD_Flag=1,LCD_Change;
float V37,Vmax=2.4,Vmin=1.2;
u8 panduan_V37;//1低 2高
u8 Upper_LED=1,Lower_LED=2;
//定义函数
void KEY1()
{
	if (B1==0)//按下，扫描，计数
	{
		if (uwTick-B1_Tick<20) return;//20ms计数1次
		B1_Tick=uwTick;
		B1_Count++;
	}
	
	if (B1==1 & B1_Count!=0)//松手检测次数
	{
		B1_Count=0;
		LCD_Change=0;
		if(LCD_Flag==1){LCD_Flag=2;}
		else{LCD_Flag=1;}
	}
}

void KEY2()
{
	if (B2==0)//按下，扫描，计数
	{
		if (uwTick-B2_Tick<20) return;//20ms计数1次
		B2_Tick=uwTick;
		B2_Count++;
	}
	
	if (B2==1 & B2_Count!=0)//松手检测次数
	{
		B2_Count=0;
		xiaochu_gaoliang=1;
		LCD_Change++;
		if(LCD_Change>4){LCD_Change=1;}
	}
}

void KEY3()
{
	if (B3==0)//按下，扫描，计数
	{
		if (uwTick-B3_Tick<20) return;//20ms计数1次
		B3_Tick=uwTick;
		B3_Count++;
	}
	
	if (B3==1 & B3_Count!=0)//松手检测次数
	{
		B3_Count=0;
		if(LCD_Change==1){Vmax+=0.3;if(Vmax>3.3){Vmax=3.3;}}
		if(LCD_Change==2){Vmin+=0.3;if(Vmin>3.3){Vmin=3.3;}}
		if(LCD_Change==3){Upper_LED++;if(Upper_LED>8){Upper_LED=8;}if(Lower_LED!=8){if(Upper_LED==Lower_LED){Upper_LED++;}}else if(Upper_LED==8){Upper_LED=7;}}
		if(LCD_Change==4){Lower_LED++;if(Lower_LED>8){Lower_LED=8;}if(Upper_LED!=8){if(Lower_LED==Upper_LED){Lower_LED++;}}else if(Lower_LED==8){Lower_LED=7;}}
	}
}

void KEY4()
{
	if (B4==0)//按下，扫描，计数
	{
		if (uwTick-B4_Tick<20) return;//20ms计数1次
		B4_Tick=uwTick;
		B4_Count++;
	}
	
	if (B4==1 & B4_Count!=0)//松手检测次数
	{
		B4_Count=0;
		if(LCD_Change==1){Vmax-=0.3;if(Vmax<0){Vmax=0;}}
		if(LCD_Change==2){Vmin-=0.3;if(Vmin<0){Vmin=0;}}
		if(LCD_Change==3){Upper_LED--;if(Upper_LED<1){Upper_LED=1;}if(Lower_LED!=1){if(Upper_LED==Lower_LED){Upper_LED--;}}else if(Upper_LED==1){Upper_LED=2;}}
		if(LCD_Change==4){Lower_LED--;if(Lower_LED<1){Lower_LED=1;}if(Upper_LED!=1){if(Lower_LED==Upper_LED){Lower_LED--;}}else if(Lower_LED==1){Lower_LED=2;}}
	}
}

void ADC_GET()
{
	static u32 adc_tick;
	if(uwTick-adc_tick<200) return;
	adc_tick=uwTick;
	HAL_ADC_Start(&hadc2);
	V37=HAL_ADC_GetValue(&hadc2)*3.3f/4095.0f;
}

void LCD_Proc()
{
	if(LCD_Flag==1)
	{
		ADC_GET();
		LCD_SetBackColor(Black);
		LCD_DisplayStringLine(Line3,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line5,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line7,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");
		
		sprintf((char*)buf,"        Main         ");
		LCD_DisplayStringLine(Line1,buf);	
		sprintf((char*)buf," Volt:%.2fV    ",V37);
		LCD_DisplayStringLine(Line4,buf);	
		sprintf((char*)buf," Status:Upper    ");
		LCD_DisplayStringLine(Line6,buf);	
	}
	
	
	if(LCD_Flag==2)
	{
		KEY2();
		KEY3();
		KEY4();
		if(LCD_Change==0 | xiaochu_gaoliang==1)
		{
			LCD_SetBackColor(Black);
			LCD_DisplayStringLine(Line4,(unsigned char *)"                    ");
			LCD_DisplayStringLine(Line6,(unsigned char *)"                    ");
			
			sprintf((char*)buf,"      Setting       ");
			LCD_DisplayStringLine(Line1,buf);	
			sprintf((char*)buf," Max Volt:%.1fV      ",Vmax);
			LCD_DisplayStringLine(Line3,buf);	
			sprintf((char*)buf," Min Volt:%.1fV      ",Vmin);
			LCD_DisplayStringLine(Line5,buf);	
			sprintf((char*)buf," Upper:LD%d            ",Upper_LED);
			LCD_DisplayStringLine(Line7,buf);	
			sprintf((char*)buf," Lower:LD%d            ",Lower_LED);
			LCD_DisplayStringLine(Line9,buf);	
		}
		
		if(LCD_Change==1)
		{
			LCD_SetBackColor(Green);
			sprintf((char*)buf," Max Volt:%.1fV      ",Vmax);
			LCD_DisplayStringLine(Line3,buf);	
			xiaochu_gaoliang=0;
		}
		if(LCD_Change==2)
		{
			LCD_SetBackColor(Green);
			sprintf((char*)buf," Min Volt:%.1fV      ",Vmin);
			LCD_DisplayStringLine(Line5,buf);	
			xiaochu_gaoliang=0;
		}
		if(LCD_Change==3)
		{
			LCD_SetBackColor(Green);
			sprintf((char*)buf," Upper:LD%d            ",Upper_LED);
			LCD_DisplayStringLine(Line7,buf);	
			xiaochu_gaoliang=0;
		}
		if(LCD_Change==4)
		{
			LCD_SetBackColor(Green);
			sprintf((char*)buf," Lower:LD%d            ",Lower_LED);
			LCD_DisplayStringLine(Line9,buf);	
			xiaochu_gaoliang=0;
		}
	}
	
	
}

void LED_Proc()
{
	if(V37>Vmax)//过高
	{
		LED_OFF(0x00<Lower_LED);
		static u32 tick;
		static u8 count;
		if(count%2==0){LED_ON(0x01<<(Upper_LED-1));}
		else{LED_OFF(0x01<<(Upper_LED-1));}
		if(uwTick-tick<200) return;
		tick=uwTick;
		count++;
	}
	
	if(V37<Vmin)//过高
	{
		LED_OFF(0x00<Upper_LED);
		static u32 tick;
		static u8 count;
		if(count%2==0){LED_ON(0x01<<(Lower_LED-1));}
		else{LED_OFF(0x01<<(Lower_LED-1));}
		if(uwTick-tick<200) return;
		tick=uwTick;
		count++;
	}
	else{LED_Init();}
}
void SystemClock_Config(void);


int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC2_Init();
	LCD_Init();
	LED_Init();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(Blue2);
	
//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	
  while (1)
  {
		LCD_Proc();
		KEY1();
		LED_Proc();
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
