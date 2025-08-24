#include "nrf.h"
#include "joystick.h"
#include "tft.h"



void setup() 
{
  Serial.begin(115200);
  TFT_Init();
  Joystick_Init();
}

void loop() 
{
  Joystick_Read();
  TFT_Disp();
}


// #include <RF24.h>
// #include <nRF24L01.h>
// #include <SPI.h>

// RF24 radio(26, 25);
// const byte address[5] = {'0', '0', '0', '1', '8'}; // 5字节地址

// void setup() {
//   Serial.begin(115200);
//   radio.begin();
//   radio.openWritingPipe(address);
//   radio.setPALevel(RF24_PA_MAX); // 提高功率
//   radio.setChannel(108);         // 设置高频通道
//   radio.stopListening();
//   radio.printDetails(); // 打印配置
// }

// void loop() {
//   const char text[] = "Hello World";
//   bool success = radio.write(&text, sizeof(text));
  
//   if (success) {
//     Serial.println("发送成功");
//   } else {
//     Serial.println("发送失败");
//   }
//   delay(1000);
// }