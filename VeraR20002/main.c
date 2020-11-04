/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :main.c 
* Desc     :适配碟刹锁项目
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
    E_RTC_MODE_HALT = 0x04,  //停机模式
};


#define BEEP_PLAY_PU_TIME   500000
#define BEEP_PLAY_PU_TIPS   500000

#define BEEP_PLAY_INTERVAL  10   //连续响时，间隔时长，单位为10ms

/*----------------------- Variable Declarations -----------------------------*/
//系统时钟值变量，上电默认值为2M，如果有重新配置时钟，则此值会同步更新
uint32_t sys_clk_freq_val = SYS_CLK_FREQ_2M;

uint32_t sys_base_clock_cnt = 0;

uint8_t rtc_mode = E_RTC_MODE_STOP;
volatile uint16_t rtc_delay_cnt = 0;  //用于使用RTC延时时的计数值，计数到0则表示计时到


soft_timet_t timer_test;

uint8_t sys_sta = E_SYS_STA_NONE;  //用于记录当前的布撤防状态


uint8_t key_val = 0;  //用于记录按键值，0：未释放，1：按下
volatile static uint8_t key_trigger_flag = 0;  //用于标记按键中断


volatile uint8_t global_interrupt_flag = 0;  //用于记录当前的总中断标志

volatile uint8_t beep_play_sta = E_BEEP_PLAY_STA_NONE;  //记录当前的播放状态
volatile uint8_t beep_play_type = E_BEEP_PLAY_NONE;
volatile uint32_t beep_play_time = 0;  //记录要响的时长
volatile uint16_t beep_play_cnt = 0;  //用于记录要重复播放的次数
//volatile uint8_t beep_pause_flag = 0;  //用于记录低电量提示的变奏状态： 0->200ms, 1->20ms
volatile uint8_t beep_play_section_cnt = 0; //用于在播放报警音时记录当前段数
uint8_t beep_play_section_time = 250;
uint8_t beep_alarm_time_dir = 0;  //用于记录间隔时长是增加还是递减，0，增加，1，递减

static soft_timet_t beep_timer = {0 , 0};
static uint8_t beep_play_pause_time = 0;  //用于在重复播放时，记录暂停时长，单位为10ms

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
* Desc     :对本系统要使用的GPIO口进行相应的初始化
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
    //系统时钟初始化
    bsp_system_clock_init(SYS_CLK_FREQ);

    //FWU和ULP位必须在等待内部参考电压稳定(VREFINTF位为1时)才能修改，
    //此处应该先判断内部参考电压稳定后再修改FWU和ULP位
    //这3位都在PWR_CSR2寄存器中
    while(RESET == PWR_GetFlagStatus(PWR_FLAG_VREFINTF));  //bob 添加，等待内部参考电压稳定
    PWR_FastWakeUpCmd(DISABLE);
    PWR_UltraLowPowerCmd(ENABLE);

    bsp_io_init();

    sim_printf_string("pritnf is ok...\r\n");

    //bsp_timer4_init();

    //halt();

    BSP_ENABLE_INTERRUPT();  //使能总中断

    

    bsp_beep_play(E_BEEP_PLAY_PU);

    key_trigger_flag = 1;  //手动置1，保证上电后能执行一次按键值读取
    
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
* Desc     :根据设置的系统时钟宏定义，初始化CPU时钟配置，除与BOOT_ROM相关模块的时钟启用外，
            其他外设模块时钟配置为关闭
* Param_in :sys_clk:需要使用的系统时钟，参见枚举类型定义
* Param_out:
* Return   :
* Global   :
* Note     :1、本项目固定使用内置HSI 16MHz为时钟源，而系统复位后默认即使用此时钟源，因此不
            再单独选择时钟源
            2、在需要修改系统时钟时，最好是通过调用此函数进行修改，以保证系统时钟变量值同步更新
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/03/28, Create this function by CenLinbo
 ******************************************************************************/
void bsp_system_clock_init(uint32_t sys_clk)
{
    CLK_HSICmd(ENABLE);//开始内部高频RC 16M
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);//HSI为系统时钟

    //根据
    if(SYS_CLK_FREQ_16M == sys_clk)
    {
        CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);//不分频，即1分频
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
        //如果传递进来的参数无效，则不作任何修改
    }
}

