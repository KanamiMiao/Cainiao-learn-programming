#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "stdio.h"
#include "i2c_hal.h"

//全局变量
unsigned char buf[30];
u8 flag=1;
u8 room=1;
u8 hour,min,sec;
u8 change_flag=0;
u8 time_num;

u32 key_stick;
u8 KEY2_Count,KEY4_Count;
u8 xiaochu=0;//消除突出

u32 jishi_hour,jishi_min,jishi_sec;
u8 qidon=0;//启动运行
u32 daojishi;
//函数定义
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
void KEY1()
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0)
	{
		HAL_Delay(10);
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0)==0);
		HAL_Delay(10);
		room++;
		flag=1;
		change_flag=0;
		if(room==6){room=1;}
	}
}
void KEY2()
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==0)//按下B2，计数
	{
		if(uwTick-key_stick<100) return;
		key_stick=uwTick;
		KEY2_Count++;
	}
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1)==1)//松开B2，查看模式
	{
		if(KEY2_Count>8)//>8，长按
		{
			EEPROM_Write(room*3-2,hour);
			EEPROM_Write(room*3-1,min);
			EEPROM_Write(room*3-0,sec);
			flag=2;
			KEY2_Count=0;
		}
		else if(KEY2_Count!=0)//<8，短按，切换位选择
		{
			change_flag++;
			KEY2_Count=0;
			xiaochu=1;
			time_num=0;
			if(change_flag==5){change_flag=2;}
		}
	}
}

void KEY3()
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2)==0)//按下B3
	{
		HAL_Delay(150);
		time_num++;
	}	
}

void KEY4()
{
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==0)//按下B4，计数
	{
		if(uwTick-key_stick<100) return;
		key_stick=uwTick;
		KEY4_Count++;
	}
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==1)//松开B4，查看模式
	{
		if(KEY4_Count>8)//>8，长按
		{
			KEY4_Count=0;
			qidon=0;
			flag=1;		
			change_flag=0;
		}
		else if(KEY4_Count!=0)//<8，短按，切换位选择
		{
			KEY4_Count=0;
			flag=2;
			if(qidon==0){qidon=1;}
			else{qidon=0;}
		}
	}
}
void LCD_Standby_AND_Setting()
{
	KEY2();	
	if(change_flag==0)//显示Standby初始值
	{
		EEPROM_Read(room*3-2);
		EEPROM_Read(room*3-1);
		EEPROM_Read(room*3-0);
		hour=EEPROM_Read(room*3-2);
		min=EEPROM_Read(room*3-1);
		sec=EEPROM_Read(room*3-0);
		sprintf((char*)buf,"  No %d  ",room);
		LCD_DisplayStringLine(Line2,buf);	
		sprintf((char*)buf,"      %02d:%02d:%02d",hour,min,sec);
		LCD_DisplayStringLine(Line5,buf);	
		sprintf((char*)buf,"      Standby       ");
		LCD_DisplayStringLine(Line8,buf);	
	}
	
	if((change_flag==1)||(xiaochu==1))//显示Setting初始值
	{
		xiaochu=0;//复原
		sprintf((char*)buf,"  No %d  ",room);
		LCD_DisplayStringLine(Line2,buf);	
		sprintf((char*)buf,"      %02d:%02d:%02d",hour,min,sec);
		LCD_DisplayStringLine(Line5,buf);	
		sprintf((char*)buf,"      Setting       ");
		LCD_DisplayStringLine(Line8,buf);	
	}

	if(change_flag==2)//change_flag==2,修改时
	{
		if(hour!=0){time_num=hour;}//防止hour被归零
		KEY3();
		hour=time_num;
		if(hour>24){hour=0;time_num=0;}
		LCD_SetTextColor(Red);
		LCD_DisplayChar(Line5,320-16*6,hour/10+'0');	
		LCD_DisplayChar(Line5,320-16*7,hour%10+'0');	
		LCD_SetTextColor(White);
	}
	
	if(change_flag==3)//change_flag==3,修改分
	{
		if(min!=0){time_num=min;}//防止mid被归零
		KEY3();
		min=time_num;
		if(min>60){min=0;time_num=0;}
		LCD_SetTextColor(Red);
		LCD_DisplayChar(Line5,320-16*9,min/10+'0');	
		LCD_DisplayChar(Line5,320-16*10,min%10+'0');	
		LCD_SetTextColor(White);
	}
	
	if(change_flag==4)//change_flag==4,修改秒
	{
		if(sec!=0){time_num=sec;}//防止sec被归零
		KEY3();
		sec=time_num;
		if(sec>60){sec=0;time_num=0;}
		LCD_SetTextColor(Red);
		LCD_DisplayChar(Line5,320-16*12,sec/10+'0');	
		LCD_DisplayChar(Line5,320-16*13,sec%10+'0');	
		LCD_SetTextColor(White);
	}
	
}
void LCD_Running()//倒计时运行
{
	if(qidon==0)
	{
		sprintf((char*)buf,"  No %d  ",room);//倒计时界面
		LCD_DisplayStringLine(Line2,buf);	
		sprintf((char*)buf,"      %02d:%02d:%02d",hour,min,sec);
		LCD_DisplayStringLine(Line5,buf);	
		sprintf((char*)buf,"      Running       ");
		LCD_DisplayStringLine(Line8,buf);	
		LED1();
		
		if(uwTick-daojishi<1000) return;
		daojishi=uwTick;
		sec--;
		if(sec==255)
		{
			sec=59;
			min--;
			if(min==255)
			{
				min=59;
				hour--;
				if(hour==255){hour=23;}
			}
		}
	}
	if(qidon==1)////停止时界面显示Pause
	{
		sprintf((char*)buf,"  No %d  ",room);
		LCD_DisplayStringLine(Line2,buf);	
		sprintf((char*)buf,"      %02d:%02d:%02d",hour,min,sec);
		LCD_DisplayStringLine(Line5,buf);	
		sprintf((char*)buf,"       Pause        ");
		LCD_DisplayStringLine(Line8,buf);	
	}
}
void SystemClock_Config(void);


int main(void)
{
	
	//初始化
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM3_Init();
	LED_Init();
	LCD_Init();
	I2CInit();
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	
	LCD_Clear(Black);
	LCD_SetBackColor(Black);
	LCD_SetTextColor(White);
//	LCD_DisplayStringLine(Line0,(unsigned char *)"                    ");
  while (1)
  {
		KEY1();
		KEY4();
		if(flag==1){LCD_Standby_AND_Setting();}
		if(flag==2){LCD_Running();}
		
		sprintf((char*)buf,"    %d %d %d     ",EEPROM_Read(room*3-2),EEPROM_Read(room*3-1),EEPROM_Read(room*3-0));
		LCD_DisplayStringLine(Line3,buf);	
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
}

void Error_Handler(void)
{
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
