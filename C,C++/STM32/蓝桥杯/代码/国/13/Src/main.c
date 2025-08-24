//
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "i2c_hal.h"
#include "stdio.h"
#include "string.h"

//
u8 LCD_Flag;
u8 LCD_Mode;
u8 ADC_Flag;//0->PA4,1->PA5
u8 PWM_Mode;//1分频，0倍频

u8 buff[30],rx_date[2],rx_buff[30],rx_count;
u16 DMA_date[2];
float PA4,PA5;
u32 PA1;
u8 X,Y,N[2];
float Min[2],Max[2],Ave[2];
float PA4_value[256],PA5_value[256];
u8 CaptureNum;
u16 GetValue1,GetValue2,GetValue;

//
void ADC_Get()
{
	PA4=DMA_date[0]*3.3f/4095.0f;
	PA5=DMA_date[1]*3.3f/4095.0f;
}
void EEPROM_Write(u8 add,u8 date);
u8 EEPROM_Read(u8 add);
void KEY1();
void KEY2();
void KEY3();
void KEY4();
void KEY_Proc();
void LCD_Proc();
void PWM_Proc();
void LED_Proc();

void SystemClock_Config(void);

int fputc(int ch, FILE *f) 
{
	HAL_UART_Transmit(&huart1,(u8*)&ch,1,50);
  return ch;
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(&huart1,rx_date,1);
	rx_buff[rx_count++]=rx_date[0];
}
void UART();

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_TIM17_Init();
  MX_USART1_UART_Init();
	LCD_Init();
	LED_Init();
	I2CInit();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	
	X=EEPROM_Read(0x00);
	Y=EEPROM_Read(0x01);

//	LCD_DisplayStringLine(Line9,(unsigned char *)"                    ");	
	HAL_ADC_Start_DMA(&hadc2,(u32*)DMA_date,2);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim17,TIM_CHANNEL_1);
	printf("Hello World\r\n");
	HAL_UART_Receive_IT(&huart1,rx_date,1);
  while (1)
  {
		HAL_ADC_Start(&hadc2);
		KEY_Proc();
		LCD_Proc();
		UART();
		LED_Proc();
		PWM_Proc();
  }
}
void PWM_Proc()
{
	if(PWM_Mode==0)
		TIM17->ARR=1000*1000/(PA1*X)-1;
	else if(PWM_Mode==1)
		TIM17->ARR=1000*1000/(PA1/X)-1;
	TIM17->CCR1=(TIM17->ARR+1)/2;
	
}
void LED_Proc()
{
	if(PWM_Mode==0)
	{
		LED_Disp(0x02,0);
		LED_Disp(0x01,1);
	}
	else if(PWM_Mode==1)
	{
		LED_Disp(0x01,0);
		LED_Disp(0x02,1);
	}
	if(LCD_Mode==0)
		LED_Disp(0x08,1);
	else
		LED_Disp(0x08,0);
	
	static u32 tick;
	static u8 count;
	if(uwTick-tick<100) return;
	tick=uwTick;
	count++;
	count%=2;
	
	if(PA4/Y>PA5){
		if(count==1)
			LED_Disp(0x04,1);
		else
			LED_Disp(0x04,0);}
	else
		LED_Disp(0x04,0);
}
void UART()
{
	static u32 tick;
	if(uwTick-tick<50) return;
	tick=uwTick;
	if(rx_count)
	{
		if(strcmp((char*)rx_buff,"X")==0)
			printf("X:%d\r\n",X);
		if(strcmp((char*)rx_buff,"Y")==0)
			printf("Y:%d\r\n",Y);
		if(strcmp((char*)rx_buff,"#")==0)
		{LCD_Mode++;LCD_Mode%=2;}
		
		if(strcmp((char*)rx_buff,"PA1")==0)
			printf("PA1:%d\r\n",PA1);
		if(strcmp((char*)rx_buff,"PA4")==0)
			printf("PA4:%.2f\r\n",PA4);
		if(strcmp((char*)rx_buff,"PA5")==0)
			printf("PA5:%.2f\r\n",PA5);
		
		if(LCD_Mode==0)
		{
			LCD_WriteReg(R1  , 0x0000);
			LCD_WriteReg(R96 , 0x2700);
			LCD_Clear(Black);
		}
		else
		{
			LCD_WriteReg(R1  , 0x0100);
			LCD_WriteReg(R96 , 0xA700);
			LCD_Clear(Black);	
		}
		
		
		u8 i;
		for(i=0;i<rx_count;i++)
			rx_buff[i]=0;
		rx_count=0;
	}
}
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
	u8 date;
	I2CStart();
	I2CSendByte(0xa0);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CStop();
	I2CStart();
	I2CSendByte(0xa1);
	I2CWaitAck();
	date=I2CReceiveByte();
	I2CSendNotAck();
	return date;
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
		if(htim->Channel==HAL_TIM_ACTIVE_CHANNEL_2)
		{
			if(CaptureNum==0)
			{
				GetValue1=TIM2->CCR2;
				CaptureNum=1;
			}
			else if(CaptureNum==1)
			{
				GetValue2=TIM2->CCR2;
				CaptureNum=0;
				
				if(GetValue2>GetValue1)
					GetValue=GetValue2-GetValue1;
				else
					GetValue=0xFF+GetValue2-GetValue1;
				PA1=1000*1000/GetValue;
			}
		}
	}
}
void KEY_Proc()
{
	KEY1();
	if(LCD_Flag==1){KEY2();KEY3();}
	KEY4();
}
void KEY1()
{
	static u32 tick;
	static u8 count;
	if(B1==0)//按下
	{		
		if(uwTick-tick<100) return;
		tick=uwTick;
		count++;
	}
	if(B1==1 & count!=0)
	{
		count=0;
		LCD_Flag++;
		LCD_Flag%=3;
	}
}

