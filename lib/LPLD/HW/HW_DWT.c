/*!
 *     COPYRIGHT NOTICE
 *     Copyright (c) 2013,ɽ��Ƽ�
 *     All rights reserved.
 *     �������ۣ�ɽ����̳ http://www.vcan123.com
 *
 *     ��ע�������⣬�����������ݰ�Ȩ����ɽ��Ƽ����У�δ����������������ҵ��;��
 *     �޸�����ʱ���뱣��ɽ��Ƽ��İ�Ȩ������
 *
 * @file       MK60_dwt.h
 * @brief      flash������
 * @author     ɽ��Ƽ�
 * @version    v5.0
 * @date       2013-10-22
 */

#include "common.h"

#define  DWT_CLK_HHZ                        (CORE_CLK_MHZ)
#define  DEM_CR_TRCENA_MASK                 (1 << 24)
#define  DWT_CTRL_CYCCNTENA                 (1 <<  0)

/*!
 *  @brief      ��ʱ(us)
 *  @param      us      ��ʱʱ��
 *  @since      v5.0
 */
void dwt_delay_us(uint32 us)
{
    uint32 st,et,ts;
    CoreDebug->DEMCR  |= CoreDebug_DEMCR_TRCENA_Msk;        //ʹ�� ����ʱ�� ��Ĭ���Ѿ��򿪣�
    DWT->CTRL         |=  DWT_CTRL_CYCCNTENA;

    st = DWT->CYCCNT;
    ts =  us * (DWT_CLK_HHZ);
    et = st + ts;
    if(et < st)
    {
        //�������Ҫת��һ��
        while(DWT->CYCCNT > et);      //�ȴ� DWT_CYCCNT ��� ����0
    }

    while(DWT->CYCCNT < et);      //�ȴ� DWT_CYCCNT ���׼���ֵ
}

/*!
 *  @brief      ��ʱ(ms)
 *  @param      ms      ��ʱʱ��
 *  @since      v5.0
 */
void dwt_delay_ms(uint32 ms)
{
    while(ms--)
    {
        dwt_delay_us(1000);
    }
}