//
#include "main.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "lcd.h"
#include "stdio.h"
#include "i2c_hal.h"


//
RTC_DateTypeDef D;
RTC_TimeTypeDef T;
u8 hour=0,min=0,sec=0;

unsigned char buf[30];
u8 PWM_PA1,PWM_PA2;
u8 moshi_PA1,moshi_PA2;

u8 rx_buf[30],rx_date[2],rx_count=0;
u8 PA_num,tim;


//
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
	I2CStop();
	return data;
}

void KEY1()
{
	static u32 key_tick;
	static u8 key_count;
	if(B1==0)
	{
		if(uwTick-key_tick<20) return;
		key_tick=uwTick;
		key_count++;
	}
	if(B1==1 & key_count!=0)
	{
		key_count=0;
		moshi_PA1++;
		moshi_PA1%=2;
	}
}

void KEY2()
{
	static u32 key_tick;
	static u8 key_count;
	if(B2==0)
	{
		if(uwTick-key_tick<20) return;
		key_tick=uwTick;
		key_count++;
	}
	if(B2==1 & key_count!=0)
	{	
		key_count=0;
		PWM_PA1+=10;
		if(PWM_PA1>100){PWM_PA1=0;}
		EEPROM_Write(1,PWM_PA1);
	}
}
void KEY3()
{
	static u32 key_tick;
	static u8 key_count;
	if(B3==0)
	{
		if(uwTick-key_tick<20) return;
		key_tick=uwTick;
		key_count++;
	}
	if(B3==1 & key_count!=0)
	{
		key_count=0;
		moshi_PA2++;
		moshi_PA2%=2;
	}
}

void KEY4()
{
	static u32 key_tick;
	static u8 key_count;
	if(B4==0)
	{
		if(uwTick-key_tick<20) return;
		key_tick=uwTick;
		key_count++;
	}
	if(B4==1 & key_count!=0)
	{
		key_count=0;
		PWM_PA2+=10;
		if(PWM_PA2>100){PWM_PA2=0;}
		EEPROM_Write(2,PWM_PA2);
	}
}

void PWM_Proc()
{
	KEY1();
	KEY2();
	KEY3();
	KEY4();
	if((moshi_PA1==0) || (PA_num==1 && ((T.Hours-hour)*60*60+(T.Minutes-min)*60+(T.Seconds-sec)==tim))){TIM2->CCR2=0;}
	if((moshi_PA1==1) || (PA_num==1 && ((T.Hours-hour)*60*60+(T.Minutes-min)*60+(T.Seconds-sec)==0))){TIM2->CCR2=PWM_PA1*(TIM2->ARR+1)/100;}
	if((moshi_PA2==0) || (PA_num==2 && ((T.Hours-hour)*60*60+(T.Minutes-min)*60+(T.Seconds-sec)==tim))){TIM2->CCR3=0;}
	if((moshi_PA2==1) || (PA_num==2 && ((T.Hours-hour)*60*60+(T.Minutes-min)*60+(T.Seconds-sec)==0))){TIM2->CCR3=PWM_PA2*(TIM2->ARR+1)/100;}
}
void RTC_GET()
{
	HAL_RTC_GetDate(&hrtc,&D,RTC_FORMAT_BIN);
	HAL_RTC_GetTime(&hrtc,&T,RTC_FORMAT_BIN);
}
void LCD_Proc()
{
	PWM_PA1=EEPROM_Read(1);
	PWM_PA2=EEPROM_Read(2);
	
	sprintf((char*)buf," PWM_PA1:%d%%          ",PWM_PA1);
	LCD_DisplayStringLine(Line0,buf);	
	sprintf((char*)buf," PWM_PA2:%d%%          ",PWM_PA2);
	LCD_DisplayStringLine(Line2,buf);	
	sprintf((char*)buf," Time:%02d:%02d:%02d          ",T.Hours,T.Minutes,T.Seconds);
	LCD_DisplayStringLine(Line4,buf);	
	if(moshi_PA1==1 & moshi_PA2==0){LCD_DisplayStringLine(Line6,(unsigned char*)" Channel:PA1          ");}
	if(moshi_PA1==0 & moshi_PA2==1){LCD_DisplayStringLine(Line6,(unsigned char*)" Channel:PA2          ");}
	if(moshi_PA1==0 & moshi_PA2==0){LCD_DisplayStringLine(Line6,(unsigned char*)" Channel:             ");}
	if(moshi_PA1==1 & moshi_PA2==1){LCD_DisplayStringLine(Line6,(unsigned char*)" Channel:PA1,PA2             ");}
		
	sprintf((char*)buf," Command:          ");
	LCD_DisplayStringLine(Line8,buf);	
}

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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(&huart1,rx_date,1);
	rx_buf[rx_count++]=rx_date[0];
//	printf("%c",rx_date[0]);
}
void RX()
{
	static u32 tick;
	if(uwTick-tick<50) return;
	tick=uwTick;
	if(rx_date[1]!=rx_date[0] & rx_count==15)
	{
		hour=(rx_buf[0]-48)*10+rx_buf[0]-48;
		min=(rx_buf[3]-48)*10+rx_buf[4]-48;
		sec=(rx_buf[6]-48)*10+rx_buf[7]-48;
		
		PA_num=rx_buf[11]-48;
		tim=rx_buf[13]-48;
		
		sprintf((char*)buf,"  %02d:%02d:%02d-PA%d-%ds     ",hour,min,sec,PA_num,tim);
		LCD_DisplayStringLine(Line9,buf);
		
		printf("  %02d:%02d:%02d-PA%d-%ds     ",hour,min,sec,PA_num,tim);
		rx_date[1]=rx_date[0];
		
	}
	else
	{
		static u8 x;
		if(x==0)
		{
			sprintf((char*)buf,"        None          ");
			LCD_DisplayStringLine(Line9,buf);	
			x=1;
		}
		rx_count=0;
	}
}
void SystemClock_Config(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
	LCD_Init();
	LED_Init();
	I2CInit();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
	
	printf("Hello World\r\n");
	
	HAL_UART_Receive_IT(&huart1,rx_date,1);
	

//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	
  while (1)
  {
		RX();
		PWM_Proc();
		RTC_GET();
		LCD_Proc();
		if(TIM2->CCR2==0){LED_Disp(0x01,0);}
		else{LED_Disp(0x01,1);}
		if(TIM2->CCR3==0){LED_Disp(0x02,0);}
		else{LED_Disp(0x02,1);}		
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
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
