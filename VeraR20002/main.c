/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :main.c 
* Desc     :�����ɲ����Ŀ
* 
* 
* Author   :CenLinbo
* Date     :2020/08/17
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/17, CenLinbo create this file
*         
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "main.h"
#include "delay.h"
#include "timer_set.h"
#include "user_app.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/
enum
{
    E_RTC_MODE_STOP = 0,
    E_RTC_MODE_ONLY_DELAY = 0x01,
    E_RTC_MODE_ONLY_COUNT = 0x02,
    E_RTC_MODE_DELAY_CONUT = 0x03,
    E_RTC_MODE_HALT = 0x04,  //ͣ��ģʽ
};


#define BEEP_PLAY_PU_TIME   500000
#define BEEP_PLAY_PU_TIPS   500000

#define BEEP_PLAY_INTERVAL  10   //������ʱ�����ʱ������λΪ10ms

/*----------------------- Variable Declarations -----------------------------*/
//ϵͳʱ��ֵ�������ϵ�Ĭ��ֵΪ2M���������������ʱ�ӣ����ֵ��ͬ������
uint32_t sys_clk_freq_val = SYS_CLK_FREQ_2M;

uint32_t sys_base_clock_cnt = 0;

uint8_t rtc_mode = E_RTC_MODE_STOP;
volatile uint16_t rtc_delay_cnt = 0;  //����ʹ��RTC��ʱʱ�ļ���ֵ��������0���ʾ��ʱ��


soft_timet_t timer_test;

uint8_t sys_sta = E_SYS_STA_NONE;  //���ڼ�¼��ǰ�Ĳ�����״̬


uint8_t key_val = 0;  //���ڼ�¼����ֵ��0��δ�ͷţ�1������
volatile static uint8_t key_trigger_flag = 0;  //���ڱ�ǰ����ж�


volatile uint8_t global_interrupt_flag = 0;  //���ڼ�¼��ǰ�����жϱ�־

volatile uint8_t beep_play_sta = E_BEEP_PLAY_STA_NONE;  //��¼��ǰ�Ĳ���״̬
volatile uint8_t beep_play_type = E_BEEP_PLAY_NONE;
volatile uint32_t beep_play_time = 0;  //��¼Ҫ���ʱ��
volatile uint16_t beep_play_cnt = 0;  //���ڼ�¼Ҫ�ظ����ŵĴ���
//volatile uint8_t beep_pause_flag = 0;  //���ڼ�¼�͵�����ʾ�ı���״̬�� 0->200ms, 1->20ms
volatile uint8_t beep_play_section_cnt = 0; //�����ڲ��ű�����ʱ��¼��ǰ����
uint8_t beep_play_section_time = 250;
uint8_t beep_alarm_time_dir = 0;  //���ڼ�¼���ʱ�������ӻ��ǵݼ���0�����ӣ�1���ݼ�

static soft_timet_t beep_timer = {0 , 0};
static uint8_t beep_play_pause_time = 0;  //�������ظ�����ʱ����¼��ͣʱ������λΪ10ms

//const uint8_t beep_play_period[E_BEEP_PLAY_END] = {0 , 172 , 248 , 168 , 168 , 168 , 250};
const uint8_t beep_play_period[E_BEEP_PLAY_END] = {0 , 198 , 198 , 198 , 198 , 198 , 250};
//const uint32_t beep_play_time_const[E_BEEP_PLAY_END] = {0 , 35000 , 25000 , 415000 , 400000 , 400000 , 10000};

const uint32_t beep_play_time_const[E_BEEP_PLAY_END] = {0 , 35000 , 25000 , 415000 , 400000 , 50000 , 10000};
const uint32_t beep_alarm_section_time_const[4] = {8000 , 8000 , 8000 , 8000};
const uint8_t beep_alarm_section_period[4] = {250 , 200 , 150 , 100};



/*----------------------- Function Prototype --------------------------------*/

/******************************************************************************
* Name     :bsp_io_init 
*
* Desc     :�Ա�ϵͳҪʹ�õ�GPIO�ڽ�����Ӧ�ĳ�ʼ��
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/25, Create this function by CenLinbo
 ******************************************************************************/
void bsp_io_init(void);

void bsp_beep_play_run(void);


void bsp_pwr_manager(void);

static void bsp_key_run(void);



/*----------------------- Function Implement --------------------------------*/
int main( void )
{
    //ϵͳʱ�ӳ�ʼ��
    bsp_system_clock_init(SYS_CLK_FREQ);

    //FWU��ULPλ�����ڵȴ��ڲ��ο���ѹ�ȶ�(VREFINTFλΪ1ʱ)�����޸ģ�
    //�˴�Ӧ�����ж��ڲ��ο���ѹ�ȶ������޸�FWU��ULPλ
    //��3λ����PWR_CSR2�Ĵ�����
    while(RESET == PWR_GetFlagStatus(PWR_FLAG_VREFINTF));  //bob ��ӣ��ȴ��ڲ��ο���ѹ�ȶ�
    PWR_FastWakeUpCmd(DISABLE);
    PWR_UltraLowPowerCmd(ENABLE);

    bsp_io_init();

    sim_printf_string("pritnf is ok...\r\n");

    //bsp_timer4_init();

    //halt();

    BSP_ENABLE_INTERRUPT();  //ʹ�����ж�

    

    bsp_beep_play(E_BEEP_PLAY_PU);

    key_trigger_flag = 1;  //�ֶ���1����֤�ϵ����ִ��һ�ΰ���ֵ��ȡ
    
    while(1)
    {
#if 0
        if(timer_expired(&timer_test))
        {
            delay_ms_1(1000);
            timer_set(&timer_test , 500);
        }
        else
        {
            delay_us_1(142);
            BUZZER_Port->ODR |= BUZZER_Pin;
            delay_us_1(118);
            BUZZER_Port->ODR &= ~BUZZER_Pin;
        }
#endif

        bsp_key_run();

        user_app_run();

        bsp_beep_play_run();

        bsp_pwr_manager();
    }
}

