/**
 * @file HW_PIT.c
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
#include "common.h"
#include "HW_PIT.h"

//用户自定义中断服务函数数组
PIT_ISR_CALLBACK PIT_ISR[4];

/*
 * LPLD_PIT_Init
 * PIT通用初始化函数，选择PITx、配置中断周期、中断函数
 * 
 * 参数:
 *    pit_init_structure--PIT初始化结构体，
 *                        具体定义见PIT_InitTypeDef
 *
 * 输出:
 *    0--配置错误
 *    1--配置成功
 */
uint8 LPLD_PIT_Init(PIT_InitTypeDef pit_init_structure)
{ 
  //计算定时加载值
  uint32 ldval = pit_init_structure.PIT_PeriodUs*(g_bus_clock/1000000)
               + pit_init_structure.PIT_PeriodMs*1000*(g_bus_clock/1000000)
               + pit_init_structure.PIT_PeriodS*1000000*(g_bus_clock/1000000);
  PITx pitx = pit_init_structure.PIT_Pitx;
  PIT_ISR_CALLBACK isr_func = pit_init_structure.PIT_Isr;
  
  //参数检查
  ASSERT( pitx <= PIT3);        //判断模块号
  ASSERT( ldval > 0);           //判断时加载值                
  
  //开启定时模块时钟
  SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
  
  // 开启 PIT
  PIT->MCR = 0x00;
 
  if(isr_func != NULL){
    PIT_ISR[pitx] = isr_func;
    //使能中断
    PIT->CHANNEL[pitx].TCTRL = PIT_TCTRL_TIE_MASK;
    //在NVIC中使能PIT中断
    //enable_irq((IRQn_Type)(PIT0_IRQn + (IRQn_Type)pitx)); 
  }
  
  //period = (period_ns/bus_period_ns)-1
  PIT->CHANNEL[pitx].LDVAL = ldval-1;
  //使能中断
  //PIT->CHANNEL[pitx].TCTRL = PIT_TCTRL_TIE_MASK;
  //开始定时
  PIT->CHANNEL[pitx].TCTRL |= PIT_TCTRL_TEN_MASK;
  
  return 1;
}

/*
 * LPLD_PIT_Deinit
 * PIT反初始化函数，关闭选择的PITx
 * 
 * 参数:
 *    pit_init_structure--PIT初始化结构体，
 *                        具体定义见PIT_InitTypeDef
 *
 * 输出:
 *    无
 */
void LPLD_PIT_Deinit(PIT_InitTypeDef pit_init_structure)
{ 
  PITx pitx = pit_init_structure.PIT_Pitx;
  
  //参数检查
  ASSERT( pitx <= PIT3);        //判断模块号              

  disable_irq((IRQn_Type)(PIT0_IRQn + (IRQn_Type)pitx)); 

  //禁用中断\停止定时
  PIT->CHANNEL[pitx].TCTRL = 0;
}

/*
 * LPLD_PIT_EnableIrq
 * 使能PITx中断
 * 
 * 参数:
 *    pit_init_structure--PIT初始化结构体，
 *                        具体定义见PIT_InitTypeDef
 *
 * 输出:
 *    无
 *
 */
void LPLD_PIT_EnableIrq(PIT_InitTypeDef pit_init_structure)
{
  PITx pitx = pit_init_structure.PIT_Pitx;
  
  //参数检查
  ASSERT( pitx <= PIT3);                //判断PITx

  enable_irq((IRQn_Type)(PIT0_IRQn + (IRQn_Type)pitx)); 
}

/*
 * LPLD_PIT_DisableIrq
 * 禁用PITx中断
 * 
 * 参数:
 *    pit_init_structure--PIT初始化结构体，
 *                        具体定义见PIT_InitTypeDef
 *
 * 输出:
 *    无
 *
 */
void LPLD_PIT_DisableIrq(PIT_InitTypeDef pit_init_structure)
{
  PITx pitx = pit_init_structure.PIT_Pitx;
  
  //参数检查
  ASSERT( pitx <= PIT3);                //判断PITx
  
  disable_irq((IRQn_Type)(PIT0_IRQn + (IRQn_Type)pitx));
}


