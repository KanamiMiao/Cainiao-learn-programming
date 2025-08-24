//头文件

#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "i2c_hal.h"


//全局变量
unsigned char buf[30];
u8 Threshold1=30,Threshold2=50,Threshold3=70;//初始阈值
unsigned char Height,Level;
float ADC;
u32 ADC_Tick;u32 LED_Tick;//ADC计时
u8 choose=1;//突出选择
u8 flag=1;
u8 flag1=0,flag2=0,flag3=0;
u8 count1;
unsigned char buff;
u8 lv_view,lv_change;

u8 rx_buf[30],rx_date[2],rx_count;
//定义函数
void LED1()
{		
	if(flag1==1)
	{
		LED_Init();
		LED_On(1);
		if(uwTick-LED_Tick<1000) return;
		LED_Tick=uwTick;
		flag1=0;
		LED_Init();
	}
	else
	{	
		if(uwTick-LED_Tick<1000) return;
		LED_Tick=uwTick;
		flag1=1;
	}
	
}

void LED2()
{			
	if(count1==5) return;
	if(flag2==1)
	{
		LED_Init();
		LED_On(2);
		if(uwTick-LED_Tick<200) return;
		LED_Tick=uwTick;
		flag2=0;
		count1++;
		LED_Init();
	}
	else
	{	
		if(uwTick-LED_Tick<200) return;
		LED_Tick=uwTick;
		flag2=1;
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
void Level_Num()//液位等级
{
	if(Height<=Threshold1){Level=0;}
	else if(Height<=Threshold2){Level=1;}
	else if(Height<=Threshold3){Level=2;}
	if(Height>Threshold3){Level=3;}
	if(lv_change>Level){lv_view=1;}
	if(lv_change<Level){lv_view=2;}
	lv_change=Level;
}
void ADC_Read()//ADC读取
{
	if(uwTick-ADC_Tick<1000) return;//ADC计时1s
	ADC_Tick=uwTick;
	HAL_ADC_Start(&hadc2);
	ADC=HAL_ADC_GetValue(&hadc2)*3.3f/4095.0f;
	Height=ADC*100/3.3;
}
void Section1()//界面1
{
	ADC_Read();
	Level_Num();
	LCD_DisplayStringLine(Line4,(unsigned char *)"                    ");
	LCD_DisplayStringLine(Line6,(unsigned char *)"                    ");
	LCD_DisplayStringLine(Line8,(unsigned char *)"                    ");		
	//覆盖界面2

	LCD_SetTextColor(White);
	sprintf((char*)buf,"    Liquid Level     ");
	LCD_DisplayStringLine(Line1,buf);
	sprintf((char*)buf,"  Height:%dcm        ",Height);
	LCD_DisplayStringLine(Line3,buf);
	sprintf((char*)buf,"  ADC:%.2fV          ",ADC);
	LCD_DisplayStringLine(Line5,buf);
	sprintf((char*)buf,"  Level:%d           ",Level);
	LCD_DisplayStringLine(Line7,buf);
	
	if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0))//按下B1切换界面2
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0));
		HAL_Delay(20);
		flag=2;
	}
	
	if(buff!=Level) return;
	LED2();
	buff=Level;
}

