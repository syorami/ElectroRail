#include "common.h"

GPIO_InitTypeDef GPIO_Init_struct;

void GPIO_Init(void)
{
  GPIO_Init_struct.GPIO_PTx=PTD;                        //选择PORT
  GPIO_Init_struct.GPIO_Pins=GPIO_Pin0;                 //选择要初始化的引脚
  GPIO_Init_struct.GPIO_Dir=DIR_OUTPUT;                 //选择输入模式
  GPIO_Init_struct.GPIO_Output=OUTPUT_L;                //选择输入上拉、下降沿产生DMA请求 
  LPLD_GPIO_Init(GPIO_Init_struct);                             //初始化对应GPIO口
}