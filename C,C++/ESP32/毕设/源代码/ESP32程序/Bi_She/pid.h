#ifndef _PID_H_
#define _PID_H_

class PID
{
  private:
   int limit_P;
   int limit_I;
   int limit_D;
   float integral = 0;
   float last_error = 0;

  public:
    float Kp;
    float Ki;
    float Kd;
    PID(float p, float i, float d, float lp, float li, float ld);
    void write_pid(float p, float i, float d);
    int calculate(float point, float measure, float dt);//pid计算,(目标值，测量值，时间变化量)
};

PID::PID(float p, float i, float d, float lp, float li, float ld)
{
  Kp = p;
  Ki = i;
  Kd = d;
  limit_P = lp;
  limit_I = li;
  limit_D = ld;
}

void PID::write_pid(float p, float i, float d)//写入pid
{
  Kp = p;
  Ki = i;
  Kd = d;
}

int PID::calculate(float point, float measure, float dt)//pid计算
{
  float error = measure - point;//误差
  float proportion_out = error * Kp;//比例P
  proportion_out = constrain(proportion_out, -limit_P, limit_P);//约束比例P

  integral += (error / 1000 * dt);
  integral = constrain(integral, (float)-limit_I/Ki, (float)limit_I/Ki);//约束I
  float integral_out = integral * Ki;//积分I

  float derivative_out = Kd* (error - last_error)/ dt;
  derivative_out = constrain(derivative_out, -limit_D, limit_D);//约束D

  last_error = error;

  float pid_out = proportion_out + integral_out + derivative_out;
  return pid_out;
}



#endif