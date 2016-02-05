#include "common.h"

/*----------------函数声明-----------------*/
void Qd_Init(void);
void PIT_Init(void);
void Steer_Init(void);
void delay(uint16);
void Steer_Change(void);
void PID_Init(void);
void Motor_Init(void);
void Motor_Change(void);
void uart_init(void);
void vcan_sendware(uint8 *wareaddr, uint32 waresize);
void DMA_Count_Init(void);
void GPIO_Init(void);

/*----------------定义变量-----------------*/


void main (void)
{
  PIT_Init();           //PIT初始化
  Qd_Init();            //初始化正交解码功能
//  PIT_Init();           //初始化PIT定时中断
  Steer_Init();         //舵机初始化
  PID_Init();           //PID初始化
  Motor_Init();         //电机初始化
  uart_init();          //蓝牙串口初始化
  GPIO_Init();          //GPIO初始化
  
  Motor_Change();       //电机转动
  
  while(1)
  {
  }
}

/*----------------延时函数-----------------*/
void delay(uint16 n)
{
  uint16 i;
  while(n--)
  {
    for(i=0; i<5000; i++)
    {
      asm("nop");
    }
  }
}