/*
 * PIT0--PIT3中断处理函数
 * 与启动文件startup_K60.s中的中断向量表关联
 * 用户无需修改，程序自动进入对应通道中断函数
 */

void PIT0_IRQHandler(void)
{
#if (UCOS_II > 0u)
  OS_CPU_SR  cpu_sr = 0u;
  OS_ENTER_CRITICAL(); //告知系统此时已经进入了中断服务子函数
  OSIntEnter();
  OS_EXIT_CRITICAL();
#endif
  
  //调用用户自定义中断服务
  PIT_ISR[0]();  
  //清除中断标志位
  PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK;
  
#if (UCOS_II > 0u)
  OSIntExit();          //告知系统此时即将离开中断服务子函数
#endif
}
void PIT1_IRQHandler(void)
{
#if (UCOS_II > 0u)
  OS_CPU_SR  cpu_sr = 0u;
  OS_ENTER_CRITICAL(); //告知系统此时已经进入了中断服务子函数
  OSIntEnter();
  OS_EXIT_CRITICAL();
#endif
  
  //调用用户自定义中断服务
  PIT_ISR[1]();  
  //清除中断标志位
  PIT->CHANNEL[1].TFLG |= PIT_TFLG_TIF_MASK;
  
#if (UCOS_II > 0u)
  OSIntExit();          //告知系统此时即将离开中断服务子函数
#endif
}
void PIT2_IRQHandler(void)
{
#if (UCOS_II > 0u)
  OS_CPU_SR  cpu_sr = 0u;
  OS_ENTER_CRITICAL(); //告知系统此时已经进入了中断服务子函数
  OSIntEnter();
  OS_EXIT_CRITICAL();
#endif
  
  //调用用户自定义中断服务
  PIT_ISR[2]();  
  //清除中断标志位
  PIT->CHANNEL[2].TFLG |= PIT_TFLG_TIF_MASK;
  
#if (UCOS_II > 0u)
  OSIntExit();          //告知系统此时即将离开中断服务子函数
#endif
}
void PIT3_IRQHandler(void)
{
#if (UCOS_II > 0u)
  OS_CPU_SR  cpu_sr = 0u;
  OS_ENTER_CRITICAL(); //告知系统此时已经进入了中断服务子函数
  OSIntEnter();
  OS_EXIT_CRITICAL();
#endif
  
  //调用用户自定义中断服务
  PIT_ISR[3]();  
  //清除中断标志位
  PIT->CHANNEL[3].TFLG |= PIT_TFLG_TIF_MASK;
  
#if (UCOS_II > 0u)
  OSIntExit();          //告知系统此时即将离开中断服务子函数
#endif
}
/*********************************以下移植山外**********************************/
/*!
 *  @brief      PITn延时
 *  @param      PITn        模块号（PIT0~PIT3）
 *  @param      cnt         延时时间(单位为bus时钟周期)
 *  @since      v5.0
 *  Sample usage:
                    pit_delay(PIT0, 1000);                         //延时 1000 个bus时钟
 */
void LPLD_PIT_Delay(PITx pitn, uint32 cnt)
{
    //PIT 用的是 Bus Clock 总线频率
    //溢出计数 = 总线频率 * 时间

    ASSERT( cnt > 0 );              //用断言检测 时间必须不能为 0

    SIM->SCGC6       |= SIM_SCGC6_PIT_MASK;                          //使能PIT时钟

    PIT->MCR         &= ~(PIT_MCR_MDIS_MASK | PIT_MCR_FRZ_MASK );    //使能PIT定时器时钟 ，调试模式下继续运行

    PIT->CHANNEL[pitn].TCTRL &= ~( PIT_TCTRL_TEN_MASK );                     //禁用PIT ，以便设置加载值生效

    PIT->CHANNEL[pitn].LDVAL  = cnt - 1;                                     //设置溢出中断时间

    PIT_Flag_Clear(pitn);                                           //清中断标志位

    PIT->CHANNEL[pitn].TCTRL &= ~ PIT_TCTRL_TEN_MASK;                        //禁止PITn定时器（用于清空计数值）
    PIT->CHANNEL[pitn].TCTRL  = ( 0
                         | PIT_TCTRL_TEN_MASK                        //使能 PITn定时器
                         //| PIT_TCTRL_TIE_MASK                      //开PITn中断
                       );

    while( !(PIT->CHANNEL[pitn].TFLG& PIT_TFLG_TIF_MASK));

    PIT_Flag_Clear(pitn);                                           //清中断标志位
}

