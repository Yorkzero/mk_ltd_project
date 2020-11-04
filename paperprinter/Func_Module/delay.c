/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :delay.c 
* Desc     :����IAR��������������STM8L051XX/STM8L151XXϵ�е�Ƭ��ƽ̨ʵ�ֵ���ʱ����ģ��
* 
* 
* Author   :CenLinbo
* Date     :2020/06/11
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/06/11, CenLinbo create this file
*         
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "delay.h"


/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
volatile uint8_t rtc_delay_flag = 0;  //����ʹ��RTC��ʱ����ʱ�ı�־λ����Ҫ��RTC�����ж�����0

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
//ϵͳʱ��1Mʱ��ָ������Ϊ1us
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_1M)
void delay_10us(void)
{
asm
(
    "TNZW X"//2T�����Ϻ������÷���ռ��8T������10T��10us
);
}


//ϵͳʱ��2Mʱ��ָ������Ϊ0.5us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
//delay_5usʹ�ú���ʵ��
void delay_5us(void)
{
asm
(
    "TNZW X"//2T�����Ϻ������÷���ռ��8T������10T��5us
);
}

//ϵͳʱ��4Mʱ��ָ������Ϊ0.25us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
//delay_2usʹ�ú���ʵ��
void delay_2us(void)
{
    //void(0);//�������úͷ����Ѿ�ռ��8T������2us
}

//ϵͳʱ��8Mʱ��ָ������Ϊ0.125us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
void delay_1us(void)
{
    void(0);//�������úͷ����Ѿ�ռ��8T������1us
}

//ϵͳʱ��16Mʱ��ָ������Ϊ0.0625us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
void delay_1us(void)
{
    //�������úͷ����Ѿ�ռ��8T,��ִ��8T������1us
    asm("TNZW X");
    asm("TNZW X");
    asm("TNZW X");
    asm("TNZW X");
}

#else
#error "Invalid system clock value...."
#endif





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
#if ((SYS_CLK_FREQ == SYS_CLK_FREQ_2M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_4M) \
      || (SYS_CLK_FREQ == SYS_CLK_FREQ_8M) || (SYS_CLK_FREQ == SYS_CLK_FREQ_16M))
void delay_us_1(uint16_t n)
{
//��С��ʱ11us,22T
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
        //��ȥ�������÷���ռ��10T
asm
(
    "CPW X,#12\n"  //2T,
    "JRC LABEL1\r\n" //��ת��2T��������1T
    "SUBW X,#11\n" //2T,��ȥ11��ִ�е��ˣ����һ���Ǵ���0��

    "INCW X\n"  //1T����1����֤���µ�ѭ������������
    "SRLW X\n"  //2T,����2���Ա�֤֮���һ��ѭ������ʱ1us
    "NOP\n"     //����2T����֤��ȥѭ���̶�Ϊ20T����10us
    "NOP\n"

    //һ��ѭ��ռ4T
"LOOP:DECW X\n"   //1T
    "NOP\n"
    "JRNE LOOP\n"//��ת��2T������ת��1T
    "JP RETURN\n" //1T��ͬʱ��Ϊ����

    //��ʱ8T
"LABEL1:TNZW X\n"//2T  ������ֻռ1�ֽڿռ䣬��ʹ��ѭ����ʡ�ռ�
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    "TNZW X\n"//2T

"RETURN:"
);


//��С��ʱ4us,��20T
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
        //��ȥ�������÷���ռ��10T
asm
(
    "CPW X,#5\n"  //2T,X<5,
    "JRC LABEL1\r\n" //��ת��2T��������1T
    "SUBW X,#4\n" //2T,��ȥ4��ִ�е��ˣ����һ���Ǵ���0��

    //һ��ѭ��ռ4T
"LOOP:DECW X\n"   //1T
    "NOP\n"
    "JRNE LOOP\n"//��ת��2T������ת��1T
    
    //ִ�е���ʱ���ѻ�����ʱ��Ϊ5T + (X-4)*4T - 1T������Ҫ����ʱ2T
    "NOP\n" //1T
    "JP RETURN\n" //1T

    //��ʱ6T
"LABEL1:TNZW X\n"//2T  ������ֻռ1�ֽڿռ䣬��ʹ��ѭ����ʡ�ռ�
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    
"RETURN:"

);


//��С��ʱ3us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
        //��ȥ�������÷���ռ��10T
asm
(
    "CPW X,#4\n"  //2T,X<4
    "JRC LABEL1\r\n" //��ת��2T��������1T 
    "SUBW X,#3\n" //2T,��ȥ3��ִ�е��ˣ����һ���Ǵ���0��

    //��Ҫ����ʱ4T
    "SLLW X\n"  //2T����2��ʹ֮���ѭ���ﵽ8T

    //һ��ѭ��ռ4T��ǰ�����2�Ϳ��Դﵽ8T
"LOOP:DECW X\n"   //1T
    "NOP\n"
    "JRNE LOOP\n"//��ת��1T������ת��2T
    "NOP\n"
    
    //ִ�е���ʱ������Ҫ����ʱ9T
    "TNZW X\n"//2T  ������ֻռ1�ֽڿռ䣬��ʹ��ѭ����ʡ�ռ�
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    "TNZW X\n"//2T
    "JP RETURN\n" //1T

    //��ʱ10T
"LABEL1:LD A,#3\n"//1T
"LOOP2:DEC A\n"//1T��3*3-1=8
    "JRNE LOOP2\n"  //��ת2T������ת1T
    "NOP\n"   //�չ�10T
    
"RETURN:"

);

//��С��ʱ2us
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
    //��ȥ�������÷���ռ��10T
asm
(
    "CPW X,#3\n"  //2T,X<3,�����16T�󷵻�
    "JRC LABEL1\r\n" //��ת��2T��������1T 
    "DECW X\n" //1T
    "DECW X\n"  //1T����ȥ2��ִ�е��ˣ����һ���Ǵ���0��,�����ʹ��SUBWָ���ܽ�ʡ1���ֽڿռ�

    //��Ҫ����ʱ4T��ʹ�̶�ռ��Ϊ16T
    "SLLW X\n"  //2T����4��ʹ֮���ѭ���ﵽ16T
    "SLLW X\n"  //2T

    //ѭ��ռ4*X - 1T��ǰ�����4�Ϳ��Դﵽ16T
"LOOP:DECW X\n"  //1T
    "NOP\n"  //����1T
    "JRNE LOOP\n"//��ת��2T������ת��1T
    "NOP\n"  //����1T

    //ִ�е���ʱ���ѻ�����ʱ��Ϊ(X-2)*16T + 9T + 10T������Ҫ����ʱ13T
    "LABEL2:LD A,#3\n"//1T��
"LOOP3:DEC A\n"//1T��3��ѭ���ܵ�Ϊ3*3-1T=8T��ʵ���ѭ��ȷʵ��ռ��3T
    "JRNE LOOP3\n"  //��ת2T������ת1T
    
    "NOP\n" //ʵ��ֻ��Ҫ����1T
    
    
    "JP RETURN\n" //��������ת��������ˮ�ߣ�ֻ��1T��ֱ����ת������

    //��ʱ18T
"LABEL1:LD A,#6\n"//1T
"LOOP2:DEC A\n"//1T��ѭ���ܵ�Ϊ3*6-1T=17T
    "JRNE LOOP2\n"  //��ת2T������ת1T
    
"RETURN:"
);

#else
#error "No define system clock value...."
#endif
}
#endif

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
void delay_ms_1(uint16_t n)
{
    while(n--)
    {
        delay_us_1(1000);
    }
}

/*---------------------------------------------------------------------------*/