/******************************************************************************
* Name     :bsp_system_clock_init 
*
* Desc     :�������õ�ϵͳʱ�Ӻ궨�壬��ʼ��CPUʱ�����ã�����BOOT_ROM���ģ���ʱ�������⣬
            ��������ģ��ʱ������Ϊ�ر�
* Param_in :sys_clk:��Ҫʹ�õ�ϵͳʱ�ӣ��μ�ö�����Ͷ���
* Param_out:
* Return   :
* Global   :
* Note     :1������Ŀ�̶�ʹ������HSI 16MHzΪʱ��Դ����ϵͳ��λ��Ĭ�ϼ�ʹ�ô�ʱ��Դ����˲�
            �ٵ���ѡ��ʱ��Դ
            2������Ҫ�޸�ϵͳʱ��ʱ�������ͨ�����ô˺��������޸ģ��Ա�֤ϵͳʱ�ӱ���ֵͬ������
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/03/28, Create this function by CenLinbo
 ******************************************************************************/
void bsp_system_clock_init(uint32_t sys_clk)
{
    CLK_HSICmd(ENABLE);//��ʼ�ڲ���ƵRC 16M
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);//HSIΪϵͳʱ��

    //����
    if(SYS_CLK_FREQ_16M == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);//����Ƶ����1��Ƶ
        sys_clk_freq_val = SYS_CLK_FREQ_16M;
    }
    else if(SYS_CLK_FREQ_8M == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_2);
        sys_clk_freq_val = SYS_CLK_FREQ_8M;
    }
    else if(SYS_CLK_FREQ_4M == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_4);
        sys_clk_freq_val = SYS_CLK_FREQ_4M;
    }
    else if(SYS_CLK_FREQ_2M == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_8);
        sys_clk_freq_val = SYS_CLK_FREQ_2M;
    }
    else if(SYS_CLK_FREQ_1M == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_16);
        sys_clk_freq_val = SYS_CLK_FREQ_1M;
    }
    else if(SYS_CLK_FREQ_500K == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_32);
        sys_clk_freq_val = SYS_CLK_FREQ_500K;
    }
    else if(SYS_CLK_FREQ_250K == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_64);
        sys_clk_freq_val = SYS_CLK_FREQ_250K;
    }
    else
    {
        //������ݽ����Ĳ�����Ч�������κ��޸�
    }
}

/******************************************************************************
* Name     :bsp_get_system_clock_val 
*
* Desc     :��ȡϵͳʱ��Ƶ��ֵ
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/18, Create this function by CenLinbo
 ******************************************************************************/
uint32_t bsp_get_system_clock_val(void)
{
    return sys_clk_freq_val;
}

/******************************************************************************
* Name     :bsp_timer4_init 
*
* Desc     :����TIM4Ϊ1ms�ж�һ�Σ���Ϊϵͳʱ��
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/22, Create this function by CenLinbo
 ******************************************************************************/
void bsp_timer4_init(void)
{
    TIM4_DeInit();  //�Ȼָ���Ĭ��ֵ

    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , ENABLE);

    //����Ϊ1m���һ��
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
//    TIM4_TimeBaseInit(TIM4_Prescaler_16 , 124);
		TIM4_TimeBaseInit(TIM4_Prescaler_128 , 124);	  	
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
        TIM4_TimeBaseInit(TIM4_Prescaler_64 , 124);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
            TIM4_TimeBaseInit(TIM4_Prescaler_32 , 124);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
            TIM4_TimeBaseInit(TIM4_Prescaler_16 , 124);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_1M)
            TIM4_TimeBaseInit(TIM4_Prescaler_8 , 124);
#else
#error "Invalid system clock value...."
#endif

    TIM4_ARRPreloadConfig(ENABLE);  //�����Զ�װ�ع���

    TIM4_ClearFlag(TIM4_FLAG_Update);

    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);

    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);
}

void bsp_timer4_init_2(uint8_t period)
{
    TIM4_DeInit();  //�Ȼָ���Ĭ��ֵ

    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , ENABLE);

    //����Ϊ1us���һ��
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
    TIM4_TimeBaseInit(TIM4_Prescaler_16 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
        TIM4_TimeBaseInit(TIM4_Prescaler_8 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
            TIM4_TimeBaseInit(TIM4_Prescaler_4 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
            TIM4_TimeBaseInit(TIM4_Prescaler_2 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_1M)
            TIM4_TimeBaseInit(TIM4_Prescaler_1 , period);
#else
#error "Invalid system clock value...."
#endif

    TIM4_ARRPreloadConfig(ENABLE);  //�����Զ�װ�ع���

    TIM4_ClearFlag(TIM4_FLAG_Update);

    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);

    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);
}

