#include "common.h"

/*----------------定义变量-----------------*/
float qd_result_1 = 0;  //记录10毫秒内左边线圈的脉冲数
float qd_result_2 = 0;  //记录10毫秒内右边线圈的脉冲数
float qd_result = 0;    //10毫秒内两边线圈脉冲数的差值   

int32_t SteerVal;
int32_t SteerValMeasureArray[5] = {0};
int32_t SteerValMeasure;
int32_t SteerValTemp;
int32_t a,b,c,d,a1,b1;

FTM_InitTypeDef ftm_init_struct;
PIT_InitTypeDef pit_init_struct;
LPTMR_InitTypeDef lptmr_init_struct;

void Qd_Init(void);
void PIT_isr(void);
void PIT_Init(void);
void DMA_Count_Init(void);
extern void Steer_PID_Calculate(void);
extern void Steer_Limit(void);
extern void Motor_Change(void);
extern void Steer_Change(void);
extern void Out_Data(void);

/*----------------舵机初始化-----------------*/
void Qd_Init(void)
{
  ftm_init_struct.FTM_Ftmx = FTM2;                      //只有FTM1和FTM2有正交解码功能
  ftm_init_struct.FTM_Mode = FTM_MODE_QD;               //正交解码功能
  ftm_init_struct.FTM_QdMode = QD_MODE_CNTDIR;          //计数和方向解码模式 
  ftm_init_struct.FTM_ClkDiv = FTM_CLK_DIV64;          //128分频

  LPLD_FTM_Init(ftm_init_struct);                       //初始化FTM
  LPLD_FTM_QD_Enable(FTM2, PTA10, PTA11);               //PTA10引脚接A相输入、PTA11引脚接B相输入
  
  lptmr_init_struct.LPTMR_Mode = LPTMR_MODE_PLACC;      //脉冲计数模式
  lptmr_init_struct.LPTMR_PluseAccInput = LPTMR_ALT2;   //选择ALT2通道，PTC5
  lptmr_init_struct.LPTMR_Isr = NULL;
  LPLD_LPTMR_Init(lptmr_init_struct);                   //初始化LPTMR     
  
  DMA_Count_Init();
}

/*----------------PIT中断函数-----------------*/
void PIT_isr(void)
{
  a1 = LPLD_FTM_GetCounter(FTM2);        //通道FTM2的脉冲值
  b1 = LPLD_LPTMR_GetPulseAcc();         //LPTMR记录的脉冲值
  c = DMA_count_get(DMA_CH0) * 5;           //DMA记录的脉冲值
  d = DMA_count_get(DMA_CH5) * 5;           //DMA记录的脉冲值
//  a1 = DMA_count_get(DMA_CH1) ;
  
  a = a1;
  b = b1;

  SteerValMeasure = (a1 - b1) * 5 / 4;
  if(SteerValMeasure < 3 && SteerValMeasure > -3)
    SteerValMeasure = 0;
  else
    if(SteerValMeasure >= 3)
      SteerValMeasure -= 3;
    else
      if(SteerValMeasure <= -3)
        SteerValMeasure +=3;
  
  Steer_PID_Calculate();       
  Steer_Limit();
  
  Steer_Change();                             //改变舵机转向
  
  Motor_Change();
  
  Out_Data();                                   //通过蓝牙发送数据
  
  LPLD_FTM_ClearCounter(FTM2);                  //清空计数器FTM2  
  LPLD_LPTMR_ResetCounter();                    //清空LPTMR
  DMA_count_reset(DMA_CH0);                     //清空DMA记录的脉冲值
  DMA_count_reset(DMA_CH5);                     //清空DMA记录的脉冲值
  DMA_count_reset(DMA_CH1);                     //清空DMA记录的脉冲值

  
  LPLD_LPTMR_Init(lptmr_init_struct);

}

/*----------------PIT定时器初始化-----------------*/
void PIT_Init(void)
{
  pit_init_struct.PIT_Pitx = PIT0;      //配置PIT0定时周期中断参数
  pit_init_struct.PIT_PeriodMs = 10;    //10毫秒产生一起中断，记录脉冲数
  pit_init_struct.PIT_Isr = PIT_isr;
  
  LPLD_PIT_Init(pit_init_struct);       //初始化PIT
  
  LPLD_PIT_EnableIrq(pit_init_struct);  //使能PIT定时中断
}

/*----------------DMAh & GPIO初始化-----------------*/
void DMA_Count_Init(void)
{
  static GPIO_InitTypeDef DMA_GPIO;
  
  DMA_GPIO.GPIO_PTx=PTA;                                //选择PORT
  DMA_GPIO.GPIO_Pins=GPIO_Pin7;                        //选择要初始化的引脚
  DMA_GPIO.GPIO_Dir=DIR_INPUT;                          //选择输入模式
  DMA_GPIO.GPIO_PinControl=INPUT_PULL_UP|IRQC_DMAFA;    //选择输入上拉、下降沿产生DMA请求 
  LPLD_GPIO_Init(DMA_GPIO);                             //初始化对应GPIO口
  DMA_count_Init(DMA_CH0, PTA7,0x7FFF);                //DMA脉冲计数初始化

  DMA_GPIO.GPIO_PTx=PTB;                                //选择PORT
  DMA_GPIO.GPIO_Pins=GPIO_Pin18;                        //选择要初始化的引脚
  DMA_GPIO.GPIO_Dir=DIR_INPUT;                          //选择输入模式
  DMA_GPIO.GPIO_PinControl=INPUT_PULL_UP|IRQC_DMAFA;    //选择输入上拉、下降沿产生DMA请求 
  LPLD_GPIO_Init(DMA_GPIO);                             //初始化对应GPIO口
  DMA_count_Init(DMA_CH5, PTB18,0x7FFF);                //DMA脉冲计数初始化

//  DMA_GPIO.GPIO_PTx=PTE;                                //选择PORT
//  DMA_GPIO.GPIO_Pins=GPIO_Pin26;                        //选择要初始化的引脚
//  DMA_GPIO.GPIO_Dir=DIR_INPUT;                          //选择输入模式
//  DMA_GPIO.GPIO_PinControl=INPUT_PULL_UP|IRQC_DMAFA;    //选择输入上拉、下降沿产生DMA请求 
//  LPLD_GPIO_Init(DMA_GPIO);                             //初始化对应GPIO口
//  DMA_count_Init(DMA_CH1, PTE26,0x7FFF);                //DMA脉冲计数初始化
}
