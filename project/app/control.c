#include "common.h"
#include "control.h"
#define VCAN_PORT UART4

#define CenterSteerVal 0      //舵机0转角对应的PWM值

PID_Typedef Turn_PID;	        //转向环的PID
PID_Typedef speed_PID;          //电机
FTM_InitTypeDef Steer_Init_Struct;
FTM_InitTypeDef FTM_Init;
UART_InitTypeDef uart0_init_struct;

float Deriv_error[5]={0};
int32_t data[4] = {0};
int32_t SteerValArray[3] = {0};
int32_t SetSpeed = 2800;
int32_t MotorSpeed = 3100;
int32_t SteerCount = 0;

extern int32_t SteerVal;
extern int32_t SteerValMeasureArray[5];
extern int32_t SteerValMeasure;
extern int32_t qd_result;
extern int32_t a,b,c,d;

void Steer_Init(void);
void Steer_Limit(void);
void Steer_Change(void);
void Motor_Init(void);
void Motor_Change(void);
void PID_Init(void);
void Steer_PID_Calculate(void);
void uart_init(void);
void vcan_sendware(uint8 *wareaddr, uint32 waresize);
void Out_Data(void);

  /*----------------舵机初始化-----------------*/
void Steer_Init(void)
{
  FTM_Init.FTM_Ftmx=FTM1;
  FTM_Init.FTM_Mode=FTM_MODE_PWM;
  FTM_Init.FTM_PwmFreq=50;
  FTM_Init.FTM_PwmDeadtimeVal=0;
  LPLD_FTM_Init(FTM_Init);
  //舵机用FTM1，PTA8口控制
  LPLD_FTM_PWM_Enable(FTM1, FTM_Ch0, 0, PTA8,ALIGN_LEFT);  //A+
}
  
/*----------------舵机转角限制-----------------*/
void Steer_Limit(void)
{
  if( (SteerValMeasureArray[0]<0&&SteerValMeasureArray[1]<0&&SteerValMeasureArray[2]<0&&SteerValMeasureArray[3]<0&&SteerValMeasureArray[4]<0)||(SteerValMeasureArray[0]>0&&SteerValMeasureArray[1]>0&&SteerValMeasureArray[2]>0&&SteerValMeasureArray[3]>0&&SteerValMeasureArray[4]>0) ) 
  { 
    if(SteerVal > 625)
      SteerVal = 625;
    else
      if(SteerVal < 475)
        SteerVal = 475;
  }
  else 
  {
    if(SteerVal > 580)
      SteerVal = 580;
    else
      if(SteerVal < 520)
        SteerVal = 520;
  }
  
  SteerValArray[2] = SteerValArray[1];
  SteerValArray[1] = SteerValArray[0];
  SteerValArray[0] = SteerVal;
}

/*----------------控制舵机转向-----------------*/
void Steer_Change(void)
{ 
  if(SteerCount)
  {
    LPLD_FTM_PWM_ChangeDuty(FTM1, FTM_Ch0, SteerVal);     //转动舵机
    SteerCount = 0;
  }
  else
    SteerCount++;
  
  if( !( SteerValMeasure >-2 && SteerValMeasure < 2 ) )
  {
    SteerValMeasureArray[4] = SteerValMeasureArray[3];
    SteerValMeasureArray[3] = SteerValMeasureArray[2];
    SteerValMeasureArray[2] = SteerValMeasureArray[1];
    SteerValMeasureArray[1] = SteerValMeasureArray[0];
    SteerValMeasureArray[0] = SteerValMeasure;
    SteerValMeasure=( SteerValMeasureArray[0]+SteerValMeasureArray[1]+SteerValMeasureArray[2] )/3;
  }

}

/*----------------电机初始化-----------------*/
void Motor_Init(void)
{
  FTM_Init.FTM_Ftmx=FTM0;
  FTM_Init.FTM_Mode=FTM_MODE_PWM;
  FTM_Init.FTM_PwmFreq=20000;
  FTM_Init.FTM_PwmDeadtimeVal=0;
  LPLD_FTM_Init(FTM_Init);
  //电机用FTM0，PTD7口控制
  LPLD_FTM_PWM_Enable(FTM0, FTM_Ch2, 0, PTC3,ALIGN_LEFT);//A+
}