/******************************************************************************
* Name     :bsp_get_system_clock_val 
*
* Desc     :获取系统时钟频率值
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
* Desc     :配置TIM4为1ms中断一次，作为系统时基
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
    TIM4_DeInit();  //先恢复成默认值

    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , ENABLE);

    //配置为1m溢出一次
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

    TIM4_ARRPreloadConfig(ENABLE);  //启用自动装载功能

    TIM4_ClearFlag(TIM4_FLAG_Update);

    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);

    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);
}

void bsp_timer4_init_2(uint8_t period)
{
    TIM4_DeInit();  //先恢复成默认值

    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , ENABLE);

    //配置为1us溢出一次
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

    TIM4_ARRPreloadConfig(ENABLE);  //启用自动装载功能

    TIM4_ClearFlag(TIM4_FLAG_Update);

    /* Enable update interrupt */
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);

    /* Enable TIM4 */
    TIM4_Cmd(ENABLE);
}

/******************************************************************************
* Name     :bsp_timer4_reload 
*
* Desc     :在不需要停止定时器直接更新中间间隔时调用
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
* Desc     :获取系统滴答时间值
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
* Desc     :系统滴答时间值加1，在定时器中调用
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
* Desc     :对本系统要使用的GPIO口进行相应的初始化
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
    EXTI_SetPinSensitivity(VIB_SENSOR_EXIT_Pin , EXTI_Trigger_Falling);  //设置为上升下降沿都触发
    ITC_SetSoftwarePriority(EXTI3_IRQn , ITC_PriorityLevel_2);  //设置为次优先级，防止干扰蜂鸣器中断

    EXTI_SetPinSensitivity(KEY_EXIT_Pin , EXTI_Trigger_Rising_Falling);//禁用总中断时才能修改
    
    GPIO_Init(KEY_GPIO_Port , KEY_Pin , GPIO_Mode_In_PU_IT);
    
    //V1.0
    GPIO_Init(VIB_SENSOR_Port , VIB_SENSOR_Pin , GPIO_Mode_In_FL_No_IT);  //浮空，初始状态不开中断
	
	GPIO_Init(VIB_EN_Port , VIB_EN_Pin , GPIO_Mode_Out_PP_Low_Slow);  //VIB初始使能关
	/********电路变动 修改**********/
	/*
    GPIO_Init(LOWV_EN_Port , LOWV_EN_Pin , GPIO_Mode_Out_PP_High_Slow);  
    GPIO_Init(LOWV_ADC_Port , LOWV_ADC_Pin , GPIO_Mode_In_FL_No_IT);
	*/
	/*******************************/
	GPIO_Init(LOWV_EN_Port , LOWV_EN_Pin , GPIO_Mode_Out_PP_High_Slow);  
    GPIO_Init(LOWV_ADC_Port , LOWV_ADC_Pin , GPIO_Mode_Out_PP_High_Slow);
	
    GPIO_Init(DC_EN_Port , DC_EN_Pin , GPIO_Mode_Out_PP_Low_Slow);

    GPIO_Init(BUZZER_Port , BUZZER_Pin , GPIO_Mode_Out_PP_Low_Slow);

    //模拟串口IO口初始化
    GPIO_Init(SIM_DEBUG_Port , SIM_DEBUG_Pin , GPIO_Mode_Out_PP_High_Slow);

    //未使用的引脚初始化为内部上拉输入
    GPIO_Init(GPIOB , PB_UNUSED_Pin , GPIO_Mode_In_PU_No_IT);
    GPIO_Init(GPIOC , PC_UNUSED_Pin , GPIO_Mode_In_PU_No_IT);
}

/******************************************************************************
* Name     :bsp_rtc_init 
*
* Desc     :配置RTC为自动唤醒
* Param_in :支持1ms、10ms、1000ms
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
    //如果定时器已经打开了，则直接退出 
    if(E_RTC_MODE_ONLY_COUNT & rtc_mode)
        return;
    
    /* Enable RTC clock */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
    /* Wait for LSE clock to be ready */
    while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
  
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC , ENABLE);  //打开外设时钟
    RTC_WakeUpCmd(DISABLE);
    
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

    if(E_RTC_WAKEUP_PERIOD_10MS == wakeup_period)
        RTC_SetWakeUpCounter(190);  //10ms中断
    else if(E_RTC_WAKEUP_PERIOD_1000MS == wakeup_period)
        RTC_SetWakeUpCounter(19000);  //1000ms中断
    else if(E_RTC_WAKEUP_PERIOD_1MS == wakeup_period)
        RTC_SetWakeUpCounter(19);  //1ms中断

    RTC_ClearITPendingBit(RTC_IT_WUT);
    ITC_SetSoftwarePriority(RTC_CSSLSE_IRQn , ITC_PriorityLevel_2);  //设置为次优先级
    RTC_ITConfig(RTC_IT_WUT , ENABLE);

    RTC_WakeUpCmd(ENABLE);

    rtc_mode = E_RTC_MODE_ONLY_COUNT;
}

