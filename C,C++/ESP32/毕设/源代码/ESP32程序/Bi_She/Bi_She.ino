/*
    1     2
    \\   //
     \\ //
     // \\
    //   \\
    3     4
*/

//导入库
#include "task.h"


void setup() 
{
  Serial.begin(115200);//开启串口
  MPU_Init();          //开启mpu6050
  Servo_Init();        //开启电机
  BL_Init();           //开启蓝牙
  nrf_Init();          //开启nrf
  Voltage_Init();      //开启电压读取
  core0_task();        //core 0
}


void loop() 
{
  MPU_Get(0.42);        //core 1
}

