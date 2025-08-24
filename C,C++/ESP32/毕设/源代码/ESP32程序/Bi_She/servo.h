#ifndef _SERVO_H_
#define _SERVO_H_

#include <ESP32Servo.h>

#define max_wide 2000
#define min_wide 1000

#define motor1_pin 33
#define motor2_pin 32
#define motor3_pin 13
#define motor4_pin 27

Servo  servo1;
Servo  servo2;
Servo  servo3;
Servo  servo4;

uint8_t motorOut[4];

void Servo_Init()
{
  // 设置 PWM 信号频率是 50 Hz
  servo1.setPeriodHertz(50); 
  servo2.setPeriodHertz(50); 
  servo3.setPeriodHertz(50); 
  servo4.setPeriodHertz(50); 
  // 设置脉冲宽度范围是 1000 - 2000 微秒  T * duty  
  servo1.attach(motor1_pin, min_wide, max_wide); 
  servo2.attach(motor2_pin, min_wide, max_wide); 
  servo3.attach(motor3_pin, min_wide, max_wide); 
  servo4.attach(motor4_pin, min_wide, max_wide); 
}



#endif
