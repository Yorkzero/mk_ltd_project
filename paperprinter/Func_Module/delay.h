/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :delay.h 
* Desc     :����STM8L051XX/STM8L151XXϵ�е�Ƭ��ƽ̨ʵ�ֵ���ʱ����ģ��
* 
* 
* Author   :CenLinbo
* Date     :2020/06/09
* Notes    :�������õ�ϵͳʱ��ʵ�ֲ�ͬʱ��Ƶ���µ���ʱ����

*           2020/08/22 11:32, add this by CenLinbo
*           1��Ҫʵ��10us�ڵ�׼ȷ��ʱ����Ҫϵͳ��Ƶ������1M����
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/06/09, CenLinbo create this file

*           2020/08/18 16:17, add this by CenLinbo
*           1��������ʹ��500K��250KHz����Ƶʱ�޷�ʵ��1us�ľ�ȷ��ʱ������ƽʱ
               ������ʹ����δ˵�Ƶ��ʱ����Ϊϵͳʱ�ӣ����ɾ����ʱ����ص���ʱ����
*         
******************************************************************************/
#ifndef _DELAY_H_     
#define _DELAY_H_    


/*------------------------------- Includes ----------------------------------*/
#include "main.h"

/*----------------------------- Global Defines ------------------------------*/


/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/
extern volatile uint8_t rtc_delay_flag;  //����ʹ��RTC��ʱ����ʱ�ı�־λ����Ҫ��RTC�����ж�����0

/*------------------------ Global Function Prototypes -----------------------*/
#if ((SYS_CLK_FREQ == SYS_CLK_FREQ_2M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_4M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_8M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_16M))
/******************************************************************************
* Name     :delay_us_1 
*
* Desc     :
* Param_in :n����ʱ��us�������ֻ������16383��ms����ʱ����ʹ��ms����ʱ����
* Param_out:
* Return   :
* Global   :
* Note     :1�����ú���ǰ��LDװ�ز�����X�Ĵ���Ҫ2T
            2�����ú���CALLҪ4T��RET����Ҫ4T
            3����Ƶ16M����С��ʱ2us������1usΪ��ȷ��ʱ
            4����Ƶ8M����С��ʱ3us������2usΪ��ȷ��ʱ
            5����Ƶ4M����С��ʱ4us������3usΪ��ȷ��ʱ
            6����Ƶ2M����С��ʱ11us������10us��Ϊż��ʱ��ȷ��ʱ��Ϊ����ʱ������ֵ��1us
            7����Ƶ1M��500K��250Kʱ�˺����������ʱ̫�󣬲���׼ȷʵ��100us�ڵ���ʱ���ʲ�ʵ��
            
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/06/11, Create this function by CenLinbo

*           2020/08/21 10:22, add this by CenLinbo
*           1�����¶�16M/8M/4M/2M��Ƶ�µ���ʱ�����㲢���ԣ�Ŀǰ����
 ******************************************************************************/
extern void delay_us_1(uint16_t n);

/******************************************************************************
* Name      :delay_ms_1 
*
* Desc     :����us��ʱʵ�ֵ�ms����ʱ������1000us����ʱ���൱׼ȷ�ģ�����ms����ʱҲ��׼ȷ
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :bobde163
* -------------------------------------
* Log     :2020/06/17 21:16:22, Create this function by bobde163
 ******************************************************************************/
extern void delay_ms_1(uint16_t n);
#endif

/******************************************************************************
* Name     :delay_ms_2 
*
* Desc     :ͨ��Ƭ��RTC���Զ����ѹ���ʵ�ֵ͹�����ʱ
* Param_in :n��Ҫ��ʱ��ms������Χ1~32767ms
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/22, Create this function by CenLinbo
 ******************************************************************************/

extern void delay_ms_2(uint16_t n);

#if 1
//ϵͳʱ��1Mʱ��ָ������Ϊ1us
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_1M)
#define delay_1us()    nop();
#define delay_2us()    asm("TNZW X")
#define delay_5us()    {asm("TNZW X");asm("TNZW X");nop();}

extern void delay_10us(void);


//ϵͳʱ��2Mʱ��ָ������Ϊ0.5us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
#define delay_1us()    asm("TNZW X")
#define delay_2us()    {asm("TNZW X");asm("TNZW X");}

//delay_5usʹ�ú���ʵ��
extern void delay_5us(void);

#define delay_10us()   {delay_5us();delay_5us();}

//ϵͳʱ��4Mʱ��ָ������Ϊ0.25us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
#define delay_1us()    {asm("TNZW X");asm("TNZW X");}


#define delay_5us()    delay_us_1(5)
#define delay_10us()   delay_us_1(10)

//ϵͳʱ��8Mʱ��ָ������Ϊ0.125us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
extern void delay_1us(void);

#define delay_2us()   {delay_1us();delay_1us();}
#define delay_5us()    delay_us_1(5)
#define delay_10us()   delay_us_1(10)

//ϵͳʱ��16Mʱ��ָ������Ϊ0.0625us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
extern void delay_1us(void);

#define delay_2us()    delay_us_1(2)
#define delay_5us()    delay_us_1(5)
#define delay_10us()   delay_us_1(10)

#else
#error "Invalid system clock value...."
#endif

#endif


#endif //_DELAY_H_