/******************************************************************************
* Name     :bsp_timer4_reload 
*
* Desc     :�ڲ���Ҫֹͣ��ʱ��ֱ�Ӹ����м���ʱ����
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/24, Create this function by CenLinbo
 ******************************************************************************/
void bsp_timer4_reload(uint8_t period)
{
#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
    TIM4_TimeBaseInit(TIM4_Prescaler_16 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_8M)
        TIM4_TimeBaseInit(TIM4_Prescaler_8 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
            TIM4_TimeBaseInit(TIM4_Prescaler_4 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_2M)
            TIM4_TimeBaseInit(TIM4_Prescaler_2 , period);
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_1M)
            TIM4_TimeBaseInit(TIM4_Prescaler_1 , period);
#else
#error "Invalid system clock value...."
#endif
}

/******************************************************************************
* Name     :bsp_get_clock 
*
* Desc     :��ȡϵͳ�δ�ʱ��ֵ
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/22, Create this function by CenLinbo
 ******************************************************************************/
uint32_t bsp_get_clock(void)
{
    return sys_base_clock_cnt;
}


/******************************************************************************
* Name     :bsp_sys_clock_inc 
*
* Desc     :ϵͳ�δ�ʱ��ֵ��1���ڶ�ʱ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/22, Create this function by CenLinbo
 ******************************************************************************/
void bsp_sys_clock_inc(void)
{
    sys_base_clock_cnt++;
}

/******************************************************************************
* Name     :bsp_io_init 
*
* Desc     :�Ա�ϵͳҪʹ�õ�GPIO�ڽ�����Ӧ�ĳ�ʼ��
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/25, Create this function by CenLinbo
 ******************************************************************************/
void bsp_io_init(void)
{
    BSP_DISABLE_INTERRUPT();
    EXTI_SetPinSensitivity(VIB_SENSOR_EXIT_Pin , EXTI_Trigger_Falling);  //����Ϊ�����½��ض�����
    ITC_SetSoftwarePriority(EXTI3_IRQn , ITC_PriorityLevel_2);  //����Ϊ�����ȼ�����ֹ���ŷ������ж�

    EXTI_SetPinSensitivity(KEY_EXIT_Pin , EXTI_Trigger_Rising_Falling);//�������ж�ʱ�����޸�
    
    GPIO_Init(KEY_GPIO_Port , KEY_Pin , GPIO_Mode_In_PU_IT);
    
    //V1.0
    GPIO_Init(VIB_SENSOR_Port , VIB_SENSOR_Pin , GPIO_Mode_In_FL_No_IT);  //���գ���ʼ״̬�����ж�
	
	GPIO_Init(VIB_EN_Port , VIB_EN_Pin , GPIO_Mode_Out_PP_Low_Slow);  //VIB��ʼʹ�ܹ�
	/********��·�䶯 �޸�**********/
	/*
    GPIO_Init(LOWV_EN_Port , LOWV_EN_Pin , GPIO_Mode_Out_PP_High_Slow);  
    GPIO_Init(LOWV_ADC_Port , LOWV_ADC_Pin , GPIO_Mode_In_FL_No_IT);
	*/
	/*******************************/
	GPIO_Init(LOWV_EN_Port , LOWV_EN_Pin , GPIO_Mode_Out_PP_High_Slow);  
    GPIO_Init(LOWV_ADC_Port , LOWV_ADC_Pin , GPIO_Mode_Out_PP_High_Slow);
	
    GPIO_Init(DC_EN_Port , DC_EN_Pin , GPIO_Mode_Out_PP_Low_Slow);

    GPIO_Init(BUZZER_Port , BUZZER_Pin , GPIO_Mode_Out_PP_Low_Slow);

    //ģ�⴮��IO�ڳ�ʼ��
    GPIO_Init(SIM_DEBUG_Port , SIM_DEBUG_Pin , GPIO_Mode_Out_PP_High_Slow);

    //δʹ�õ����ų�ʼ��Ϊ�ڲ���������
    GPIO_Init(GPIOB , PB_UNUSED_Pin , GPIO_Mode_In_PU_No_IT);
    GPIO_Init(GPIOC , PC_UNUSED_Pin , GPIO_Mode_In_PU_No_IT);
}

/******************************************************************************
* Name     :bsp_rtc_init 
*
* Desc     :����RTCΪ�Զ�����
* Param_in :֧��1ms��10ms��1000ms
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/16, Create this function by CenLinbo
 ******************************************************************************/