void Motor_Change(void)
{
  if( MotorSpeed != 2500 )
  {
    if( (SteerValMeasureArray[0]<0&&SteerValMeasureArray[1]<0&&SteerValMeasureArray[2]<0&&SteerValMeasureArray[3]<0&&SteerValMeasureArray[4]<0)||(SteerValMeasureArray[0]>0&&SteerValMeasureArray[1]>0&&SteerValMeasureArray[2]>0&&SteerValMeasureArray[3]>0&&SteerValMeasureArray[4]>0) ) 
    { 
      MotorSpeed = 2700;        //弯道减速
    }
    else 
    { 
      MotorSpeed = 3400;        //直道速度
    }
  }

  LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch2, MotorSpeed);
}

/*----------------PID初始化-----------------*/
void PID_Init(void)
{
  Turn_PID.P=1.2;                 //转向的PID
  Turn_PID.I=0;
  Turn_PID.D=10;        
//  Turn_PID.iLimit=200;
  
  speed_PID.P=100;
  speed_PID.I=0;
  speed_PID.D=0;
  speed_PID.iLimit=5000;
}

/*----------------PID计算-----------------*/
void Steer_PID_Calculate(void)
{  	
  if( (SteerValMeasureArray[0]<0&&SteerValMeasureArray[1]<0&&SteerValMeasureArray[2]<0&&SteerValMeasureArray[3]<0&&SteerValMeasureArray[4]<0)||(SteerValMeasureArray[0]>0&&SteerValMeasureArray[1]>0&&SteerValMeasureArray[2]>0&&SteerValMeasureArray[3]>0&&SteerValMeasureArray[4]>0) ) 
    Turn_PID.P=2;                 //弯道的P
  else 
    Turn_PID.P=0.8;                 //直道的P

  Turn_PID.Error=CenterSteerVal-SteerValMeasure;       //误差=期望值-测量值
  Deriv_error[0]=Turn_PID.Error;
  Turn_PID.Deriv=Deriv_error[0]-2*Deriv_error[1]+Deriv_error[2];     
  Turn_PID.Output=Turn_PID.P*Turn_PID.Error+Turn_PID.Deriv*Turn_PID.I+Turn_PID.D*Turn_PID.Deriv+550;
 
  Deriv_error[4]=Deriv_error[3];
  Deriv_error[3]=Deriv_error[2];
  Deriv_error[2]=Deriv_error[1];
  Deriv_error[1]=Deriv_error[0];
  
  if( c > 3600 && c < 3800 && a > 840 && a < 870 && b > 840 && b < 870 ) 
  {
    if( SteerValMeasureArray[2] < 0 && SteerValMeasureArray[3] < 0 && SteerValMeasureArray[4] < 0 )
      SteerVal = 625;           //冲出赛道时角度打死
    else 
      if( SteerValMeasureArray[2] > 0 && SteerValMeasureArray[3] > 0 && SteerValMeasureArray[4] > 0 )
          SteerVal = 475;       //冲出赛道时角度打死

    MotorSpeed = 2500;          //冲出赛道时减速
  }
  else
    SteerVal = Turn_PID.Output;
}

/*----------------蓝牙串口初始化-----------------*/
void uart_init(void)
{
  uart0_init_struct.UART_Uartx = VCAN_PORT;         //使用UART0
  uart0_init_struct.UART_BaudRate = 115200;     //设置波特率115200
  uart0_init_struct.UART_RxPin = PTE25;         //接收引脚为PTA15
  uart0_init_struct.UART_TxPin = PTE24;         //发送引脚为PTA14
  uart0_init_struct.UART_TxDMAEnable = TRUE;
  uart0_init_struct.UART_TxIntEnable = TRUE;
  LPLD_UART_Init(uart0_init_struct);            //初始化UART
}

/*----------------蓝牙数据发送命令-----------------*/
void vcan_sendware(uint8 *wareaddr, uint32 waresize)
{
  #define CMD_WARE 3
  uint8 cmdf[2] = {CMD_WARE, ~CMD_WARE};    //串口调试 使用的前命令
  uint8 cmdr[2] = {~CMD_WARE, CMD_WARE};    //串口调试 使用的后命令

  LPLD_UART_PutCharArr(VCAN_PORT, cmdf, sizeof(cmdf));    //先发送前命令
  LPLD_UART_PutCharArr(VCAN_PORT, wareaddr, waresize);    //发送数据
  LPLD_UART_PutCharArr(VCAN_PORT, cmdr, sizeof(cmdr));    //发送后命令
}

/*----------------发送蓝牙数据-----------------*/
void Out_Data(void)
{
  data[0] = SteerValMeasureArray[0];
  data[1] = SteerValMeasureArray[1];
  data[2] = SteerValMeasureArray[2];
  data[3] = SteerValMeasureArray[3];
  vcan_sendware((uint8_t *)data, sizeof(data));

}

