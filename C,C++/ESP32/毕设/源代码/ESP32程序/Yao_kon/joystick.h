#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "nrf.h"

int Joystick1_x = 39, Joystick1_y = 33;
int Joystick1_xval, Joystick1_yval;

int Joystick2_x = 36, Joystick2_y = 32, Joystick2_z = 15;
int Joystick2_xval, Joystick2_yval;


void Joystick_Init()
{
  pinMode(Joystick1_x,INPUT);
  pinMode(Joystick1_y,INPUT);
  pinMode(Joystick2_x,INPUT);
  pinMode(Joystick2_y,INPUT);
  pinMode(Joystick2_z,INPUT_PULLUP);
}

void Joystick_Read()
{
  Joystick1_xval = analogRead(Joystick1_x)*256/4096-127;
  Joystick1_yval = analogRead(Joystick1_y)*256/4096-127;
  Joystick2_xval = analogRead(Joystick2_x)*256/4096-127;
  Joystick2_yval = analogRead(Joystick2_y)*256/4096-127;

  data.joystick.pwm[0] = Joystick1_xval;
  data.joystick.pwm[1] = Joystick1_yval;
  data.joystick.pwm[2] = Joystick2_xval;
  data.joystick.pwm[3] = Joystick2_yval;

  if(digitalRead(Joystick2_z) == 0)
    data.joystick.mode = 0;
}







#endif
