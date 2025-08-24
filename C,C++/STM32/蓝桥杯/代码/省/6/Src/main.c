//头文件

#include "main.h"
#include "adc.h"
#include "rtc.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "i2c_hal.h"

//全局变量
u8 tx_buff[20];
u8 rx_buff[20];
u8 rx_date[2];
u8 rx_printer[2];

unsigned char buf[20];
u32 ADC_Tick,Clock_Tick,LED_Tick;
float V1;
float k=0.1,VDD
	=3.3;
u8 hour=23,min=59,sec=55;
u8 hour_send=0,min_send=0,sec_send=0,time_send=0;
u8 led_ctrl=1;
u8 flag=1;
u8 Clean_Panduan,xiaochu;
u8 change;
//定义函数
void Clock()
{
	if(uwTick-Clock_Tick<1000) return;
	Clock_Tick=uwTick;
	sec++;
	if(sec>59)
	{
		sec=0;
		min++;
		if(min>59)
		{
			min=0;
			hour++;
			if(hour>23){hour=0;}
		}
	}
}
void ADC_GET()
{
	u32 ADC_Num;
	if(uwTick-ADC_Tick<1000) return;
	ADC_Tick=uwTick;
	HAL_ADC_Start(&hadc2);
	HAL_ADC_GetValue(&hadc2);
	ADC_Num=HAL_ADC_GetValue(&hadc2);
	V1=ADC_Num*3.3f/4095.0f;
}

void KEY1()
{
	static u8 count;
	if(count!=0 && count!=1 && B1==1)
	{
		if(led_ctrl==1){led_ctrl=0;}
		else{led_ctrl=1;}
		count=0;
	}
	if(B1==0)
	{
		if(uwTick-LED_Tick<100) return;
		LED_Tick=uwTick;
		count++;
	}
}

void KEY2()
{
	static u8 count;
	if(count!=0 && count!=1 && B2==1)
	{
		if(flag==1){flag=2;}
		else{flag=1;}
		Clean_Panduan=0;
		change=0;
		count=0;
	}
	if(B2==0)
	{
		if(uwTick-LED_Tick<100) return;
		LED_Tick=uwTick;
		count++;
	}
}

void KEY3()
{
	static u8 count;
	if(count!=0 && count!=1 && B3==1)
	{
		change++;
		xiaochu=1;
		time_send=0;
		if(change>3){change=1;}
		count=0;
	}
	if(B3==0)
	{
		if(uwTick-LED_Tick<100) return;
		LED_Tick=uwTick;
		count++;
	}
}

void KEY4()
{
	static u8 count;
	if(count!=0 && count!=1)
	{
		time_send++;
		count=0;
	}
	if(B4==0)
	{
		if(uwTick-LED_Tick<120) return;
		LED_Tick=uwTick;
		count++;
	}
}


void LCD1()
{
	Clock();
	ADC_GET();
	if(Clean_Panduan==0){LCD_Clear(Black);Clean_Panduan=1;}
	sprintf((char*)buf,"  V1:%.2fV  ",V1);
	LCD_DisplayStringLine(Line2,buf);	
	sprintf((char*)buf,"  k:%.1f  ",k);
	LCD_DisplayStringLine(Line4,buf);	
	if(led_ctrl==0){sprintf((char*)buf,"  LED:OFF  ");}
	if(led_ctrl==1){sprintf((char*)buf,"  LED:ON  ");}	
	LCD_DisplayStringLine(Line6,buf);	
	sprintf((char*)buf,"  T:%02d-%02d-%02d     ",hour,min,sec);
	LCD_DisplayStringLine(Line8,buf);	
	sprintf((char*)buf,"                   1");
	LCD_DisplayStringLine(Line9,buf);	
}

void LCD2()
{
	KEY3();
	KEY4();
	if(change==0||xiaochu==1)
	{
		if(Clean_Panduan==0){LCD_Clear(Black);Clean_Panduan=1;}
		sprintf((char*)buf,"      %02d-%02d-%02d      ",hour_send,min_send,sec_send);
		LCD_DisplayStringLine(Line5,buf);
		sprintf((char*)buf,"                   2");
		LCD_DisplayStringLine(Line9,buf);	
		sprintf((char*)buf,"      Setting      ");
		LCD_DisplayStringLine(Line2,buf);	
	}
	if(change==1)
	{
		if(time_send==0){time_send=hour_send;}
		hour_send=time_send;
		if(hour_send>23){hour_send=0;time_send=0;}
		LCD_SetTextColor(Red);
		LCD_DisplayChar(Line5,320-16*6,hour_send/10+'0');
		LCD_DisplayChar(Line5,320-16*7,hour_send%10+'0');
		LCD_SetTextColor(White);
		xiaochu=0;
	}
	
	if(change==2)
	{
		if(time_send==0){time_send=min_send;}
		min_send=time_send;
		if(min_send>59){min_send=0;time_send=0;}
		LCD_SetTextColor(Red);
		LCD_DisplayChar(Line5,320-16*9,min_send/10+'0');
		LCD_DisplayChar(Line5,320-16*10,min_send%10+'0');
		LCD_SetTextColor(White);
		xiaochu=0;
	}
	
	if(change==3)
	{
		if(time_send==0){time_send=sec_send;}
		sec_send=time_send;
		if(sec_send>59){sec_send=0;time_send=0;}
		LCD_SetTextColor(Red);
		LCD_DisplayChar(Line5,320-16*12,sec_send/10+'0');
		LCD_DisplayChar(Line5,320-16*13,sec_send%10+'0');
		LCD_SetTextColor(White);
		xiaochu=0;
	}
	
}
void SystemClock_Config(void);
struct __FILE
{
  int handle;
};

FILE __stdout;
int fputc(int ch, FILE *f) 
{
	HAL_UART_Transmit(&huart1,(u8*)&ch,1,50);
  return ch;
}
void TX()
{
	static u8 hhh;
	if(hour==hour_send && min==min_send && sec==sec_send&hhh==0)
	{
		printf("%.2f+%.1f+%02d%02d%02d\n",V1,k,hour,min,sec);
		hhh=1;
	}
}
//void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
//{
//	HAL_UART_Receive_IT(&huart1,rx_buff,1);
//	rx_buff[rx_printer++]=rx_date[0];
//}
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC2_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
	LCD_Init();
	LED_Init();
	I2CInit();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	HAL_UART_Receive_IT(&huart1,rx_buff,1);
	printf("HHHHHHHH\n");
  while (1)
  {
		if(flag==1){LCD1();}
		if(flag==2){LCD2();}
		KEY1();
		KEY2();
		if(V1>VDD*k && led_ctrl==1){LED1();}else{LED_Init();}
		TX();
  }
  /* USER CODE END 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12CLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

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
