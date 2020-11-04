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
#define VIB_TRIGGER_CNT         5   //用于设置100ms内振动有效次数阀值
#define VIB_NON_TRIGGER_CNT     10   //用于设置1000ms内振动无效次数阀值


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
volatile uint8_t vib_sta = 0;
volatile uint16_t vib_flag = 0;
volatile uint8_t vib_cnt = 0;  //用于记录10ms内的振动中断次数
volatile uint8_t vib_invalid_cnt = 0;  //用于记录无振动的次数
volatile int32_t vib_time_start = 0;


static soft_timet_t vib_timeout_timer = {0 , 0};
static soft_timet_t vib_delay_timer = {0 , 0};

static soft_timet_t user_app_timer = {0 , 0};

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name     :user_app_vib_detect_it 
*
* Desc     :在引脚中断中调用
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
        //空闲状态，等待中断触发
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
* Desc     :在大循环中调用
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
        //防止在中断中调用RTC初始化函数产生与延时模式相冲突的情况
        bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);  //打开rtc计时
        vib_cnt = 1;

        timer_set(&vib_timeout_timer , 10);  //启动100ms超时
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

            timer_set(&vib_timeout_timer , 10);  //重新开始新超时
            
            if(vib_cnt < VIB_TRIGGER_CNT)
            {
#if 0
                sim_printf_string("vib invalid,vib_cnt=");
                sim_printf_hex(vib_cnt);

                sim_printf_string(",vib_invalid_cnt=");
                sim_printf_hex(vib_invalid_cnt);
                sim_printf_string("\r\n");
#endif
                //未达到阀值，判断为无效振动
                vib_invalid_cnt++;
                
                if((vib_invalid_cnt > VIB_NON_TRIGGER_CNT) || (0 == vib_flag))
                {
//                    bsp_rtc_deinit();  //关闭RTC，以保证进入完整halt
                    
                    vib_invalid_cnt = 0;
                    vib_flag = 0;
                    vib_cnt = 0;
                    vib_time_cnt = 0;

                    vib_sta = 0;  //状态改变放在最后，防止前面发生中断导致设置异常
                }
            }
            else
            {
                if(0 == vib_flag)
                {
                    //开始计数时，启动定时器
                    //timer_set(&vib_delay_timer , 10);

                    vib_time_cnt = 0;
                }

                vib_flag++;
                if(vib_flag > 250)
                    vib_flag = 250;

                vib_cnt = 0;
                vib_invalid_cnt = 0;
                //timer_set(&vib_timeout_timer , 10);  //重新开始新超时

                //sim_printf_string("vib valid,and start new...\r\n");
            }

            //处理
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
                    VIB_IT_DISABLE();//关闭检测中断
					VIB_DISABLE();
                    //清相关变量标志，进入纯报警音播放模式
                    vib_invalid_cnt = 0;
                    vib_flag = 0;
                    vib_cnt = 0;
                    vib_time_cnt = 0;

                    vib_sta = 0;  //状态改变放在最后，防止前面发生中断导致设置异常
                    
                    sys_sta = E_SYS_STA_ALARM;  //切换到报警状态
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
    soft_timet_t adc_timer = {0 ,0};  //用于采样超时处理
    uint8_t flag = 0;
    uint16_t adc_vcc = 0;//采样内部参考电压来间接获取当前的VCC电压
    uint32_t adc_data;
    //LOWV_ENABLE();  //打开外部电源开关
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

    bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);  //启用RTC定时器
    
    for(i = 0;i < 4;i++)
    {
        /* Start ADC1 Conversion using Software trigger*/
        ADC_SoftwareStartConv(ADC1);

        timer_set(&adc_timer , 3);  //启动超时定时
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
        //发生超时，理论上不会出现
        sim_printf_string("ADC Timeout...\r\n");
        adc_data = 300;  //固定值，不报警

        goto EXIT;  //跳转到结束
    }
    else
    {
        //求平均
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
//        timer_set(&adc_timer , 1);  //启动超时定时
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
//        //发生超时，理论上不会出现
//        sim_printf_string("ADC Timeout...\r\n");
//        adc_data = 300;  //固定值，不报警
//    }
//    else
//    {
//        //求平均
//        adc_data >>= 2;
//
//        //通过Vref换算出电压
//        adc_data *= 1224;
//        adc_data /= adc_vcc;  
//        adc_data /= 10;      //得到电压值，,      外部2个100K分压，此值为实际电压的一半，单位10mV
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

    //LOWV_DISABLE();  //关闭外部电源开关

    return (uint16_t)adc_data;
}

/******************************************************************************
* Name     :user_app_run 
*
* Desc     :应用层处理函数，在大循环中调用
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
        //系统上电状态，延时50ms，等待上电提示音播放完成
        sim_printf_string("system power on,wait 50ms...\r\n");
        bsp_beep_play(E_BEEP_PLAY_PU);
        
        bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
        sys_sta = E_SYS_STA_PU;
    }
    else if(E_SYS_STA_PU == sys_sta)
    {
        //需要等待上电提示音播放结束
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
        //未布防，进入停机模式
        sys_sta = E_SYS_STA_HALT;
        
        //关振动检测中断
        
        VIB_IT_DISABLE();
		VIB_DISABLE();
		KEY_DISABLE();
        vib_flag = 0;
        vib_sta = 0;

        //关闭蜂鸣器
        bsp_beep_stop();

        sim_printf_string("no def,enter low power mode...\r\n");
    }
    else if(((E_SYS_STA_NO_DEF == sys_sta) || (E_SYS_STA_HALT == sys_sta)) && (1 == key_val))
    {
        //开始布防
        bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
        timer_set(&user_app_timer , 500);  //启动5秒定时
        
        //需要进行电量检测
        if(user_app_lowv() < 240)
        {
            sim_printf_string("low pwr...\r\n");
            
            bsp_beep_play(E_BEEP_PLAY_LOW_PWR);  //播放低电量提示
            sys_sta = E_SYS_STA_WAIT_DEF;
        }
        else
        {
            sim_printf_string("detect def,wait 5s for comfirm...\r\n");

            bsp_beep_play(E_BEEP_PLAY_DEF);  //播放布防提示
            sys_sta = E_SYS_STA_WAIT_DEF;
        }
    }
    else if((E_SYS_STA_WAIT_DEF == sys_sta) && (1 == key_val))
    {
        //等待5秒确认布防
        if(timer_expired(&user_app_timer))
        {
            //启用振动检测中断
			VIB_ENABLE();
			VIB_IT_ENABLE();
			
			
			
            sim_printf_string("确认布防成功\r\n");

            //播放布防成功提示音
            bsp_beep_play(E_BEEP_PLAY_DEF_OK);

            sys_sta = E_SYS_STA_DEF;
        }
        
    }
    else if(E_SYS_STA_ALARM == sys_sta)
    {
        //需要等待报警音播放结束，恢复到布防状态
        if(E_BEEP_PLAY_NONE == beep_play_type)
        {
            sys_sta = E_SYS_STA_DEF;
			VIB_ENABLE();
            VIB_IT_ENABLE();
        }
    }
    else if(E_SYS_STA_DEF == sys_sta)
    {
        //正常布防状态
		KEY_ENABLE();
		user_app_vib_detect_run();
    }
    else
    {
        //停机模式，无动作
    }
}

/*---------------------------------------------------------------------------*/

