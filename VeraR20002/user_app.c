/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :user_app.c 
* Desc     :
* 
* 
* Author   :CenLinbo
* Date     :2020/09/18
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/09/18, CenLinbo create this file
*         
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "user_app.h"
#include "delay.h"
#include "timer_set.h"

/*------------------- Global Definitions and Declarations -------------------*/
#define VIB_TRIGGER_CNT         5   //��������100ms������Ч������ֵ
#define VIB_NON_TRIGGER_CNT     10   //��������1000ms������Ч������ֵ


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
volatile uint8_t vib_sta = 0;
volatile uint16_t vib_flag = 0;
volatile uint8_t vib_cnt = 0;  //���ڼ�¼10ms�ڵ����жϴ���
volatile uint8_t vib_invalid_cnt = 0;  //���ڼ�¼���񶯵Ĵ���
volatile int32_t vib_time_start = 0;


static soft_timet_t vib_timeout_timer = {0 , 0};
static soft_timet_t vib_delay_timer = {0 , 0};

static soft_timet_t user_app_timer = {0 , 0};

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name     :user_app_vib_detect_it 
*
* Desc     :�������ж��е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/18, Create this function by CenLinbo
 ******************************************************************************/
void user_app_vib_detect_it(void)
{
    switch(vib_sta)
    {
        //����״̬���ȴ��жϴ���
        case 0:
            vib_sta = 1;
            break;
        
        case 2:
            vib_cnt++;
            if(vib_cnt > 250)
                vib_cnt = 250;
            sim_uart_printf_it('A');
            sim_uart_printf_it(' ');
            break;
    }
}

/******************************************************************************
* Name     :user_app_vib_detect_run 
*
* Desc     :�ڴ�ѭ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/18, Create this function by CenLinbo
 ******************************************************************************/
void user_app_vib_detect_run(void)
{
    static uint16_t vib_time_cnt = 0;
    
    if(1 == vib_sta)
    {
        //��ֹ���ж��е���RTC��ʼ��������������ʱģʽ���ͻ�����
        bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);  //��rtc��ʱ
        vib_cnt = 1;

        timer_set(&vib_timeout_timer , 10);  //����100ms��ʱ
        sim_printf_string("vib on\r\n");

        vib_sta = 2;
    }
    else if(2 == vib_sta)
    {
        if(timer_expired(&vib_timeout_timer))
        {
            sim_uart_printf('e');
            sim_printf_hex(vib_cnt);
            sim_printf_hex(vib_invalid_cnt);

            timer_set(&vib_timeout_timer , 10);  //���¿�ʼ�³�ʱ
            
            if(vib_cnt < VIB_TRIGGER_CNT)
            {
#if 0
                sim_printf_string("vib invalid,vib_cnt=");
                sim_printf_hex(vib_cnt);

                sim_printf_string(",vib_invalid_cnt=");
                sim_printf_hex(vib_invalid_cnt);
                sim_printf_string("\r\n");
#endif
                //δ�ﵽ��ֵ���ж�Ϊ��Ч��
                vib_invalid_cnt++;
                
                if((vib_invalid_cnt > VIB_NON_TRIGGER_CNT) || (0 == vib_flag))
                {
//                    bsp_rtc_deinit();  //�ر�RTC���Ա�֤��������halt
                    
                    vib_invalid_cnt = 0;
                    vib_flag = 0;
                    vib_cnt = 0;
                    vib_time_cnt = 0;

                    vib_sta = 0;  //״̬�ı������󣬷�ֹǰ�淢���жϵ��������쳣
                }
            }
            else
            {
                if(0 == vib_flag)
                {
                    //��ʼ����ʱ��������ʱ��
                    //timer_set(&vib_delay_timer , 10);

                    vib_time_cnt = 0;
                }

                vib_flag++;
                if(vib_flag > 250)
                    vib_flag = 250;

                vib_cnt = 0;
                vib_invalid_cnt = 0;
                //timer_set(&vib_timeout_timer , 10);  //���¿�ʼ�³�ʱ

                //sim_printf_string("vib valid,and start new...\r\n");
            }

            //����
            {
                vib_time_cnt++;

                if((10 == vib_time_cnt) && vib_flag)
                {
                    sim_printf_string("vib valid,play tips...\r\n");
                    
                    bsp_beep_play(E_BEEP_PLAY_TIPS);
                    //bsp_beep_play(E_BEEP_PLAY_ALARM);
                    //bsp_beep_play(E_BEEP_PLAY_LOW_PWR);

                }
                else if((50 == vib_time_cnt) && vib_flag)
                {
                    
                    sim_printf_string("vib continue,play alarm...\r\n");
                    bsp_beep_play(E_BEEP_PLAY_ALARM);
                    VIB_IT_DISABLE();//�رռ���ж�
					VIB_DISABLE();
                    //����ر�����־�����봿����������ģʽ
                    vib_invalid_cnt = 0;
                    vib_flag = 0;
                    vib_cnt = 0;
                    vib_time_cnt = 0;

                    vib_sta = 0;  //״̬�ı������󣬷�ֹǰ�淢���жϵ��������쳣
                    
                    sys_sta = E_SYS_STA_ALARM;  //�л�������״̬
                }
            }

            sim_printf_string("vib_sta:");
            sim_printf_hex(vib_sta);
        }

        
    }
}