/******************************************************************************
* Name     :bsp_rtc_halt 
*
* Desc     :配置RTC为1000ms自动唤醒一次，在系统需要关闭RTC进入停机模式前调用
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
    //避免反复的设置
    if(E_RTC_MODE_HALT == rtc_mode)
        return;
    
    /* Enable RTC clock */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
    /* Wait for LSE clock to be ready */
    while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
  
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC , ENABLE);  //打开外设时钟
    RTC_WakeUpCmd(DISABLE);
    
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);

    RTC_SetWakeUpCounter(19000);  //1000ms中断

    RTC_ClearITPendingBit(RTC_IT_WUT);
    ITC_SetSoftwarePriority(RTC_CSSLSE_IRQn , ITC_PriorityLevel_2);  //设置为次优先级
    RTC_ITConfig(RTC_IT_WUT , ENABLE);

    RTC_WakeUpCmd(ENABLE);

    rtc_mode = E_RTC_MODE_HALT;
    rtc_delay_cnt = 0;
}


/******************************************************************************
* Name     :bsp_rtc_deinit 
*
* Desc     :禁用RTC
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
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC , DISABLE);  //关闭外设时钟
    CLK_RTCClockConfig(CLK_RTCCLKSource_Off, CLK_RTCCLKDiv_1);

    rtc_mode = E_RTC_MODE_STOP;
    rtc_delay_cnt = 0;
}

/******************************************************************************
* Name     :bsp_delay_ms_by_rtc 
*
* Desc     :使用RTC来做ms级延时，可根据当前RTC的模式选择对应的方式，以节省功耗
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
        rtc_delay_cnt = n + 1;  //不补偿1的话，延时时长会比实际延时少0~10ms，补偿后会比实际长0~10ms

        while(rtc_delay_cnt)
        {
            halt();
        }
    }
    else
    {
        //RTC未使用时，直接调用单次延时
        delay_ms_2(n);
    }

    rtc_mode &= ~E_RTC_MODE_ONLY_DELAY;
}


/******************************************************************************
* Name     :bsp_rtc_it_handler 
*
* Desc     :RTC自动唤醒中断中调用
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
    //同类型声音在播放时不允许中断
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
* Desc     :重新加载播放参数，需要重复播放时才调用，否则其他情况下不允许调用
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
    //处理需要重复播放的声音
    switch(beep_play_type)
    {
        case E_BEEP_PLAY_NONE:
            break;
            
        //需要连续播放的提示音和低电量报警音
        case E_BEEP_PLAY_TIPS:
        case E_BEEP_PLAY_LOW_PWR:
            if(E_BEEP_PLAY_STA_REPEAT == beep_play_sta)
            {
                //首次播放，需要启动延时，再响一次
                if(0 == beep_play_cnt)
                {
                    //次数错误，理论上不应该执行到此处
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
                    //先变更状态
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
* Desc     :进入待机模式时，关闭蜂鸣器相关的外设和变量
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
* Desc     :按键检测函数，在大循环中调用
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
        return;//是无按键中断，不处理

    switch(key_trigger_flag)
    {
        case 1:
            if((!KEY_READ()) && (0 == key_val))
            {
                sim_printf_string("key_detect...\r\n");

                bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
                timer_set(&key_timer , 1);  //启动10ms定时检测按键
                key_trigger_flag = 2;
            }
            else if(KEY_READ() && (1 == key_val))
            {
                bsp_rtc_init(E_RTC_WAKEUP_PERIOD_10MS);
                timer_set(&key_timer , 1);  //启动10ms定时检测按键
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
                    //消抖完成
                    sim_printf_string("key_down...\r\n");
                    sim_printf_hex(sys_sta);
                    key_val = 1;
                }

                key_trigger_flag = 0;
            }
            break;

        //释放时消抖
        case 3:
            if(timer_expired(&key_timer))
            {
                if(KEY_READ())
                {
                    //确认释放
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
* Desc     :按键中断中调用
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
        BUZZER_TOGGLE();  //先换向
        
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

                TIM4_DeInit();  //停止定时器
                CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
				
				
                goto EXIT;
            }
            
            break;
        case E_BEEP_PLAY_TIPS:
        case E_BEEP_PLAY_LOW_PWR:
            if(0 == beep_play_time)
            {
                TIM4_DeInit();  //停止定时器
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
                    //播放结束
                    TIM4_DeInit();  //停止定时器
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
                        //递增
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
                        //递增
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

                TIM4_DeInit();  //停止定时器
                CLK_PeripheralClockConfig(CLK_Peripheral_TIM4 , DISABLE);
				
				DC_DISABLE();
                goto EXIT;
            }
            
            break;

        
            
    }*/
	//if(beep_play_type == E_BEEP_PLAY_PU){
    	temp = beep_play_period[beep_play_type];  //使用临时变量转一下，防止2个volatile的变量在同一条语句时编译警告
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
            //bsp_rtc_deinit();  //关闭RTC，以保证进入完整halt
            bsp_rtc_halt();  //防止进入halt前发生按键中断导致可能会检测不到按键的情况
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
* Desc     :使用PC5模拟串口，波特率115200
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :在16M主频时，延时8.68us需要延时约139T
            在4M主频时，延时8.68us需要延时约34T

