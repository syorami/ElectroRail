#include "common.h"

/*----------------��������-----------------*/
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

/*----------------�������-----------------*/


void main (void)
{
  PIT_Init();           //PIT��ʼ��
  Qd_Init();            //��ʼ���������빦��
//  PIT_Init();           //��ʼ��PIT��ʱ�ж�
  Steer_Init();         //�����ʼ��
  PID_Init();           //PID��ʼ��
  Motor_Init();         //�����ʼ��
  uart_init();          //�������ڳ�ʼ��
  GPIO_Init();          //GPIO��ʼ��
  
  Motor_Change();       //���ת��
  
  while(1)
  {
  }
}

/*----------------��ʱ����-----------------*/
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