void bsp_rtc_init(uint16_t wakeup_period)
{
    //�����ʱ���Ѿ����ˣ���ֱ���˳� 
    if(E_RTC_MODE_ONLY_COUNT & rtc_mode)
        return;
    
    /* Enable RTC clock */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
    /* Wait for LSE clock to be ready */
    while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
  
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC , ENABLE);  //������ʱ��
    RTC_WakeUpCmd(DISABLE);
    
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

    if(E_RTC_WAKEUP_PERIOD_10MS == wakeup_period)
        RTC_SetWakeUpCounter(190);  //10ms�ж�
    else if(E_RTC_WAKEUP_PERIOD_1000MS == wakeup_period)
        RTC_SetWakeUpCounter(19000);  //1000ms�ж�
    else if(E_RTC_WAKEUP_PERIOD_1MS == wakeup_period)
        RTC_SetWakeUpCounter(19);  //1ms�ж�

    RTC_ClearITPendingBit(RTC_IT_WUT);
    ITC_SetSoftwarePriority(RTC_CSSLSE_IRQn , ITC_PriorityLevel_2);  //����Ϊ�����ȼ�
    RTC_ITConfig(RTC_IT_WUT , ENABLE);

    RTC_WakeUpCmd(ENABLE);

    rtc_mode = E_RTC_MODE_ONLY_COUNT;
}

/******************************************************************************
* Name     :bsp_rtc_halt 
*
* Desc     :����RTCΪ1000ms�Զ�����һ�Σ���ϵͳ��Ҫ�ر�RTC����ͣ��ģʽǰ����
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/23, Create this function by CenLinbo
 ******************************************************************************/
void bsp_rtc_halt(void)
{
    //���ⷴ��������
    if(E_RTC_MODE_HALT == rtc_mode)
        return;
    
    /* Enable RTC clock */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
    /* Wait for LSE clock to be ready */
    while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
  
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC , ENABLE);  //������ʱ��
    RTC_WakeUpCmd(DISABLE);
    
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

    RTC_SetWakeUpCounter(19000);  //1000ms�ж�

    RTC_ClearITPendingBit(RTC_IT_WUT);
    ITC_SetSoftwarePriority(RTC_CSSLSE_IRQn , ITC_PriorityLevel_2);  //����Ϊ�����ȼ�
    RTC_ITConfig(RTC_IT_WUT , ENABLE);

    RTC_WakeUpCmd(ENABLE);

    rtc_mode = E_RTC_MODE_HALT;
    rtc_delay_cnt = 0;
}


/******************************************************************************
* Name     :bsp_rtc_deinit 
*
* Desc     :����RTC
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/16, Create this function by CenLinbo
 ******************************************************************************/
void bsp_rtc_deinit(void)
{
    RTC_WakeUpCmd(DISABLE);

    RTC_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC , DISABLE);  //�ر�����ʱ��
    CLK_RTCClockConfig(CLK_RTCCLKSource_Off, CLK_RTCCLKDiv_1);

    rtc_mode = E_RTC_MODE_STOP;
    rtc_delay_cnt = 0;
}

/******************************************************************************
* Name     :bsp_delay_ms_by_rtc 
*
* Desc     :ʹ��RTC����ms����ʱ���ɸ��ݵ�ǰRTC��ģʽѡ���Ӧ�ķ�ʽ���Խ�ʡ����
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/17, Create this function by CenLinbo
 ******************************************************************************/
void bsp_delay_ms_by_rtc(uint16_t n)
{
    if(0 == n)
        return;
    
    rtc_mode |= E_RTC_MODE_ONLY_DELAY;
    if(E_RTC_MODE_ONLY_COUNT & rtc_mode)
    {
        rtc_delay_cnt = n + 1;  //������1�Ļ�����ʱʱ�����ʵ����ʱ��0~10ms����������ʵ�ʳ�0~10ms

        while(rtc_delay_cnt)
        {
            halt();
        }
    }
    else
    {
        //RTCδʹ��ʱ��ֱ�ӵ��õ�����ʱ
        delay_ms_2(n);
    }

    rtc_mode &= ~E_RTC_MODE_ONLY_DELAY;
}


/******************************************************************************
* Name     :bsp_rtc_it_handler 
*
* Desc     :RTC�Զ������ж��е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/17, Create this function by CenLinbo
 ******************************************************************************/
void bsp_rtc_it_handler(void)
{
    if(E_RTC_MODE_ONLY_DELAY == rtc_mode)
    {
        rtc_delay_flag = 0;
    }
    else
    {
        if(rtc_delay_cnt > 0)
        {
            rtc_delay_cnt--;
        }

        if(E_RTC_MODE_ONLY_COUNT & rtc_mode)
        {
            bsp_sys_clock_inc();
            //sim_uart_printf_it('T');
        }
    }
}

void bsp_beep_play(uint8_t type)
{
    //ͬ���������ڲ���ʱ�������ж�
    if((type == beep_play_type) && (E_BEEP_PLAY_STA_NONE != beep_play_sta))
    {
        return;
    }
	DC_ENABLE();
	delay_us_1(120);
    beep_play_type = type;
    beep_play_time = beep_play_time_const[type];
    bsp_timer4_init_2(beep_play_period[type]);

    beep_play_sta = E_BEEP_PLAY_STA_FIRST;
}

/******************************************************************************
* Name     :bsp_beep_replay 
*
* Desc     :���¼��ز��Ų�������Ҫ�ظ�����ʱ�ŵ��ã�������������²��������
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/23, Create this function by CenLinbo
 ******************************************************************************/