* Author   :bobde163
* -------------------------------------
* Log     :2020/06/21 17:43:35, Create this function by bobde163
 ******************************************************************************/
void sim_uart_printf(uint8_t data)
{
    //PB_ODR |= bit0;
    //PB_DDR |= bit0;
    //PB_CR1 |= bit0;

    asm("SIM");  //关总中断

    //发送起始位
    GPIOB->ODR &= ~GPIO_Pin_0;

#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
asm
(
    //保存本程序使用到的寄存器
    "PUSHW Y\n"  //2T
    "PUSHW X\n"  //2T
    
    "LDW Y,#9\n"  //2T用来控制循环
    "LDW X,#41\n" //2T，赋值，实测得到
"Delay0: DECW X\n" //减1，1个指令周期
    "JRNE Delay0\n" //判断跳转，跳转要2T，不跳要1T
    "NOP\n"

    //到设置新的电平，需要6T
"SEND_DATA:DECW Y\n"  //1T,判断循环是否结束
    "JREQ STOP\n"//结束则跳转，2T，不跳转则1T
    "NOP\n"//1T，补偿未跳转的1T
    
    "SRL A\n" //1T,逻辑右移，同时bit0在C标志位中
    "JRC SET1\n"  //等于1则跳转，跳转2T，不跳转1T
    "NOP\n"//1T，补偿未跳转的1T
    "BRES 0x5005,#0\n" //1T，输出低
    "JP Delay1\n"  //1T，无条件跳转
"SET1:BSET 0x5005,#0\n" //1T，输出高
    "NOP\n"

"Delay1:NOP\n"
    "LDW X,#41\n" //2T，实测得到
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//跳转则2T，不跳转则1T
    
    "JP SEND_DATA\n"  //1T，无条件跳转


    //发送2位时长的停止位
"STOP:TNZW X\n" //2T,只是为了延时2T
    "TNZW X\n" //2T,只是为了延时2T
    "BSET 0x5005,#0\n" //1T，输出高
    "LDW X,#92\n" //2T，延时139x2-2=276T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//跳转则2T，不跳转则1T
    "NOP\n"

    //恢复本程序使用到的寄存器
    "POPW X\n"
    "POPW Y"
)       ;
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
    //每个bit延时34T
asm
(
    //保存本程序使用到的寄存器
    "PUSHW Y\n"
    "PUSHW X\n"
    
    "LDW Y,#9\n"  //2T用来控制循环
    "LDW X,#6\n" //2T，赋值
    //本段延时19T
"Delay0: DECW X\n" //减1，1个指令周期
    "JRNE Delay0\n" //判断跳转，跳转要2T，不跳要1T
    "NOP\n"
    "NOP\n"

    //到设置新的电平，需要6T
"SEND_DATA:DECW Y\n"  //1T,判断循环是否结束
    "JREQ STOP\n"//结束则跳转，2T，不跳转则1T
    "NOP\n"//1T，补偿未跳转的1T

    "SRL A\n" //1T,逻辑右移，同时bit0在C标志位中
    "JRC SET1\n"  //等于1则跳转，跳转2T，确认不跳转1T
    //"NOP\n"//1T，补偿未跳转的1T，实测调整
    "BRES 0x5005,#0\n" //1T，输出低
    "JP Delay1\n"  //1T，无条件跳转
"SET1:BSET 0x5005,#0\n" //1T，输出高
    "NOP\n"
    "NOP\n" //实测调整
    "NOP\n"

    //到下个bit变化前，需要再延时26T
"Delay1:LDW X,#7\n" //2T，
    //本段延时21T
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//跳转则2T，不跳转则1T
    "JP SEND_DATA\n"  //1T，无条件跳转


    //发送2位时长的停止位
"STOP:TNZW X\n" //2T,只是为了延时2T
    "NOP\n" //1T,只是为了延时1T
    "BSET 0x5005,#0\n" //1T，输出高

    //需要延时68T
    "LDW X,#22\n" //2T
    //本段总延时66T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//跳转则2T，不跳转则1T
    "NOP\n"

    //恢复本程序使用到的寄存器
    "POPW X\n"
    "POPW Y"    
);
#endif
    asm("RIM");	//使能全局中断
}

