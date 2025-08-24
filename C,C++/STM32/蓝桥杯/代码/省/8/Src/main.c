//
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"
#include "stdio.h"
RTC_DateTypeDef D;
RTC_TimeTypeDef T;

//
unsigned char buf[20];
u8 pin_tai=1;
u8 Mubiao_Place[4];
u8 Counter;//设置多个平台
u8 B1_Count,B2_Count,B3_Count,B4_Count;
u32 B1_Tick,B2_Tick,B3_Tick,B4_Tick;
u8 shen_jiang;//判断升降
u8 sec;
u32 tick;
int max;
//
void x()
{
	int x;
		for(x=0;x<4;x++)
	{
		if(Mubiao_Place[x]==pin_tai){Mubiao_Place[x]=0;max=0;}
	}
}
void jishu()
{
	if(uwTick-tick<1000) return;
	tick=uwTick;
	sec++;
}

void bijiao()//比较
{

	int i;
	for(i=0;i<4;i++)
	{
		if(max<Mubiao_Place[i])
		{max=Mubiao_Place[i];}
	}
}

void KEY_anxia()
{
	if(B1==0 & pin_tai!=1){if(uwTick-B1_Tick<80) return;B1_Tick=uwTick;B1_Count++;}
	if(B2==0 & pin_tai!=2){if(uwTick-B2_Tick<80) return;B2_Tick=uwTick;B2_Count++;}
	if(B3==0 & pin_tai!=3){if(uwTick-B3_Tick<80) return;B3_Tick=uwTick;B3_Count++;}
	if(B4==0 & pin_tai!=4){if(uwTick-B4_Tick<80) return;B4_Tick=uwTick;B4_Count++;}
}
void KEY()
{
	KEY_anxia();
	if(B1==1&B2==1&B3==1&B4==1)//松手扫描
	{
		if(B1_Count!=0 & Mubiao_Place[0]==0){Mubiao_Place[0]=1;Counter++;B1_Count=0;}
		if(B2_Count!=0 & Mubiao_Place[1]==0){Mubiao_Place[1]=2;Counter++;B2_Count=0;}
		if(B3_Count!=0 & Mubiao_Place[2]==0){Mubiao_Place[2]=3;Counter++;B3_Count=0;}
		if(B4_Count!=0 & Mubiao_Place[3]==0){Mubiao_Place[3]=4;Counter++;B4_Count=0;}
}
}
void panduan()
{
	if(max==0){shen_jiang=0;}
	if(max!=0)
	{
		if(max>pin_tai){shen_jiang=1;}//升
		if(max<pin_tai){shen_jiang=2;}//降
		
	
		if(shen_jiang==1)
		{
			jishu();
			if(sec==6)
			{
				pin_tai++;
				if(pin_tai==Mubiao_Place[0] |
					 pin_tai==Mubiao_Place[1] |
					 pin_tai==Mubiao_Place[2] |
					 pin_tai==Mubiao_Place[3] ){Counter--;}
					 
				sec=0;
			}
		}
		
		if(shen_jiang==2)
		{
			jishu();
			if(sec==6)
			{
				pin_tai--;
				if(pin_tai==Mubiao_Place[0] |
					 pin_tai==Mubiao_Place[1] |
					 pin_tai==Mubiao_Place[2] |
					 pin_tai==Mubiao_Place[3] ){Counter--;}
					 
				sec=0;
			}
		}
	}
}
void RTC_GET()
{
	HAL_RTC_GetDate(&hrtc,&D,RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc,&T,RTC_FORMAT_BIN);
}
void LED_Proc()
{
	LED_On(0x0F);
	LED_Off(0x01<<(pin_tai-1));
	if(shen_jiang==1){LED_liushui(1);}
	if(shen_jiang==2){LED_liushui(2);}
	if(shen_jiang==0){LED_Off(0xF0);}
}
void LCD()
{
	sprintf((char*)buf,"     Now Place       ");
	LCD_DisplayStringLine(Line2,buf);
	sprintf((char*)buf,"         %d           ",pin_tai);
	LCD_DisplayStringLine(Line5,buf);
	sprintf((char*)buf,"      %02d:%02d:%02d    ",T.Hours,T.Minutes,T.Seconds);
	LCD_DisplayStringLine(Line8,buf);
}
void SystemClock_Config(void);
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_TIM16_Init();
  MX_TIM17_Init();
	LCD_Init();
	LED_Init();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,GPIO_PIN_SET);//开门
	
//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	
  while (1)
  {
		RTC_GET();
		KEY();
		x();
		bijiao();
		panduan();
		LED_Proc();
		
		LCD();
		
		LCD_SetTextColor(Red);
		sprintf((char*)buf,"sec:%d",sec);
		LCD_DisplayStringLine(Line6,buf);
		sprintf((char*)buf,"Counter:%d",Counter);
		LCD_DisplayStringLine(Line7,buf);
		sprintf((char*)buf,"sj:%d",shen_jiang);
		LCD_DisplayStringLine(Line0,buf);
		sprintf((char*)buf,"bj:%d  ",max);
		LCD_DisplayStringLine(Line1,buf);
		sprintf((char*)buf,"%d %d %d %d",Mubiao_Place[0],Mubiao_Place[1],Mubiao_Place[2],Mubiao_Place[3]);
		LCD_DisplayStringLine(Line3,buf);
		LCD_SetTextColor(White);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

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