void bsp_beep_replay(uint8_t type)
{
    beep_play_type = type;
    beep_play_time = beep_play_time_const[type];
    bsp_timer4_init_2(beep_play_period[type]);
}


void bsp_beep_play_run(void)
{
    //������Ҫ�ظ����ŵ�����
    switch(beep_play_type)
    {
        case E_BEEP_PLAY_NONE:
            break;
            
        //��Ҫ�������ŵ���ʾ���͵͵���������
        case E_BEEP_PLAY_TIPS:
        case E_BEEP_PLAY_LOW_PWR:
            if(E_BEEP_PLAY_STA_REPEAT == beep_play_sta)
            {
                //�״β��ţ���Ҫ������ʱ������һ��
                if(0 == beep_play_cnt)
                {
                    //�������������ϲ�Ӧ��ִ�е��˴�
                    sim_printf_string("code err...\r\n");
                    beep_play_sta = E_BEEP_PLAY_STA_NONE;
                    beep_play_type = E_BEEP_PLAY_NONE;
                }
                else
                {
                    beep_play_cnt--;

                    bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
                    timer_set(&beep_timer , beep_play_pause_time);  //150ms
                    beep_play_sta = E_BEEP_PLAY_STA_PAUSE;
                }
            }
            else if(E_BEEP_PLAY_STA_PAUSE == beep_play_sta)
            {
                if(timer_expired(&beep_timer))
                {
                    //�ȱ��״̬
                    if(0 == beep_play_cnt)
                    {
                        beep_play_sta = E_BEEP_PLAY_STA_LAST;
                    }
                    else
                    {
                        beep_play_sta = E_BEEP_PLAY_STA_REPLAY;
                    }

                    sim_printf_hex(beep_play_cnt);
                    bsp_beep_replay(beep_play_type);
                }
            }

            break;
		
        default:
            break;
    }
}