void sim_uart_printf_it(uint8_t data)
{
    //PB_ODR |= bit0;
    //PB_DDR |= bit0;
    //PB_CR1 |= bit0;

    //asm("SIM");  //关总中断

    //发送起始位
    GPIOB->ODR &= ~GPIO_Pin_0;

#if (SYS_CLK_FREQ == SYS_CLK_FREQ_16M)
asm
(
    //保存本程序使用到的寄存器
    "PUSHW Y\n"  //2T
    "PUSHW X\n"  //2T
    
    "LDW Y,#9\n"  //2T用来控制循环
    "LDW X,#41\n" //2T，赋值，实测得到
"Delay0: DECW X\n" //减1，1个指令周期
    "JRNE Delay0\n" //判断跳转，跳转要2T，不跳要1T
    "NOP\n"

    //到设置新的电平，需要6T
"SEND_DATA:DECW Y\n"  //1T,判断循环是否结束
    "JREQ STOP\n"//结束则跳转，2T，不跳转则1T
    "NOP\n"//1T，补偿未跳转的1T
    
    "SRL A\n" //1T,逻辑右移，同时bit0在C标志位中
    "JRC SET1\n"  //等于1则跳转，跳转2T，不跳转1T
    "NOP\n"//1T，补偿未跳转的1T
    "BRES 0x5005,#0\n" //1T，输出低
    "JP Delay1\n"  //1T，无条件跳转
"SET1:BSET 0x5005,#0\n" //1T，输出高
    "NOP\n"

"Delay1:NOP\n"
    "LDW X,#41\n" //2T，实测得到
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//跳转则2T，不跳转则1T
    
    "JP SEND_DATA\n"  //1T，无条件跳转


    //发送2位时长的停止位
"STOP:TNZW X\n" //2T,只是为了延时2T
    "TNZW X\n" //2T,只是为了延时2T
    "BSET 0x5005,#0\n" //1T，输出高
    "LDW X,#92\n" //2T，延时139x2-2=276T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//跳转则2T，不跳转则1T
    "NOP\n"

    //恢复本程序使用到的寄存器
    "POPW X\n"
    "POPW Y"
)       ;
#elif (SYS_CLK_FREQ == SYS_CLK_FREQ_4M)
    //每个bit延时34T
asm
(
    //保存本程序使用到的寄存器
    "PUSHW Y\n"
    "PUSHW X\n"
    
    "LDW Y,#9\n"  //2T用来控制循环
    "LDW X,#6\n" //2T，赋值
    //本段延时19T
"Delay0: DECW X\n" //减1，1个指令周期
    "JRNE Delay0\n" //判断跳转，跳转要2T，不跳要1T
    "NOP\n"
    "NOP\n"

    //到设置新的电平，需要6T
"SEND_DATA:DECW Y\n"  //1T,判断循环是否结束
    "JREQ STOP\n"//结束则跳转，2T，不跳转则1T
    "NOP\n"//1T，补偿未跳转的1T

    "SRL A\n" //1T,逻辑右移，同时bit0在C标志位中
    "JRC SET1\n"  //等于1则跳转，跳转2T，确认不跳转1T
    //"NOP\n"//1T，补偿未跳转的1T，实测调整
    "BRES 0x5005,#0\n" //1T，输出低
    "JP Delay1\n"  //1T，无条件跳转
"SET1:BSET 0x5005,#0\n" //1T，输出高
    "NOP\n"
    "NOP\n" //实测调整
    "NOP\n"

    //到下个bit变化前，需要再延时26T
"Delay1:LDW X,#7\n" //2T，
    //本段延时21T
"LOOP1:DECW X\n"  //1T
    "JRNE LOOP1\n"//跳转则2T，不跳转则1T
    "JP SEND_DATA\n"  //1T，无条件跳转


    //发送2位时长的停止位
"STOP:TNZW X\n" //2T,只是为了延时2T
    "NOP\n" //1T,只是为了延时1T
    "BSET 0x5005,#0\n" //1T，输出高

    //需要延时68T
    "LDW X,#22\n" //2T
    //本段总延时66T
"LOOP2:DECW X\n"  //1T
    "JRNE LOOP2\n"//跳转则2T，不跳转则1T
    "NOP\n"

    //恢复本程序使用到的寄存器
    "POPW X\n"
    "POPW Y"    
);
#endif
//    asm("RIM");	//使能全局中断
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


