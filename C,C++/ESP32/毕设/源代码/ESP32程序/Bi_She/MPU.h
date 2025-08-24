#ifndef _MPU_H
#define _MPU_H

#include <math.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "data.h"

#define Pi 3.1415926

Adafruit_MPU6050 mpu;
sensors_event_t a,g,t;

uint64_t time1;
float r,p,y;

void MPU_Init()
{
  mpu.begin(0x68,&Wire);
}


void MPU_Get(float K)
{
  mpu.getEvent(&a,&g,&t);
  //用加速度计计算角度
  float p1 = atan2(a.acceleration.y , a.acceleration.z)/Pi*180;
  float r1 = -atan2(a.acceleration.x , sqrt(pow(a.acceleration.y,2)+pow(a.acceleration.z,2) ) )/Pi*180;

  //用陀螺仪计算角度
  if(r==0){r=r1;}
  if(p==0){p=p1;}
  float x1=sin(r*Pi/180)*sin(p*Pi/180)/cos(p*Pi/180);
  float x2=cos(r*Pi/180)*sin(p*Pi/180)/cos(p*Pi/180);
  float x3=cos(r*Pi/180);
  float x4=-sin(r*Pi/180);
  float x5=sin(r*Pi/180)/cos(p*Pi/180);
  float x6=cos(r*Pi/180)/cos(p*Pi/180);

  float dt = (micros() - time1) / 1000000.0f;//计算时间变化量s
  time1 = micros();//获取时间

  float hhh[3] = {(57.3f * g.gyro.y - 0.75), (57.3f * g.gyro.x + 1.35), (57.3f * g.gyro.z + 0.81)};

  float r2 = r + hhh[0] * dt;//Pi rad/s = 180。/s
  float p2 = p + hhh[1] * dt;//Pi rad/s = 180。/s
  float y2 = y + hhh[2] * dt;//Pi rad/s = 180。/s

  r = r2 * K + r1 * (1-K);
  p = p2 * K + p1 * (1-K);
  y = y2;

  data.uav.angle[0] = r;
  data.uav.angle[1] = p;
  data.uav.angle[2] = y;

  data.uav.Vangle[0] = hhh[0];
  data.uav.Vangle[1] = hhh[1];
  data.uav.Vangle[2] = hhh[2];

}


#endif 