void KEY2()
{
	static u32 tick;
	static u8 count;
	if(B2==0)//按下
	{		
		if(uwTick-tick<100) return;
		tick=uwTick;
		count++;
	}
	if(B2==1 & count!=0)
	{
		count=0;
		X++;
		if(X>4)
			X=1;
		EEPROM_Write(0x00,X);
	}
}

void KEY3()
{
	static u32 tick;
	static u8 count;
	if(B3==0)//按下
	{		
		if(uwTick-tick<100) return;
		tick=uwTick;
		count++;
	}
	if(B3==1 & count!=0)
	{
		count=0;
		Y++;
		if(Y>4)
			Y=1;
		EEPROM_Write(0x01,Y);
	}
}

void KEY4()
{
	static u32 tick;
	static u8 count;
	static float x,y;
	if(B4==0)//按下
	{		
		if(uwTick-tick<100) return;
		tick=uwTick;
		count++;
	}
	else if(B4==1 & count!=0 & count<10)
	{
		count=0;
		if(LCD_Flag==0)
		{
			if(N[0]==0)	Min[0]=4;
			if(N[1]==0)	Min[1]=4;

			ADC_Get();
			PA4_value[++N[0]]=PA4;
			PA5_value[++N[1]]=PA5;
			
			if(Max[0]<PA4_value[N[0]])
				Max[0]=PA4_value[N[0]];
			if(Min[0]>PA4_value[N[0]])
				Min[0]=PA4_value[N[0]];
			x+=PA4_value[N[0]];
			Ave[0]=x/N[0];
			
			if(Max[1]<PA5_value[N[1]])
				Max[1]=PA5_value[N[1]];
			if(Min[1]>PA5_value[N[1]])
				Min[1]=PA5_value[N[1]];
			y+=PA5_value[N[1]];
			Ave[1]=y/N[1];
			
		}
		else if(LCD_Flag==1)
		{PWM_Mode++;PWM_Mode%=2;}
		else if(LCD_Flag==2)
		{ADC_Flag++;ADC_Flag%=2;}
	}
	else if(B4==1 & count>10)
	{
		count=0;
		if(LCD_Flag==2)
		{
			if(ADC_Flag==0)
			{
				N[0]=0;
				Max[0]=0;
				Min[0]=0;
				Ave[0]=0;
				x=0;
				u8 i;
				for(i=0;i<101;i++)
					PA4_value[i]=0;
			}
			
			if(ADC_Flag==1)
			{
				N[1]=0;
				Max[1]=0;
				Min[1]=0;
				Ave[1]=0;
				y=0;
				u8 i;
				for(i=0;i<101;i++)
					PA5_value[i]=0;
			}
		}
		
	}
}



void LCD_Proc()
{
	if(LCD_Flag==0)
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"        DATA            ");	
		sprintf((char*)buff,"     PA4=%.2f     ",PA4_value[N[0]]);
		LCD_DisplayStringLine(Line3,buff);	
		sprintf((char*)buff,"     PA5=%.2f     ",PA5_value[N[1]]);
		LCD_DisplayStringLine(Line4,buff);	
		sprintf((char*)buff,"     PA1=%d     ",PA1);
		LCD_DisplayStringLine(Line5,buff);	
		LCD_DisplayStringLine(Line6,(unsigned char *)"                        ");	
	}
	
	if(LCD_Flag==1)
	{
		LCD_DisplayStringLine(Line1,(unsigned char *)"        PARA            ");	
		sprintf((char*)buff,"     X=%d     ",X);
		LCD_DisplayStringLine(Line3,buff);	
		sprintf((char*)buff,"     Y=%d     ",Y);
		LCD_DisplayStringLine(Line4,buff);	
		LCD_DisplayStringLine(Line5,(unsigned char *)"                        ");	
		LCD_DisplayStringLine(Line6,(unsigned char *)"                        ");	

	}
	
	if(LCD_Flag==2)
	{
		if(ADC_Flag==0)
		{
			LCD_DisplayStringLine(Line1,(unsigned char *)"        REC-PA4            ");	
			sprintf((char*)buff,"     N=%d     ",N[0]);
			LCD_DisplayStringLine(Line3,buff);	
			sprintf((char*)buff,"     A=%.2f     ",Max[0]);
			LCD_DisplayStringLine(Line4,buff);	
			sprintf((char*)buff,"     T=%.2f     ",Min[0]);
			LCD_DisplayStringLine(Line5,buff);	
			sprintf((char*)buff,"     H=%.2f     ",Ave[0]);
			LCD_DisplayStringLine(Line6,buff);	
		}
		if(ADC_Flag==1)
		{
			LCD_DisplayStringLine(Line1,(unsigned char *)"        REC-PA5            ");	
			sprintf((char*)buff,"     N=%d     ",N[1]);
			LCD_DisplayStringLine(Line3,buff);	
			sprintf((char*)buff,"     A=%.2f     ",Max[1]);
			LCD_DisplayStringLine(Line4,buff);	
			sprintf((char*)buff,"     T=%.2f     ",Min[1]);
			LCD_DisplayStringLine(Line5,buff);	
			sprintf((char*)buff,"     H=%.2f     ",Ave[1]);
			LCD_DisplayStringLine(Line6,buff);	
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
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
