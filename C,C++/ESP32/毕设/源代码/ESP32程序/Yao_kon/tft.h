#ifndef _TFT_H_
#define _TFT_H_


#include <TFT_eSPI.h>
#include "stdio.h"
#include "joystick.h"
#include "nrf.h"


#define TFT_wide 128
#define TFT_Height 160
#define Block 16


#define Menu 0
#define PID_Config 1
#define Joystick_Disp 2
#define Angle_Disp 3

int Page = Joystick_Disp;
char tft_buf[30];

TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

void TFT_Init()
{
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);

  spr.createSprite(128, 160);
}


void TFT_Disp()
{
  spr.drawString("    Joystick_Disp:    ", 0, 0);
  sprintf(tft_buf, "   x1=%d   y1=%d    ", Joystick1_xval,Joystick1_yval);
  spr.drawString(tft_buf, 0, Block* 1);

  sprintf(tft_buf, "   x2=%d   y2=%d    ", Joystick2_xval,Joystick2_yval);
  spr.drawString(tft_buf, 0, Block* 2);

  tft.setTextColor(TFT_RED);
  sprintf(tft_buf,"======= V=%d ============", data.uav.voltage);
  spr.drawString(tft_buf, 0, Block* 3);
  tft.setTextColor(TFT_WHITE);

  spr.drawString("       UAV_Disp:", 0, Block* 4);
  sprintf(tft_buf, "     m0=%d   m1=%d    ", data.uav.motor[0],data.uav.motor[1]);
  spr.drawString(tft_buf, 0, Block* 5);

  sprintf(tft_buf, "     m2=%d   m3=%d    ", data.uav.motor[2],data.uav.motor[3]);
  spr.drawString(tft_buf, 0, Block* 6);

  sprintf(tft_buf, "   r=%.2f   Vr=%.2f    ", data.uav.angle[0], data.uav.Vangle[0]);
  spr.drawString(tft_buf, 0, Block* 7);
  
  sprintf(tft_buf, "   p=%.2f   Vp=%.2f    ", data.uav.angle[1], data.uav.Vangle[1]);
  spr.drawString(tft_buf, 0, Block* 8);
  
  sprintf(tft_buf, "   y=%.2f   Vy=%.2f    ", data.uav.angle[3], data.uav.Vangle[3]);
  spr.drawString(tft_buf, 0, Block* 9);
  
  spr.pushSprite(0, 0);
}




#endif