void Section2()//界面2
{
	if(choose==1)
	{
		LCD_DisplayStringLine(Line3,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line5,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line7,(unsigned char *)"                    ");		
		//覆盖界面1
		
		LCD_SetTextColor(White);
		sprintf((char*)buf,"  Parameter Setup     ");
		LCD_DisplayStringLine(Line1,buf);
		
		LCD_SetTextColor(Green);
		sprintf((char*)buf," Threshold 1: %dcm    ",Threshold1);
		LCD_DisplayStringLine(Line4,buf);
		
		LCD_SetTextColor(White);
		sprintf((char*)buf," Threshold 2: %dcm    ",Threshold2);
		LCD_DisplayStringLine(Line6,buf);
		
		LCD_SetTextColor(White);
		sprintf((char*)buf," Threshold 3: %dcm    ",Threshold3);
		LCD_DisplayStringLine(Line8,buf);
		
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)&&(Threshold1!=95))  //+5
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
			HAL_Delay(20);
			Threshold1+=5;
		}
		
		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)&&(Threshold1!=5))   //-5
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);
			HAL_Delay(20);
			Threshold1-=5;
		}		
	}
	
	if(choose==2)
	{
		LCD_DisplayStringLine(Line3,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line5,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line7,(unsigned char *)"                    ");		
		//覆盖界面1
		
		LCD_SetTextColor(White);
		sprintf((char*)buf,"  Parameter Setup     ");
		LCD_DisplayStringLine(Line1,buf);
		
		LCD_SetTextColor(White);
		sprintf((char*)buf," Threshold 1: %dcm    ",Threshold1);
		LCD_DisplayStringLine(Line4,buf);
		
		LCD_SetTextColor(Green);
		sprintf((char*)buf," Threshold 2: %dcm    ",Threshold2);
		LCD_DisplayStringLine(Line6,buf);
		
		LCD_SetTextColor(White);
		sprintf((char*)buf," Threshold 3: %dcm    ",Threshold3);
		LCD_DisplayStringLine(Line8,buf);
		
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)&&(Threshold2!=95))  //+5
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
			HAL_Delay(20);
			Threshold2+=5;
		}
		
		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)&&(Threshold2!=5))   //-5
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);
			HAL_Delay(20);
			Threshold2-=5;
		}		
	}
	
	if(choose==3)
	{
		LCD_DisplayStringLine(Line3,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line5,(unsigned char *)"                    ");
		LCD_DisplayStringLine(Line7,(unsigned char *)"                    ");		
		//覆盖界面1
		
		LCD_SetTextColor(White);
		sprintf((char*)buf,"  Parameter Setup     ");
		LCD_DisplayStringLine(Line1,buf);
		
		LCD_SetTextColor(White);
		sprintf((char*)buf," Threshold 1: %dcm    ",Threshold1);
		LCD_DisplayStringLine(Line4,buf);
		
		LCD_SetTextColor(White);
		sprintf((char*)buf," Threshold 2: %dcm    ",Threshold2);
		LCD_DisplayStringLine(Line6,buf);
		
		LCD_SetTextColor(Green);
		sprintf((char*)buf," Threshold 3: %dcm    ",Threshold3);
		LCD_DisplayStringLine(Line8,buf);
		
		if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)&&(Threshold3!=95))  //+5
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0);
			HAL_Delay(20);
			Threshold3+=5;
		}
		
		if((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)&&(Threshold3!=5))   //-5
		{
			HAL_Delay(20);
			while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0);
			HAL_Delay(20);
			Threshold3-=5;
		}		
	}
	
	if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0))//按下B2,切换选项
	{
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0);
		HAL_Delay(20);
		choose++;
		if(choose==4){choose=1;}
	}
	
	if((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0))//按下B1切换界面1
	{
		HAL_Delay(20);
		while((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0));
		HAL_Delay(20);
		
		flag=1;
		
		EEPROM_Write(1,Threshold1);
		EEPROM_Write(2,Threshold2);
		EEPROM_Write(3,Threshold3);
		//EEPROM写入
	}
}

int fputc(int ch, FILE *f) 
{
  HAL_UART_Transmit(&huart1,(u8*)&ch,1,50);
  return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(&huart1,rx_date,1);
	rx_buf[rx_count++]=rx_date[0];
}
void RX()
{
	static u32 tick;
	if(uwTick-tick<50) return;
	tick=uwTick;
	if(rx_date[0]=='C' & rx_count==1)
	{
		printf("C:H%d+L%d\r\n",Height,Level);
		rx_count=0;
	}
	else if(rx_date[0]=='S' & rx_count==1)
	{
		printf("S:TL%d+TM%d+TH%d\r\n",Threshold1,Threshold2,Threshold3);
		rx_count=0;
	}
	else if(lv_view!=0)
	{
		if(lv_view==1)printf("A:H%d+L%d+D\r\n",Height,Level);
		if(lv_view==2)printf("A:H%d+L%d+U\r\n",Height,Level);
		lv_view=0;
	}
	else rx_count=0;
}
void SystemClock_Config(void);


int main(void)
{
	//初始化
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC2_Init();
  MX_USART1_UART_Init();
	LCD_Init();
	LED_Init();
	I2CInit();
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
	

	EEPROM_Read(1);
	EEPROM_Read(2);
	EEPROM_Read(3);
	//EEPROM上电读取
//	sprintf((char*)buf,"%d   %d   %d  ",EEPROM_Read(1),EEPROM_Read(2),EEPROM_Read(3));
//	LCD_DisplayStringLine(Line9,buf);	

	printf("Hello World\r\n");
	
	HAL_UART_Receive_IT(&huart1,rx_date,1);

  while (1)
  {	
		if(flag==1){Section1();LED1();}
		else {Section2();}
		RX();
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