/******************************************************************************
* Name     :bsp_beep_stop 
*
* Desc     :�������ģʽʱ���رշ�������ص�����ͱ���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
void bsp_beep_stop(void)
{
    TIM4_DeInit();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
    BUZZER_CLOSE();
	DC_DISABLE();
    beep_play_type = E_BEEP_PLAY_NONE;
    beep_play_sta = E_BEEP_PLAY_STA_NONE;
}

/******************************************************************************
* Name     :bsp_key_run 
*
* Desc     :������⺯�����ڴ�ѭ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
static void bsp_key_run(void)
{
    static soft_timet_t key_timer = {0 , 0};
    
    if(!key_trigger_flag)
        return;//���ް����жϣ�������

    switch(key_trigger_flag)
    {
        case 1:
            if((!KEY_READ()) && (0 == key_val))
            {
                sim_printf_string("key_detect...\r\n");

                bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
                timer_set(&key_timer , 1);  //����10ms��ʱ��ⰴ��
                key_trigger_flag = 2;
            }
            else if(KEY_READ() && (1 == key_val))
            {
                bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
                timer_set(&key_timer , 1);  //����10ms��ʱ��ⰴ��
                key_trigger_flag = 3;
                
            }
            else
            {
                key_trigger_flag = 0;
            }
            break;

        case 2:
            if(timer_expired(&key_timer))
            {
                if(!KEY_READ())
                {
                    //�������
                    sim_printf_string("key_down...\r\n");
                    sim_printf_hex(sys_sta);
                    key_val = 1;
                }

                key_trigger_flag = 0;
            }
            break;

        //�ͷ�ʱ����
        case 3:
            if(timer_expired(&key_timer))
            {
                if(KEY_READ())
                {
                    //ȷ���ͷ�
                    sim_printf_string("key_up...\r\n");

                    key_val = 0;
                }
                key_trigger_flag = 0;
            }
            break;

    default:
           key_trigger_flag = 0;
       break;
    }
}

/******************************************************************************
* Name     :bsp_key_it 
*
* Desc     :�����ж��е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
void bsp_key_it(void)
{
    if(0 == key_trigger_flag)
        key_trigger_flag = 1;
}

void bsp_beep_play_it(void)
{
    uint8_t temp;
    
    if(beep_play_time > 0)
    {    
        BUZZER_TOGGLE();  //�Ȼ���
        
    }
    else
    {
        BUZZER_CLOSE();
		
    }

    switch(beep_play_type)
    {
        case E_BEEP_PLAY_PU:
        case E_BEEP_PLAY_DEF:
        case E_BEEP_PLAY_DEF_OK:
            if(0 == beep_play_time)
            {
				
				beep_play_sta = E_BEEP_PLAY_STA_NONE;
                beep_play_type = E_BEEP_PLAY_NONE;

                TIM4_DeInit();  //ֹͣ��ʱ��
                CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
				
				
                goto EXIT;
            }
            
            break;
        case E_BEEP_PLAY_TIPS:
        case E_BEEP_PLAY_LOW_PWR:
            if(0 == beep_play_time)
            {
                TIM4_DeInit();  //ֹͣ��ʱ��
                CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
                
                if(E_BEEP_PLAY_STA_FIRST == beep_play_sta)
                {
                    beep_play_sta = E_BEEP_PLAY_STA_REPEAT;
					
                    if(E_BEEP_PLAY_TIPS == beep_play_type)
                    {
                        beep_play_cnt = 1;
                        beep_play_pause_time = 30;  //300ms
                    }
                    else
                    {
                        beep_play_cnt = 2;
                        beep_play_pause_time = 5;  //50ms
                    }
                }
                else if(E_BEEP_PLAY_STA_REPLAY == beep_play_sta)
                {
                    beep_play_sta = E_BEEP_PLAY_STA_REPEAT;
                }
                else if(E_BEEP_PLAY_STA_LAST == beep_play_sta)
                {
                    beep_play_sta = E_BEEP_PLAY_STA_NONE;
                    beep_play_type = E_BEEP_PLAY_NONE;
					
                }

                goto EXIT;
            }
            break;
		
        case E_BEEP_PLAY_ALARM:
            if(0 == beep_play_time)
            {
                if(E_BEEP_PLAY_STA_FIRST == beep_play_sta)
                {
                    beep_play_section_time = 250;
                    beep_play_cnt = 1500;
                    beep_alarm_time_dir = 1;
                    beep_play_sta = E_BEEP_PLAY_STA_CONTINUE;
                }

                if(0 == beep_play_cnt)
                {
                    //���Ž���
                    TIM4_DeInit();  //ֹͣ��ʱ��
                    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
                    
                    beep_play_sta = E_BEEP_PLAY_STA_NONE;
                    beep_play_type = E_BEEP_PLAY_NONE;
                    beep_play_section_time = 250;
                }
				else if(beep_play_cnt >= 700)
                {
                    beep_play_cnt--;
					
                    if(0 == beep_alarm_time_dir)
                    {
                        if(beep_play_section_time < 185)
                        {
                            beep_play_section_time = 185;
                            beep_alarm_time_dir = 1;
                        }
                        else
                        {
                            beep_play_section_time -= 6;
                        }
                    }
                    else
                    {
                        //����
                        if(beep_play_section_time > 220)
                        {
                            beep_play_section_time = 220;
                            beep_alarm_time_dir = 0;
                        }
                        else
                        {
                            beep_play_section_time += 2;
                        }
                    }

                    bsp_timer4_reload(beep_play_section_time);
                    beep_play_time = 10000;  //100ms
                }
				else if(beep_play_cnt < 700)
                {
                    beep_play_cnt--;
					
                    if(0 == beep_alarm_time_dir)
                    {
                        if(beep_play_section_time < 190)
                        {
                            beep_play_section_time = 190;
                            beep_alarm_time_dir = 1;
                        }
                        else
                        {
                            beep_play_section_time -= 8;
                        }
                    }
                    else
                    {
                        //����
                        if(beep_play_section_time > 220)
                        {
                            beep_play_section_time = 220;
                            beep_alarm_time_dir = 0;
                        }
                        else
                        {
                            beep_play_section_time += 7;
                        }
                    }

                    bsp_timer4_reload(beep_play_section_time);
                    beep_play_time = 10000;  //100ms
                }
                
            }
            else if(beep_play_time < beep_play_section_time)
                beep_play_time = 0;
            else
            {
                beep_play_time -= beep_play_section_time;
            }
            goto EXIT;
            break;
    }

   /* switch(beep_play_type)
    {
        case E_BEEP_PLAY_PU:
        case E_BEEP_PLAY_DEF:
        case E_BEEP_PLAY_DEF_OK:     
        case E_BEEP_PLAY_TIPS:
        case E_BEEP_PLAY_LOW_PWR:
		case E_BEEP_PLAY_ALARM:
            if(0 == beep_play_time)
            {
				
				beep_play_sta = E_BEEP_PLAY_STA_NONE;
                beep_play_type = E_BEEP_PLAY_NONE;

                TIM4_DeInit();  //ֹͣ��ʱ��
                CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
				
				DC_DISABLE();
                goto EXIT;
            }
            
            break;

        
            
    }*/
	//if(beep_play_type == E_BEEP_PLAY_PU){
    	temp = beep_play_period[beep_play_type];  //ʹ����ʱ����תһ�£���ֹ2��volatile�ı�����ͬһ�����ʱ���뾯��
    	if(beep_play_time < temp)
                	beep_play_time = 0;
    	else
    	{
        	beep_play_time -= temp;
    	}
	//}

EXIT:
	
	return;
}

void bsp_pwr_manager(void)
{
    if((E_BEEP_PLAY_STA_FIRST == beep_play_sta) || (E_BEEP_PLAY_STA_REPLAY == beep_play_sta) || (E_BEEP_PLAY_STA_LAST == beep_play_sta)
        || (E_BEEP_PLAY_STA_CONTINUE == beep_play_sta) || (E_BEEP_PLAY_STA_REPEAT == beep_play_sta))
    {
//        sim_printf_string("beep playing,enter wait...\r\n");
        wfi();
//        sim_printf_string("exit wait from wait...\r\n");
    }
    else
    {
        if((0 == vib_flag) && (0 == vib_sta) 
            && (E_BEEP_PLAY_STA_NONE == beep_play_sta) && (E_SYS_STA_WAIT_DEF != sys_sta)
            && (E_SYS_STA_PU != sys_sta)
            && (0 == key_trigger_flag))
        {
            //bsp_rtc_deinit();  //�ر�RTC���Ա�֤��������halt
            bsp_rtc_halt();  //��ֹ����haltǰ���������жϵ��¿��ܻ��ⲻ�����������
            sim_printf_string("system free,close RTC,");
            //sim_printf_hex(rtc_mode);
        }
        
//        sim_printf_string("enter halt...\r\n");
        halt();
        
//        wfi();
//        sim_printf_string("exit halt...\r\n");
    }
}

