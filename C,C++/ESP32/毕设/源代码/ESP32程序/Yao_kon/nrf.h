#ifndef _DATA_H_
#define _DATA_H_



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
  radio.openReadingPipe(1, addresses[0]);
  radio.openWritingPipe(addresses[1]);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
}
#include <stdio.h>
#include <string.h>


//遥控数据
struct joystick_data 
{
  byte data_head = 0xA5;  //1  //包头
  int8_t pwm[4];          //4  //摇杆输入
  uint8_t pid_choose;     //1  //pid选择
  uint8_t pid_set=0;        //1  //是否写入pid参数
  uint8_t mode=1;           //1  //模式选择
  float pid[3];           //12 //pid系数
  byte check;             //1  //校验位
  byte data_tail = 0x5A;  //1  //包尾
};

//无人机数据
struct UAV_data
{
  byte data_head = 0xA5;  //1  //包头
  uint8_t voltage = 12;        //1  //电压
  uint8_t motor[4] = {30,29,30,29};       //4  //油门
  float angle[3]={0.135,-0.224,7};        //12 //姿态角
  float Vangle[3]={0.072,0.147,0.789};          //12 //角速度
  byte check;             //1  //校验位
  byte data_tail = 0x5A;  //1  //包尾
};

class DATA
{
  private:
    uint8_t bl_receive_buf[sizeof(UAV_data)];//接收数组
    uint8_t bl_send_buf[sizeof(joystick_data)];//发送数组

  public:
    UAV_data uav;
    joystick_data joystick;
    
    void pack(uint8_t* buffer);  //打包数据
    void sendpackit();           //发送数据包
    bool receivepackit();        //接收数据包
};

void DATA::pack(uint8_t* buffer)//打包数据
{

  int index = 0;

  buffer[index++] = joystick.data_head;

  for (int i = 0; i < 4; i++)
    buffer[index++] = joystick.pwm[i];

  buffer[index++] = joystick.pid_choose;
  buffer[index++] = joystick.pid_set;
  buffer[index++] = joystick.mode;

  for (int i = 0; i < 3; i++) {
    memcpy(buffer + index, &joystick.pid[i], sizeof(joystick.pid[i]));
    index += sizeof(joystick.pid[i]);
  }

  joystick.check = 0;
  for (int i = 1; i < index; i++)
    joystick.check += buffer[i];
  buffer[index++] = joystick.check & 0xFF;

  buffer[index++] = joystick.data_tail;
}

void DATA::sendpackit()//发送数据包
{
  pack(bl_send_buf);
  radio.stopListening();
  radio.write(bl_send_buf, sizeof(joystick));
}

bool DATA::receivepackit()//接收数据包
{
  radio.startListening();
  while(radio.available())
    radio.read(&bl_receive_buf,sizeof(UAV_data));


  int sum;
  for(int i = 1 ; i < 29 ; i++)
    sum += bl_receive_buf[i];

  if(bl_receive_buf[0] == uav.data_head && bl_receive_buf[sizeof(uav) - 1] == uav.data_tail && sum & 0xff == bl_receive_buf[30])
  {
    int index = 1;
    
    memcpy(&uav.voltage, bl_receive_buf + index, 1);
    index++;
    for(int i=0;i<4;i++)
    {
      memcpy(&uav.motor[i], bl_receive_buf + index, 1);
      index++;
    }

    for(int i=0;i<3;i++)
    {
      memcpy(&uav.angle[i], bl_receive_buf + index, 4);
      index += sizeof(uav.angle[i]);
    }
    
    for(int i=0;i<3;i++)
    {
      memcpy(&uav.Vangle[i], bl_receive_buf + index, 4);
      index += sizeof(uav.Vangle[i]);
    }
    
    return 1;

  }
}


DATA data;





#endif
