#include "common.h"

/*----------------�������-----------------*/
float qd_result_1 = 0;  //��¼10�����������Ȧ��������
float qd_result_2 = 0;  //��¼10�������ұ���Ȧ��������
float qd_result = 0;    //10������������Ȧ�������Ĳ�ֵ   

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

/*----------------�����ʼ��-----------------*/
void Qd_Init(void)
{
  ftm_init_struct.FTM_Ftmx = FTM2;                      //ֻ��FTM1��FTM2���������빦��
  ftm_init_struct.FTM_Mode = FTM_MODE_QD;               //�������빦��
  ftm_init_struct.FTM_QdMode = QD_MODE_CNTDIR;          //�����ͷ������ģʽ 
  ftm_init_struct.FTM_ClkDiv = FTM_CLK_DIV64;          //128��Ƶ

  LPLD_FTM_Init(ftm_init_struct);                       //��ʼ��FTM
  LPLD_FTM_QD_Enable(FTM2, PTA10, PTA11);               //PTA10���Ž�A�����롢PTA11���Ž�B������
  
  lptmr_init_struct.LPTMR_Mode = LPTMR_MODE_PLACC;      //�������ģʽ
  lptmr_init_struct.LPTMR_PluseAccInput = LPTMR_ALT2;   //ѡ��ALT2ͨ����PTC5
  lptmr_init_struct.LPTMR_Isr = NULL;
  LPLD_LPTMR_Init(lptmr_init_struct);                   //��ʼ��LPTMR     
  
  DMA_Count_Init();
}

/*----------------PIT�жϺ���-----------------*/
void PIT_isr(void)
{
  a1 = LPLD_FTM_GetCounter(FTM2);        //ͨ��FTM2������ֵ
  b1 = LPLD_LPTMR_GetPulseAcc();         //LPTMR��¼������ֵ
  c = DMA_count_get(DMA_CH0) * 5;           //DMA��¼������ֵ
  d = DMA_count_get(DMA_CH5) * 5;           //DMA��¼������ֵ
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
  
  Steer_Change();                             //�ı���ת��
  
  Motor_Change();
  
  Out_Data();                                   //ͨ��������������
  
  LPLD_FTM_ClearCounter(FTM2);                  //��ռ�����FTM2  
  LPLD_LPTMR_ResetCounter();                    //���LPTMR
  DMA_count_reset(DMA_CH0);                     //���DMA��¼������ֵ
  DMA_count_reset(DMA_CH5);                     //���DMA��¼������ֵ
  DMA_count_reset(DMA_CH1);                     //���DMA��¼������ֵ

  
  LPLD_LPTMR_Init(lptmr_init_struct);

}

/*----------------PIT��ʱ����ʼ��-----------------*/
void PIT_Init(void)
{
  pit_init_struct.PIT_Pitx = PIT0;      //����PIT0��ʱ�����жϲ���
  pit_init_struct.PIT_PeriodMs = 10;    //10�������һ���жϣ���¼������
  pit_init_struct.PIT_Isr = PIT_isr;
  
  LPLD_PIT_Init(pit_init_struct);       //��ʼ��PIT
  
  LPLD_PIT_EnableIrq(pit_init_struct);  //ʹ��PIT��ʱ�ж�
}

/*----------------DMAh & GPIO��ʼ��-----------------*/
void DMA_Count_Init(void)
{
  static GPIO_InitTypeDef DMA_GPIO;
  
  DMA_GPIO.GPIO_PTx=PTA;                                //ѡ��PORT
  DMA_GPIO.GPIO_Pins=GPIO_Pin7;                        //ѡ��Ҫ��ʼ��������
  DMA_GPIO.GPIO_Dir=DIR_INPUT;                          //ѡ������ģʽ
  DMA_GPIO.GPIO_PinControl=INPUT_PULL_UP|IRQC_DMAFA;    //ѡ�������������½��ز���DMA���� 
  LPLD_GPIO_Init(DMA_GPIO);                             //��ʼ����ӦGPIO��
  DMA_count_Init(DMA_CH0, PTA7,0x7FFF);                //DMA���������ʼ��

  DMA_GPIO.GPIO_PTx=PTB;                                //ѡ��PORT
  DMA_GPIO.GPIO_Pins=GPIO_Pin18;                        //ѡ��Ҫ��ʼ��������
  DMA_GPIO.GPIO_Dir=DIR_INPUT;                          //ѡ������ģʽ
  DMA_GPIO.GPIO_PinControl=INPUT_PULL_UP|IRQC_DMAFA;    //ѡ�������������½��ز���DMA���� 
  LPLD_GPIO_Init(DMA_GPIO);                             //��ʼ����ӦGPIO��
  DMA_count_Init(DMA_CH5, PTB18,0x7FFF);                //DMA���������ʼ��

//  DMA_GPIO.GPIO_PTx=PTE;                                //ѡ��PORT
//  DMA_GPIO.GPIO_Pins=GPIO_Pin26;                        //ѡ��Ҫ��ʼ��������
//  DMA_GPIO.GPIO_Dir=DIR_INPUT;                          //ѡ������ģʽ
//  DMA_GPIO.GPIO_PinControl=INPUT_PULL_UP|IRQC_DMAFA;    //ѡ�������������½��ز���DMA���� 
//  LPLD_GPIO_Init(DMA_GPIO);                             //��ʼ����ӦGPIO��
//  DMA_count_Init(DMA_CH1, PTE26,0x7FFF);                //DMA���������ʼ��
}