#if (SIM_UART_PRINTF_EN)
/******************************************************************************
* Name      :sim_uart_printf 
*
* Desc     :ʹ��PC5ģ�⴮�ڣ�������115200
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :��16M��Ƶʱ����ʱ8.68us��Ҫ��ʱԼ139T
            ��4M��Ƶʱ����ʱ8.68us��Ҫ��ʱԼ34T

* Author   :bobde163
* -------------------------------------
* Log     :2020/06/21 17:43:35, Create this function by bobde163
 ******************************************************************************/
void sim_uart_printf(uint8_t data)
{
    //PB_ODR |= bit0;
    //PB_DDR |= bit0;
    //PB_CR1 |= bit0;

    asm("SIM");  //�����ж�

    //������ʼλ
    GPIOB->ODR &= ~GPIO_Pin_0;

#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
asm
(
    //���汾����ʹ�õ��ļĴ���
    "PUSHW Y\n"  //2T
    "PUSHW X\n"  //2T
    
    "LDW Y,#9\n"  //2T��������ѭ��
    "LDW X,#41\n" //2T����ֵ��ʵ��õ�
"Delay0: DECW X\n" //��1��1��ָ������
    "JRNE Delay0\n" //�ж���ת����תҪ2T������Ҫ1T
    "NOP\n"

    //�������µĵ�ƽ����Ҫ6T
"SEND_DATA:DECW Y\n"  //1T,�ж�ѭ���Ƿ����
    "JREQ STOP\n"//��������ת��2T������ת��1T
    "NOP\n"//1T������δ��ת��1T
    
    "SRL A\n" //1T,�߼����ƣ�ͬʱbit0��C��־λ��
    "JRC SET1\n"  //����1����ת����ת2T������ת1T
    "NOP\n"//1T������δ��ת��1T
    "BRES 0x5005,#0\n" //1T�������
    "JP Delay1\n"  //1T����������ת
"SET1:BSET 0x5005,#0\n" //1T�������
    "NOP\n"

"Delay1:NOP\n"
    "LDW X,#41\n" //2T��ʵ��õ�
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//��ת��2T������ת��1T
    
    "JP SEND_DATA\n"  //1T����������ת


    //����2λʱ����ֹͣλ
"STOP:TNZW X\n" //2T,ֻ��Ϊ����ʱ2T
    "TNZW X\n" //2T,ֻ��Ϊ����ʱ2T
    "BSET 0x5005,#0\n" //1T�������
    "LDW X,#92\n" //2T����ʱ139x2-2=276T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//��ת��2T������ת��1T
    "NOP\n"

    //�ָ�������ʹ�õ��ļĴ���
    "POPW X\n"
    "POPW Y"
)       ;
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
    //ÿ��bit��ʱ34T
asm
(
    //���汾����ʹ�õ��ļĴ���
    "PUSHW Y\n"
    "PUSHW X\n"
    
    "LDW Y,#9\n"  //2T��������ѭ��
    "LDW X,#6\n" //2T����ֵ
    //������ʱ19T
"Delay0: DECW X\n" //��1��1��ָ������
    "JRNE Delay0\n" //�ж���ת����תҪ2T������Ҫ1T
    "NOP\n"
    "NOP\n"

    //�������µĵ�ƽ����Ҫ6T
"SEND_DATA:DECW Y\n"  //1T,�ж�ѭ���Ƿ����
    "JREQ STOP\n"//��������ת��2T������ת��1T
    "NOP\n"//1T������δ��ת��1T

    "SRL A\n" //1T,�߼����ƣ�ͬʱbit0��C��־λ��
    "JRC SET1\n"  //����1����ת����ת2T��ȷ�ϲ���ת1T
    //"NOP\n"//1T������δ��ת��1T��ʵ�����
    "BRES 0x5005,#0\n" //1T�������
    "JP Delay1\n"  //1T����������ת
"SET1:BSET 0x5005,#0\n" //1T�������
    "NOP\n"
    "NOP\n" //ʵ�����
    "NOP\n"

    //���¸�bit�仯ǰ����Ҫ����ʱ26T
"Delay1:LDW X,#7\n" //2T��
    //������ʱ21T
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//��ת��2T������ת��1T
    "JP SEND_DATA\n"  //1T����������ת


    //����2λʱ����ֹͣλ
"STOP:TNZW X\n" //2T,ֻ��Ϊ����ʱ2T
    "NOP\n" //1T,ֻ��Ϊ����ʱ1T
    "BSET 0x5005,#0\n" //1T�������

    //��Ҫ��ʱ68T
    "LDW X,#22\n" //2T
    //��������ʱ66T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//��ת��2T������ת��1T
    "NOP\n"

    //�ָ�������ʹ�õ��ļĴ���
    "POPW X\n"
    "POPW Y"    
);
#endif
    asm("RIM");	//ʹ��ȫ���ж�
}