uint16_t user_app_lowv(void)
{
    
    uint8_t i;
    soft_timet_t adc_timer = {0 ,0};  //���ڲ�����ʱ����
    uint8_t flag = 0;
    uint16_t adc_vcc = 0;//�����ڲ��ο���ѹ����ӻ�ȡ��ǰ��VCC��ѹ
    uint32_t adc_data;
    //LOWV_ENABLE();  //���ⲿ��Դ����
    delay_ms_1(10);
    /* Enable ADC1 clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
	/* Disable ADC1 clock */
    //CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
    /* Initialize and configure ADC1 */
	ADC_VrefintCmd(ENABLE);
    ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_12Bit, ADC_Prescaler_2);
	
    /* ADC channel used for IDD measurement */
    ADC_SamplingTimeConfig(ADC1, ADC_Group_FastChannels , ADC_SamplingTime_96Cycles);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);



    /* Enable ADC1 Channel used for Vref measurement */
    //ADC_ChannelCmd(ADC1, LOWV_ADC_CH , ENABLE);
    ADC_ChannelCmd(ADC1 , ADC_Channel_Vrefint , ENABLE);

    bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);  //����RTC��ʱ��
    
    for(i = 0;i < 4;i++)
    {
        /* Start ADC1 Conversion using Software trigger*/
        ADC_SoftwareStartConv(ADC1);

        timer_set(&adc_timer , 3);  //������ʱ��ʱ
        flag = 0;

        /* Wait until End-Of-Convertion */
        do
        {
            flag = timer_expired(&adc_timer);
        }
        while ((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0) && (0 == flag));

        if(1 == flag)
        {
            break;
        }
        else
        {
            /* Get conversion value */
			//uint16_t adc_value;
			
            adc_vcc += ADC_GetConversionValue(ADC1);
//			adc_value = ADC_GetConversionValue(ADC1);
//			sim_printf_string("Value=");
//        	sim_printf_hex((uint8_t)(adc_value >> 8));
//        	sim_printf_hex((uint8_t)(adc_value & 0xFF));
//        	sim_printf_string("\r\n");
        }
    }

    if(i < 4)
    {
        //������ʱ�������ϲ������
        sim_printf_string("ADC Timeout...\r\n");
        adc_data = 300;  //�̶�ֵ��������

        goto EXIT;  //��ת������
    }
    else
    {
        //��ƽ��
        adc_vcc >>= 2;
		adc_data = 501350; 
		adc_data /= adc_vcc;        
        

//        sim_printf_string("Vref adc=");
//        sim_printf_hex((uint8_t)(adc_vcc >> 8));
//        sim_printf_hex((uint8_t)(adc_vcc & 0xFF));
//        sim_printf_string("\r\n");
		
    }

	
	
//	ADC_ChannelCmd(ADC1, LOWV_ADC_CH , ENABLE);
//    adc_data = 0;
//    
//	for(i = 0;i < 4;i++)
//	{
//    
//        /* Start ADC1 Conversion using Software trigger*/
//        ADC_SoftwareStartConv(ADC1);
//
//        timer_set(&adc_timer , 1);  //������ʱ��ʱ
//        flag = 0;
//
//        /* Wait until End-Of-Convertion */
//        do
//        {
//            flag = timer_expired(&adc_timer);
//        }
//        while ((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0) && (0 == flag));
//
//        if(1 == flag)
//        {
//            break;
//        }
//        else
//        {
//            /* Get conversion value */
//            adc_data += ADC_GetConversionValue(ADC1);
//        }
//		}
//	
//    if(i < 4)
//    {
//        //������ʱ�������ϲ������
//        sim_printf_string("ADC Timeout...\r\n");
//        adc_data = 300;  //�̶�ֵ��������
//    }
//    else
//    {
//        //��ƽ��
//        adc_data >>= 2;
//
//        //ͨ��Vref�������ѹ
//        adc_data *= 1224;
//        adc_data /= adc_vcc;  
//        adc_data /= 10;      //�õ���ѹֵ��,      �ⲿ2��100K��ѹ����ֵΪʵ�ʵ�ѹ��һ�룬��λ10mV
//
//        sim_printf_string("1/2*batt vol=");
//        sim_printf_hex((uint8_t)adc_data);
//        sim_printf_string("\r\n");
//    }
	

