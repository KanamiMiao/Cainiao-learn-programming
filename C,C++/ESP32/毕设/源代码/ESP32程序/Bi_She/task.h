#ifndef _TASK_H_
#define _TASK_H_

#include "MPU.h"
#include "data.h"
#include "servo.h"
#include "voltage.h"
#include "pid.h"

#define maxPID_out 100
PID pid_r(0.9f, 1.5f, 40.0f, 250, 100, 200);
PID pid_p(0.9f, 1.5f, 40.0f, 250, 100, 200);
PID pid_y(4.5f, 0.0f, 0.00f, 250, 100, 200);

PID pid_Vr(1.6f, 1.5f, 40.0f, 100, 100, 100);
PID pid_Vp(1.6f, 1.5f, 40.0f, 100, 100, 100);
PID pid_Vy(4.5f, 0.0f, 0.00f, 100, 100, 100);

int motor_out[4] = {0,0,0,0};

////////////////////////////////////////////////
//通讯任务
////////////////////////////////////////////////
void task_data(void* parameters)
{
  for(;;)
  {
    data.sendpackit();//发送数据包
    data.receivepackit();//接收数据包
    vTaskDelay(5);
  }
}
////////////////////////////////////////////////
//pid写入任务
////////////////////////////////////////////////
void task_pid(void* parameters)
{
  for(;;)
  {
    if(data.joystick.pid_set)//打开pid写入
    {
      switch(data.joystick.pid_choose)//选择pid实体
      {
        case 0: pid_r.write_pid(data.joystick.pid[0], data.joystick.pid[1], data.joystick.pid[2]);
        case 1: pid_p.write_pid(data.joystick.pid[0], data.joystick.pid[1], data.joystick.pid[2]);
        case 2: pid_y.write_pid(data.joystick.pid[0], data.joystick.pid[1], data.joystick.pid[2]);
        case 3: pid_Vr.write_pid(data.joystick.pid[0], data.joystick.pid[1], data.joystick.pid[2]);
        case 4: pid_Vp.write_pid(data.joystick.pid[0], data.joystick.pid[1], data.joystick.pid[2]);
        case 5: pid_Vy.write_pid(data.joystick.pid[0], data.joystick.pid[1], data.joystick.pid[2]);
      }
    }
    vTaskDelay(20);
  }
}
////////////////////////////////////////////////
//电机任务
////////////////////////////////////////////////
void task_motor(void* parameters)
{
  for(;;)
  {
    servo1.writeMicroseconds(motor_out[0]);
    servo2.writeMicroseconds(motor_out[1]);
    servo3.writeMicroseconds(motor_out[2]);
    servo4.writeMicroseconds(motor_out[3]);

    for (int i = 0; i < 4; i++)//存入uav
      data.uav.motor[i] = map(motor_out[i], min_wide, max_wide, 0, 100);

    vTaskDelay(3);
  }
}
////////////////////////////////////////////////
//控制任务
////////////////////////////////////////////////
void task_control(void* parameters)
{
  uint8_t DT = 3;
  for(;;)
  {
    //目标角度
    float point_r = map(data.joystick.pwm[2], -126, 127, -30, 30);//x2
    float point_p = map(data.joystick.pwm[3], -126, 127, -30, 30);//y2
    float point_y = map(data.joystick.pwm[0], -126, 127, -30, 30);//x1

    //pid输出
    int pid_out_angle[3];
    pid_out_angle[0] = pid_r.calculate(point_r, data.uav.angle[0], DT);
    pid_out_angle[1] = pid_p.calculate(point_p, data.uav.angle[1], DT);
    pid_out_angle[2] = pid_y.calculate(point_y, data.uav.angle[2], DT);

    int pid_out_Vangle[3];
    pid_out_Vangle[0] = pid_r.calculate(pid_out_angle[0], data.uav.Vangle[0], DT);
    pid_out_Vangle[1] = pid_p.calculate(pid_out_angle[1], data.uav.Vangle[1], DT);
    pid_out_Vangle[2] = pid_y.calculate(pid_out_angle[2], data.uav.Vangle[2], DT);


    int pid_motor_out[4];
    pid_motor_out[0] = 0 + pid_out_Vangle[0] - pid_out_Vangle[1] - pid_out_Vangle[2];//左前
    pid_motor_out[1] = 0 - pid_out_Vangle[0] - pid_out_Vangle[1] + pid_out_Vangle[2];//右前
    pid_motor_out[2] = 0 + pid_out_Vangle[0] + pid_out_Vangle[1] + pid_out_Vangle[2];//左后
    pid_motor_out[3] = 0 - pid_out_Vangle[0] + pid_out_Vangle[1] - pid_out_Vangle[2];//右后
    for(int i = 0; i < 4; i++)
      pid_motor_out[i] = constrain(pid_motor_out[i], -maxPID_out, maxPID_out);

    //获取油门基础转速
    int base_motor = map(data.joystick.pwm[1], -126, 127, min_wide, max_wide);
    //计算油门输出
    for(int i = 0; i < 4; i++)
    {
      motor_out[i] = base_motor + pid_motor_out[i];
      motor_out[i] = constrain(motor_out[i], min_wide, max_wide);
    }

    if(data.joystick.pid_set)
    {
      for(int i = 0; i < 4; i++)
      motor_out[i] = min_wide;
    }
    
    vTaskDelay(DT);
  }
}
////////////////////////////////////////////////
//电压
////////////////////////////////////////////////
void task_voltage(void* parameters)
{
  for(;;)
  {
    Voltage_Get();
    int vvv = v1s + v2s +v3s;
    data.uav.voltage =vvv;
    vTaskDelay(1000);
  }
}

//创建任务句柄
TaskHandle_t TaskHandle_data;
TaskHandle_t TaskHandle_pid;
TaskHandle_t TaskHandle_motor;
TaskHandle_t TaskHandle_control;
TaskHandle_t TaskHandle_voltage;

void core0_task()
{
  //(任务函数，任务名称，堆栈大小，参数，优先级，句柄，核心）
  xTaskCreatePinnedToCore(task_data, "task_data", 1000, NULL, 1, &TaskHandle_data, 0);
  xTaskCreatePinnedToCore(task_pid, "task_pid", 1000, NULL, 4, &TaskHandle_pid, 0);
  xTaskCreatePinnedToCore(task_motor, "task_motor", 1000, NULL, 2, &TaskHandle_motor, 0);
  xTaskCreatePinnedToCore(task_control, "task_control", 1000, NULL, 3, &TaskHandle_control, 0);
  xTaskCreatePinnedToCore(task_voltage, "task_voltage", 1000, NULL, 5, &TaskHandle_voltage, 0);
}

#endif