void sim_uart_printf_it(uint8_t data)
{
    //PB_ODR |= bit0;
    //PB_DDR |= bit0;
    //PB_CR1 |= bit0;

    //asm("SIM");  //�����ж�

    //������ʼλ
    GPIOB->ODR &= ~GPIO_Pin_0;

#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
asm
(
    //���汾����ʹ�õ��ļĴ���
    "PUSHW Y\n"  //2T
    "PUSHW X\n"  //2T
    
    "LDW Y,#9\n"  //2T��������ѭ��
    "LDW X,#41\n" //2T����ֵ��ʵ��õ�
"Delay0: DECW X\n" //��1��1��ָ������
    "JRNE Delay0\n" //�ж���ת����תҪ2T������Ҫ1T
    "NOP\n"

    //�������µĵ�ƽ����Ҫ6T
"SEND_DATA:DECW Y\n"  //1T,�ж�ѭ���Ƿ����
    "JREQ STOP\n"//��������ת��2T������ת��1T
    "NOP\n"//1T������δ��ת��1T
    
    "SRL A\n" //1T,�߼����ƣ�ͬʱbit0��C��־λ��
    "JRC SET1\n"  //����1����ת����ת2T������ת1T
    "NOP\n"//1T������δ��ת��1T
    "BRES 0x5005,#0\n" //1T�������
    "JP Delay1\n"  //1T����������ת
"SET1:BSET 0x5005,#0\n" //1T�������
    "NOP\n"

"Delay1:NOP\n"
    "LDW X,#41\n" //2T��ʵ��õ�
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//��ת��2T������ת��1T
    
    "JP SEND_DATA\n"  //1T����������ת


    //����2λʱ����ֹͣλ
"STOP:TNZW X\n" //2T,ֻ��Ϊ����ʱ2T
    "TNZW X\n" //2T,ֻ��Ϊ����ʱ2T
    "BSET 0x5005,#0\n" //1T�������
    "LDW X,#92\n" //2T����ʱ139x2-2=276T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//��ת��2T������ת��1T
    "NOP\n"

    //�ָ�������ʹ�õ��ļĴ���
    "POPW X\n"
    "POPW Y"
)       ;
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
    //ÿ��bit��ʱ34T
asm
(
    //���汾����ʹ�õ��ļĴ���
    "PUSHW Y\n"
    "PUSHW X\n"
    
    "LDW Y,#9\n"  //2T��������ѭ��
    "LDW X,#6\n" //2T����ֵ
    //������ʱ19T
"Delay0: DECW X\n" //��1��1��ָ������
    "JRNE Delay0\n" //�ж���ת����תҪ2T������Ҫ1T
    "NOP\n"
    "NOP\n"

    //�������µĵ�ƽ����Ҫ6T
"SEND_DATA:DECW Y\n"  //1T,�ж�ѭ���Ƿ����
    "JREQ STOP\n"//��������ת��2T������ת��1T
    "NOP\n"//1T������δ��ת��1T

    "SRL A\n" //1T,�߼����ƣ�ͬʱbit0��C��־λ��
    "JRC SET1\n"  //����1����ת����ת2T��ȷ�ϲ���ת1T
    //"NOP\n"//1T������δ��ת��1T��ʵ�����
    "BRES 0x5005,#0\n" //1T�������
    "JP Delay1\n"  //1T����������ת
"SET1:BSET 0x5005,#0\n" //1T�������
    "NOP\n"
    "NOP\n" //ʵ�����
    "NOP\n"

    //���¸�bit�仯ǰ����Ҫ����ʱ26T
"Delay1:LDW X,#7\n" //2T��
    //������ʱ21T
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//��ת��2T������ת��1T
    "JP SEND_DATA\n"  //1T����������ת


    //����2λʱ����ֹͣλ
"STOP:TNZW X\n" //2T,ֻ��Ϊ����ʱ2T
    "NOP\n" //1T,ֻ��Ϊ����ʱ1T
    "BSET 0x5005,#0\n" //1T�������

    //��Ҫ��ʱ68T
    "LDW X,#22\n" //2T
    //��������ʱ66T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//��ת��2T������ת��1T
    "NOP\n"

    //�ָ�������ʹ�õ��ļĴ���
    "POPW X\n"
    "POPW Y"    
);
#endif
//    asm("RIM");	//ʹ��ȫ���ж�
}


void sim_printf_string(uint8_t *str)
{
    if(0 == str)
        return;
    
    while(*str)
    {
        sim_uart_printf(*str++);
    }
}

const uint8_t hex_tab[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void sim_printf_hex(uint8_t data)
{
    sim_uart_printf('0');
    sim_uart_printf('x');
    sim_uart_printf(hex_tab[data >> 4]);
    sim_uart_printf(hex_tab[data & 0x0F]);
    sim_uart_printf(' ');
}

#endif

/*---------------------------------------------------------------------------*/