EXIT:
    ADC_VrefintCmd(DISABLE);
	/* Enable ADC1 Channel used for BATT measurement */
    ADC_ChannelCmd(ADC1 , ADC_Channel_Vrefint , DISABLE);

    /* DeInitialize ADC1 */
    ADC_DeInit(ADC1);

    /* Disable ADC1 clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);

    //LOWV_DISABLE();  //�ر��ⲿ��Դ����

    return (uint16_t)adc_data;
}

/******************************************************************************
* Name     :user_app_run 
*
* Desc     :Ӧ�ò㴦�������ڴ�ѭ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
void user_app_run(void)
{
    if(E_SYS_STA_NONE == sys_sta)
    {
        //ϵͳ�ϵ�״̬����ʱ50ms���ȴ��ϵ���ʾ���������
        sim_printf_string("system power on,wait 50ms...\r\n");
        bsp_beep_play(E_BEEP_PLAY_PU);
        
        bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
        sys_sta = E_SYS_STA_PU;
    }
    else if(E_SYS_STA_PU == sys_sta)
    {
        //��Ҫ�ȴ��ϵ���ʾ�����Ž���
        if(E_BEEP_PLAY_NONE == beep_play_type)
        {
            sim_printf_string("system power on ok...\r\n");
            sys_sta = E_SYS_STA_NO_DEF;
        }
    }
    else if(((E_SYS_STA_NO_DEF == sys_sta) || (E_SYS_STA_DEF == sys_sta) 
        || (E_SYS_STA_WAIT_DEF == sys_sta) || (E_SYS_STA_ALARM == sys_sta)) 
        && (0 == key_val))
    {
        //δ����������ͣ��ģʽ
        sys_sta = E_SYS_STA_HALT;
        
        //���񶯼���ж�
        
        VIB_IT_DISABLE();
		VIB_DISABLE();
		KEY_DISABLE();
        vib_flag = 0;
        vib_sta = 0;

        //�رշ�����
        bsp_beep_stop();

        sim_printf_string("no def,enter low power mode...\r\n");
    }
    else if(((E_SYS_STA_NO_DEF == sys_sta) || (E_SYS_STA_HALT == sys_sta)) && (1 == key_val))
    {
        //��ʼ����
        bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
        timer_set(&user_app_timer , 500);  //����5�붨ʱ
        
        //��Ҫ���е������
        if(user_app_lowv() < 240)
        {
            sim_printf_string("low pwr...\r\n");
            
            bsp_beep_play(E_BEEP_PLAY_LOW_PWR);  //���ŵ͵�����ʾ
            sys_sta = E_SYS_STA_WAIT_DEF;
        }
        else
        {
            sim_printf_string("detect def,wait 5s for comfirm...\r\n");

            bsp_beep_play(E_BEEP_PLAY_DEF);  //���Ų�����ʾ
            sys_sta = E_SYS_STA_WAIT_DEF;
        }
    }
    else if((E_SYS_STA_WAIT_DEF == sys_sta) && (1 == key_val))
    {
        //�ȴ�5��ȷ�ϲ���
        if(timer_expired(&user_app_timer))
        {
            //�����񶯼���ж�
			VIB_ENABLE();
			VIB_IT_ENABLE();
			
			
			
            sim_printf_string("ȷ�ϲ����ɹ�\r\n");

            //���Ų����ɹ���ʾ��
            bsp_beep_play(E_BEEP_PLAY_DEF_OK);

            sys_sta = E_SYS_STA_DEF;
        }
        
    }
    else if(E_SYS_STA_ALARM == sys_sta)
    {
        //��Ҫ�ȴ����������Ž������ָ�������״̬
        if(E_BEEP_PLAY_NONE == beep_play_type)
        {
            sys_sta = E_SYS_STA_DEF;
			VIB_ENABLE();
            VIB_IT_ENABLE();
        }
    }
    else if(E_SYS_STA_DEF == sys_sta)
    {
        //��������״̬
		KEY_ENABLE();
		user_app_vib_detect_run();
    }
    else
    {
        //ͣ��ģʽ���޶���
    }
}

/*---------------------------------------------------------------------------*/

