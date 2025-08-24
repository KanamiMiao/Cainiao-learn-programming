#ifndef _DATA_H_
#define _DATA_H_

int MODE = 0;


//////////////////////////////////////////////////////
//nrf初始化
//////////////////////////////////////////////////////
#include <RF24.h>
#include <nRF24L01.h>
#include <SPI.h>


RF24 radio(26, 25);

const byte addresses[][6] = { "00001", "00002" };

void nrf_Init() {
  radio.begin();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.setChannel(108);
  radio.openReadingPipe(1, addresses[1]);
  radio.openWritingPipe(addresses[0]);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
}
//////////////////////////////////////////////////////
//蓝牙初始化
//////////////////////////////////////////////////////
#include <BluetoothSerial.h>


BluetoothSerial bl;

void BL_Init() 
{
  bl.begin("ESP32_UAV");
}
//////////////////////////////////////////////////////



#include <stdio.h>
#include <string.h>

#include "servo.h"
#include "voltage.h"
#include "MPU.h"


//遥控数据
struct joystick_data 
{
  byte data_head = 0xA5;  //1  //包头
  int8_t pwm[4];          //4  //摇杆输入
  uint8_t pid_choose;     //1  //pid选择
  uint8_t pid_set;        //1  //是否写入pid参数
  uint8_t mode;           //1  //模式选择
  float pid[3];           //12 //pid系数
  byte check;             //1  //校验位
  byte data_tail = 0x5A;  //1  //包尾
};

//无人机数据
struct UAV_data
{
  byte data_head = 0xA5;  //1  //包头
  uint8_t voltage;        //1  //电压
  uint8_t motor[4];       //4  //油门
  float angle[3] ;        //12 //姿态角
  float Vangle[3];        //12 //角速度
  byte check;             //1  //校验位
  byte data_tail = 0x5A;  //1  //包尾
};

class DATA
{
  private:
    uint8_t bl_receive_buf[sizeof(joystick_data)];//接收数组
    uint8_t bl_send_buf[sizeof(UAV_data)];//发送数组

  public:
    UAV_data uav;
    joystick_data joystick;
    
    uint8_t connect_mode;        //模式选择
    void pack(uint8_t* buffer);  //打包数据
    void sendpackit();           //发送数据包
    bool receivepackit();        //接收数据包
};

void DATA::pack(uint8_t* buffer)//打包数据
{

  int index = 0;

  buffer[index++] = uav.data_head;
  buffer[index++] = uav.voltage;

  for (int i = 0; i < 4; i++)
    buffer[index++] = uav.motor[i];

  for (int i = 0; i < 3; i++) {
    memcpy(buffer + index, &uav.angle[i], sizeof(uav.angle[i]));
    index += sizeof(uav.angle[i]);
  }

  for (int i = 0; i < 3; i++) {
    memcpy(buffer + index, &uav.Vangle[i], sizeof(uav.Vangle[i]));
    index += sizeof(uav.Vangle[i]);
  }


  uav.check = 0;
  for (int i = 1; i < index; i++)
    uav.check += buffer[i];
  buffer[index++] = uav.check & 0xFF;

  buffer[index++] = uav.data_tail;
}

void DATA::sendpackit()//发送数据包
{
  pack(bl_send_buf);
  if(MODE == 0)
    bl.write(bl_send_buf, sizeof(uav));
  else
  {
    radio.stopListening();
    radio.write(bl_send_buf, sizeof(uav));
  }
}

bool DATA::receivepackit()//接收数据包
{
  int index = 0; 
  if(MODE == 0)
  {
    while(bl.available() && index < sizeof(joystick_data))
      bl_receive_buf[index++] = bl.read();
  }
  else
  {
    radio.startListening();
    while(bl.available())
      radio.read(&bl_receive_buf,sizeof(joystick_data));
  }

  int sum;
  for(int i = 1 ; i < 19 ; i++)
    sum += bl_receive_buf[i];
  
  if(bl_receive_buf[0] == joystick.data_head && bl_receive_buf[sizeof(joystick) - 1] == joystick.data_tail && sum & 0xff == bl_receive_buf[20])
  {
    int index = 1;
    for(int i=0;i<4;i++)
    {
      memcpy(&joystick.pwm[i], bl_receive_buf + index, 1);
      index++;
    }
    memcpy(&joystick.pid_choose, bl_receive_buf + index, 1);
    index++;
    memcpy(&joystick.pid_set, bl_receive_buf + index, 1);
    index++;
    memcpy(&joystick.mode, bl_receive_buf + index, 1);
    index++;

    for(int i=0;i<3;i++)
    {
      memcpy(&joystick.pid[i], bl_receive_buf + index, 4);
      index += sizeof(joystick.pid[i]);
    }
    MODE = joystick.mode;

    return 1;

  }
}


DATA data;





#endif
