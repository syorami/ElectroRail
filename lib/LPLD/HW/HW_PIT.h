/**
 * @file HW_PIT.h
 * @version 3.0[By LPLD]
 * @date 2013-06-18
 * @brief PIT底层模块相关函数
 *
 * 更改建议:不建议修改
 *
 * 版权所有:北京拉普兰德电子技术有限公司
 * http://www.lpld.cn
 * mail:support@lpld.cn
 *
 * @par
 * 本代码由拉普兰德[LPLD]开发并维护，并向所有使用者开放源代码。
 * 开发者可以随意修使用或改源代码。但本段及以上注释应予以保留。
 * 不得更改或删除原版权所有者姓名，二次开发者可以加注二次版权所有者。
 * 但应在遵守此协议的基础上，开放源代码、不得出售代码本身。
 * 拉普兰德不负责由于使用本代码所带来的任何事故、法律责任或相关不良影响。
 * 拉普兰德无义务解释、说明本代码的具体原理、功能、实现方法。
 * 除非拉普兰德[LPLD]授权，开发者不得将本代码用于商业产品。
 */
#ifndef __HW_PIT_H__
#define __HW_PIT_H__
/********************************************************************/

//PIT模块中断回调函数类型
typedef void (*PIT_ISR_CALLBACK)(void);

//PIT模块号定义
typedef enum PITx
{
  PIT0=0,
  PIT1=1,
  PIT2=2,
  PIT3=3
}PITx;

//PIT模块初始化结构体，用于配置PIT各项参数
typedef struct
{  
  /*
    描述：
      选择PITx
    取值：
      PIT0、PIT1、PIT2、PIT3
    初始化：
      必须初始化
  */
  PITx PIT_Pitx;
  
  /*
    描述：
      PITx模块的周期，单位us
    取值：
      1~0xFFFFFFFF
    初始化：
      不必须初始化，PIT_PeriodUs、PIT_PeriodMs、PIT_PeriodS至少一个初始化
  */
  uint32 PIT_PeriodUs;
  
  /*
    描述：
      PITx模块的周期，单位ms
    取值：
      1~0xFFFFFFFF
    初始化：
      不必须初始化，PIT_PeriodUs、PIT_PeriodMs、PIT_PeriodS至少一个初始化
  */
  uint32 PIT_PeriodMs;
  
  /*
    描述：
      PITx模块的周期，单位s
    取值：
      1~0xFFFFFFFF
    初始化：
      不必须初始化，PIT_PeriodUs、PIT_PeriodMs、PIT_PeriodS至少一个初始化
  */
  uint32 PIT_PeriodS;

  /*
    描述：
      PITx中断回调函数
    取值：
      函数必须为无返回值,无参数(eg. void isr(void);)
    初始化：
      不必须初始化、如未初始化则不会触发中断
  */
  PIT_ISR_CALLBACK PIT_Isr; 
} PIT_InitTypeDef;

//PIT通用初始化函数，选择PITx、配置中断周期、中断函数
uint8 LPLD_PIT_Init(PIT_InitTypeDef);
//PIT反初始化函数，关闭选择的PITx
void LPLD_PIT_Deinit(PIT_InitTypeDef);
//使能PITx中断
void LPLD_PIT_EnableIrq(PIT_InitTypeDef);
//禁用PITx中断
void LPLD_PIT_DisableIrq(PIT_InitTypeDef);
/**********************以下功能移植山外*********************************/
#define     PIT_Flag_Clear(PITx)          PIT->CHANNEL[pitn].TFLG|=PIT_TFLG_TIF_MASK        //清中断标志（写1 清空标志位）
void        LPLD_PIT_Delay(PITx, uint32 cnt);                                       //PIT延时（不需要初始化的）
#define     LPLD_PIT_DelayMs(PITx,ms)          LPLD_PIT_Delay(PITx,ms * g_bus_clock/1000);        //PIT延时 ms
#define     LPLD_PIT_DelayUs(PITx,us)          LPLD_PIT_Delay(PITx,us * g_bus_clock/1000000);   //PIT延时 us
#define     LPLD_PIT_DelayNs(PITx,ns)          LPLD_PIT_Delay(PITx,ns * g_bus_clock/1000000000);//PIT延时 ns
/*****************************以下代码用于PIT计时*************************************/
void    LPLD_PIT_Close(PITx);
void    LPLD_PIT_Time_Start(PITx);                                                       //PIT开始计时
uint32   LPLD_PIT_Time_Get(PITx);                                                       //获取 PITn计时时间(超时时会关闭 定时器)（单位为 bus时钟）(若值为 0xFFFFFFFF，则表示溢出)

#define LPLD_PIT_Time_GetMs(PITx)   (LPLD_PIT_Time_Get(PITx)/(g_bus_clock/1000))                    //获取计时时间（单位为 ms）
#define LPLD_PIT_Time_GetUs(PITx)   (LPLD_PIT_Time_Get(PITx)/(g_bus_clock/1000000))             //获取计时时间（单位为 us）

/*****************************以上代码用于PIT计时*************************************/

#endif /* __HW_PIT_H__ */
