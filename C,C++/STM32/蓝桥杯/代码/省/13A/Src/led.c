#include "led.h"


void LED_Init()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_All, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
}

void LED_On(unsigned char led_num)
{
	if(led_num==1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	}
	if(led_num==2)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);
	}
}