/*!
 *  @brief      PITn计时开始
 *  @param      PITn        模块号（PIT0~PIT3）
 *  @since      v5.0
 *  Sample usage:
                    pit_time_start(PIT0);                          //PIT0计时开始
 */
void LPLD_PIT_Time_Start(PITx pitn)
{
    //PIT 用的是 Bus Clock 总线频率
    //溢出计数 = 总线频率 * 时间

    SIM->SCGC6       |= SIM_SCGC6_PIT_MASK;                          //使能PIT时钟

    PIT->MCR         &= ~(PIT_MCR_MDIS_MASK | PIT_MCR_FRZ_MASK );    //使能PIT定时器时钟 ，调试模式下继续运行

    PIT->CHANNEL[pitn].TCTRL &= ~( PIT_TCTRL_TEN_MASK );                     //禁用PIT ，以便设置加载值生效

    PIT->CHANNEL[pitn].LDVAL  = ~0;                                          //设置溢出中断时间

    PIT_Flag_Clear(pitn);                                           //清中断标志位

     PIT->CHANNEL[pitn].TCTRL &= ~ PIT_TCTRL_TEN_MASK;                        //禁止PITn定时器（用于清空计数值）
     PIT->CHANNEL[pitn].TCTRL  = ( 0
                         | PIT_TCTRL_TEN_MASK                        //使能 PITn定时器
                         //| PIT_TCTRL_TIE_MASK                      //开PITn中断
                       );
}

/*!
 *  @brief      获取 PITn计时时间(超时时会关闭 定时器)
 *  @param      PITn        模块号（PIT0~PIT3）
 *  @since      v5.0
 *  Sample usage:
                        uint32 time = pit_time_get(PIT0);                         //获取 PITn计时时间
                        if(time != ~0)       //没超时
                        {
                            printf("\n计时时间为：%d us",time*1000/bus_clk_khz);
                        }
 */
uint32 LPLD_PIT_Time_Get(PITx pitn)
{
    uint32 val;

    val = (~0) - PIT->CHANNEL[pitn].CVAL;

    if(PIT->CHANNEL[pitn].TFLG& PIT_TFLG_TIF_MASK)                   //判断是否时间超时
    {
        PIT_Flag_Clear(pitn);                                       //清中断标志位
        PIT->CHANNEL[pitn].TCTRL &= ~ PIT_TCTRL_TEN_MASK;           //禁止PITn定时器（用于清空计数值）
        PIT->CHANNEL[pitn].LDVAL  = ~0;
        PIT->CHANNEL[pitn].TCTRL  = ( 0
                         | PIT_TCTRL_TEN_MASK                       //使能 PITn定时器
                         //| PIT_TCTRL_TIE_MASK                     //开PITn中断
                       );       
        //return ~0;
    }

    if(val == (~0))
    {
        val--;              //确保 不等于 ~0
    }
    return val;
}

/*!
 *  @brief      关闭 pit
 *  @param      PITn        模块号（PIT0~PIT3）
 *  @since      v5.0
 *  Sample usage:
                        pit_close(PIT0);                         //关闭PIT
 */
void LPLD_PIT_Close(PITx pitn)
{
    PIT_Flag_Clear(pitn);                                       //清中断标志位
     PIT->CHANNEL[pitn].TCTRL &= ~ PIT_TCTRL_TEN_MASK;                    //禁止PITn定时器（用于清空计数值）
}
