#include "common.h"

GPIO_InitTypeDef GPIO_Init_struct;

void GPIO_Init(void)
{
  GPIO_Init_struct.GPIO_PTx=PTD;                        //ѡ��PORT
  GPIO_Init_struct.GPIO_Pins=GPIO_Pin0;                 //ѡ��Ҫ��ʼ��������
  GPIO_Init_struct.GPIO_Dir=DIR_OUTPUT;                 //ѡ������ģʽ
  GPIO_Init_struct.GPIO_Output=OUTPUT_L;                //ѡ�������������½��ز���DMA���� 
  LPLD_GPIO_Init(GPIO_Init_struct);                             //��ʼ����ӦGPIO��
}