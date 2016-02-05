#include "common.h"
#include "control.h"
#define VCAN_PORT UART4

#define CenterSteerVal 0      //���0ת�Ƕ�Ӧ��PWMֵ

PID_Typedef Turn_PID;	        //ת�򻷵�PID
PID_Typedef speed_PID;          //���
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

  /*----------------�����ʼ��-----------------*/
void Steer_Init(void)
{
  FTM_Init.FTM_Ftmx=FTM1;
  FTM_Init.FTM_Mode=FTM_MODE_PWM;
  FTM_Init.FTM_PwmFreq=50;
  FTM_Init.FTM_PwmDeadtimeVal=0;
  LPLD_FTM_Init(FTM_Init);
  //�����FTM1��PTA8�ڿ���
  LPLD_FTM_PWM_Enable(FTM1, FTM_Ch0, 0, PTA8,ALIGN_LEFT);  //A+
}
  
/*----------------���ת������-----------------*/
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

/*----------------���ƶ��ת��-----------------*/
void Steer_Change(void)
{ 
  if(SteerCount)
  {
    LPLD_FTM_PWM_ChangeDuty(FTM1, FTM_Ch0, SteerVal);     //ת�����
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

/*----------------�����ʼ��-----------------*/
void Motor_Init(void)
{
  FTM_Init.FTM_Ftmx=FTM0;
  FTM_Init.FTM_Mode=FTM_MODE_PWM;
  FTM_Init.FTM_PwmFreq=20000;
  FTM_Init.FTM_PwmDeadtimeVal=0;
  LPLD_FTM_Init(FTM_Init);
  //�����FTM0��PTD7�ڿ���
  LPLD_FTM_PWM_Enable(FTM0, FTM_Ch2, 0, PTC3,ALIGN_LEFT);//A+
}

void Motor_Change(void)
{
  if( MotorSpeed != 2500 )
  {
    if( (SteerValMeasureArray[0]<0&&SteerValMeasureArray[1]<0&&SteerValMeasureArray[2]<0&&SteerValMeasureArray[3]<0&&SteerValMeasureArray[4]<0)||(SteerValMeasureArray[0]>0&&SteerValMeasureArray[1]>0&&SteerValMeasureArray[2]>0&&SteerValMeasureArray[3]>0&&SteerValMeasureArray[4]>0) ) 
    { 
      MotorSpeed = 2700;        //�������
    }
    else 
    { 
      MotorSpeed = 3400;        //ֱ���ٶ�
    }
  }

  LPLD_FTM_PWM_ChangeDuty(FTM0, FTM_Ch2, MotorSpeed);
}

/*----------------PID��ʼ��-----------------*/
void PID_Init(void)
{
  Turn_PID.P=1.2;                 //ת���PID
  Turn_PID.I=0;
  Turn_PID.D=10;        
//  Turn_PID.iLimit=200;
  
  speed_PID.P=100;
  speed_PID.I=0;
  speed_PID.D=0;
  speed_PID.iLimit=5000;
}

/*----------------PID����-----------------*/
void Steer_PID_Calculate(void)
{  	
  if( (SteerValMeasureArray[0]<0&&SteerValMeasureArray[1]<0&&SteerValMeasureArray[2]<0&&SteerValMeasureArray[3]<0&&SteerValMeasureArray[4]<0)||(SteerValMeasureArray[0]>0&&SteerValMeasureArray[1]>0&&SteerValMeasureArray[2]>0&&SteerValMeasureArray[3]>0&&SteerValMeasureArray[4]>0) ) 
    Turn_PID.P=2;                 //�����P
  else 
    Turn_PID.P=0.8;                 //ֱ����P

  Turn_PID.Error=CenterSteerVal-SteerValMeasure;       //���=����ֵ-����ֵ
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
      SteerVal = 625;           //�������ʱ�Ƕȴ���
    else 
      if( SteerValMeasureArray[2] > 0 && SteerValMeasureArray[3] > 0 && SteerValMeasureArray[4] > 0 )
          SteerVal = 475;       //�������ʱ�Ƕȴ���

    MotorSpeed = 2500;          //�������ʱ����
  }
  else
    SteerVal = Turn_PID.Output;
}

/*----------------�������ڳ�ʼ��-----------------*/
void uart_init(void)
{
  uart0_init_struct.UART_Uartx = VCAN_PORT;         //ʹ��UART0
  uart0_init_struct.UART_BaudRate = 115200;     //���ò�����115200
  uart0_init_struct.UART_RxPin = PTE25;         //��������ΪPTA15
  uart0_init_struct.UART_TxPin = PTE24;         //��������ΪPTA14
  uart0_init_struct.UART_TxDMAEnable = TRUE;
  uart0_init_struct.UART_TxIntEnable = TRUE;
  LPLD_UART_Init(uart0_init_struct);            //��ʼ��UART
}

/*----------------�������ݷ�������-----------------*/
void vcan_sendware(uint8 *wareaddr, uint32 waresize)
{
  #define CMD_WARE 3
  uint8 cmdf[2] = {CMD_WARE, ~CMD_WARE};    //���ڵ��� ʹ�õ�ǰ����
  uint8 cmdr[2] = {~CMD_WARE, CMD_WARE};    //���ڵ��� ʹ�õĺ�����

  LPLD_UART_PutCharArr(VCAN_PORT, cmdf, sizeof(cmdf));    //�ȷ���ǰ����
  LPLD_UART_PutCharArr(VCAN_PORT, wareaddr, waresize);    //��������
  LPLD_UART_PutCharArr(VCAN_PORT, cmdr, sizeof(cmdr));    //���ͺ�����
}

/*----------------������������-----------------*/
void Out_Data(void)
{
  data[0] = SteerValMeasureArray[0];
  data[1] = SteerValMeasureArray[1];
  data[2] = SteerValMeasureArray[2];
  data[3] = SteerValMeasureArray[3];
  vcan_sendware((uint8_t *)data, sizeof(data));

}

