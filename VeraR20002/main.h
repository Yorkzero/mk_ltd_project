/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :main.h 
* Desc     :适配华为电子把手锁的项目
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
#ifndef _MAIN_H_     
#define _MAIN_H_    


/*------------------------------- Includes ----------------------------------*/
#include "stm8l15x.h"
#include <stddef.h>

/*----------------------------- Global Defines ------------------------------*/
//模拟串口打印开关
#define SIM_UART_PRINTF_EN     0

//系统时钟值宏定义
#define SYS_CLK_FREQ_16M       16000000
#define SYS_CLK_FREQ_8M        8000000
#define SYS_CLK_FREQ_4M        4000000
#define SYS_CLK_FREQ_2M        2000000
#define SYS_CLK_FREQ_1M        1000000
#define SYS_CLK_FREQ_500K      500000
#define SYS_CLK_FREQ_250K      250000
#define SYS_CLK_FREQ_125K      125000


//配置系统时钟，在初始化时钟时根据此定义初始化，注：本系统使用内部16MHz HSI时钟源
#define SYS_CLK_FREQ          SYS_CLK_FREQ_4M    //上电默认为2M，可根据情况修改

//无自锁开关
#define KEY_GPIO_Port         GPIOA
#define KEY_Pin               GPIO_Pin_2
#define KEY_EXIT_Pin          EXTI_Pin_2
#define KEY_READ()            (KEY_GPIO_Port->IDR & KEY_Pin)
#define KEY_ENABLE()		  GPIO_Init(KEY_GPIO_Port , KEY_Pin , GPIO_Mode_In_FL_IT)
#define KEY_DISABLE()		  GPIO_Init(KEY_GPIO_Port , KEY_Pin , GPIO_Mode_In_PU_IT)


//振动传感器
#define VIB_SENSOR_Port     GPIOA
#define VIB_SENSOR_Pin      GPIO_Pin_3
#define VIB_SENSOR_EXIT_Pin  EXTI_Pin_3

#define VIB_EN_Port     GPIOC
#define VIB_EN_Pin      GPIO_Pin_4


#define VIB_IT_ENABLE()        GPIO_Init(VIB_SENSOR_Port , VIB_SENSOR_Pin , GPIO_Mode_In_FL_IT)
#define VIB_IT_DISABLE()       GPIO_Init(VIB_SENSOR_Port , VIB_SENSOR_Pin , GPIO_Mode_In_FL_No_IT)

#define VIB_ENABLE()        (VIB_EN_Port -> ODR |= VIB_EN_Pin)
#define VIB_DISABLE()       (VIB_EN_Port -> ODR &= ~VIB_EN_Pin)

//电池电压检测
#define LOWV_ADC_Port        GPIOD
#define LOWV_ADC_Pin         GPIO_Pin_0
#define LOWV_EN_Port        GPIOB
//#define LOWV_EN_Pin         GPIO_Pin_5// v1.0
#define LOWV_EN_Pin         GPIO_Pin_0// v1.1
#define LOWV_ENABLE()       {LOWV_ADC_Port->DDR&= ~LOWV_ADC_Pin;LOWV_EN_Port->ODR &= ~LOWV_EN_Pin;}
#define LOWV_DISABLE()      {LOWV_EN_Port->ODR |= LOWV_EN_Pin;LOWV_ADC_Port->DDR|= LOWV_ADC_Pin;}

#define LOWV_ADC_CH         ADC_Channel_22
#define LOWV_ADC_CH_GROUP   ADC_Group_SlowChannels


//DCDC使能引脚
#define DC_EN_Port        GPIOB
#define DC_EN_Pin         GPIO_Pin_4
#define DC_ENABLE()        DC_EN_Port -> ODR |= DC_EN_Pin
#define DC_DISABLE()       DC_EN_Port -> ODR &= ~DC_EN_Pin


//蜂鸣器控制
#define BUZZER_Port          GPIOB
#define BUZZER_Pin           GPIO_Pin_6
#define BUZZER_TOGGLE()        BUZZER_Port->ODR ^= BUZZER_Pin
#define BUZZER_CLOSE()        BUZZER_Port->ODR &= ~BUZZER_Pin
#define BUZZER_OPEN()        BUZZER_Port->ODR |= BUZZER_Pin


//用于模拟串口打印的
#define SIM_DEBUG_Port       GPIOB
#define SIM_DEBUG_Pin        GPIO_Pin_5

//PX口未用引脚
#define PB_UNUSED_Pin     (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7)
//v1.0
//#define PC_UNUSED_Pin     (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6)
//v1.1
#define PC_UNUSED_Pin     (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6)


#define BSP_ENABLE_INTERRUPT()     {rim();global_interrupt_flag = 1;}
#define BSP_DISABLE_INTERRUPT()     {sim();global_interrupt_flag = 0;}
#define BSP_SET_INTERRUPT(flag)     {if(flag) rim();else sim();global_interrupt_flag = flag;}
#define BSP_GET_INTERRUPT()        global_interrupt_flag

enum
{
    E_SYS_STA_NONE = 0, //初始状态
    E_SYS_STA_PU,  //上电状态
    E_SYS_STA_HALT, //停机状态
    E_SYS_STA_NO_DEF, //未布防
    E_SYS_STA_WAIT_DEF,//等待确认布防
    E_SYS_STA_DEF,  //已布防
    E_SYS_STA_ALARM, //报警状态
};

enum
{
    E_BEEP_PLAY_NONE = 0,
    E_BEEP_PLAY_PU,  //上电提示“嘀”
    E_BEEP_PLAY_DEF, //布防提示
    E_BEEP_PLAY_DEF_OK,//布防成功提示
    E_BEEP_PLAY_TIPS,//布防提示、预报警提示，“嘀嘀”
    E_BEEP_PLAY_LOW_PWR, //低电量提示，“嘀嘀嘀”
    E_BEEP_PLAY_ALARM,  //报警
    
    E_BEEP_PLAY_END,
};

enum
{
    E_BEEP_PLAY_STA_NONE = 0,//空闲，无播放
    E_BEEP_PLAY_STA_PAUSE,    //连续播放时，间隔时处于暂停状态
    E_BEEP_PLAY_STA_REPLAY,    //连续播放时，恢复重放状态
    E_BEEP_PLAY_STA_FIRST,   //第一次播放
    E_BEEP_PLAY_STA_REPEAT,  //重复播放状态
    E_BEEP_PLAY_STA_CONTINUE,//连续播放状态
    E_BEEP_PLAY_STA_LAST,    //最后一次播放
};

//RTC自动唤醒时的配置
enum
{
    E_RTC_WAKEUP_PERIOD_1MS = 1,
    E_RTC_WAKEUP_PERIOD_10MS,
    E_RTC_WAKEUP_PERIOD_1000MS,
};

/*----------------------------- Global Typedefs -----------------------------*/

/*----------------------------- External Variables --------------------------*/
//编译日期，程序版本升级时需要修改
extern const uint8_t sys_complied_date[3];//使用BCD码，年份只取后2位


extern volatile uint8_t global_interrupt_flag;  //用于记录当前的总中断标志

extern uint8_t sys_sta;  //用于记录当前的布撤防状态

extern volatile uint8_t beep_play_type;

extern uint8_t key_val;  //用于记录按键值，0：未释放，1：按下

/*------------------------ Global Function Prototypes -----------------------*/
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
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/03/28, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_system_clock_init(uint32_t sys_clk);

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
extern void bsp_sys_clock_inc(void);

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
extern uint32_t bsp_get_clock(void);

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
extern void bsp_delay_ms_by_rtc(uint16_t n);

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
void bsp_rtc_init(uint16_t wakeup_period);


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
extern void bsp_rtc_deinit(void);


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
extern void bsp_rtc_it_handler(void);

extern void bsp_beep_play(uint8_t type);
extern void bsp_beep_play_it(void);

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
extern void bsp_beep_stop(void);

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
extern void bsp_key_it(void);

#if (SIM_UART_PRINTF_EN)
extern void sim_uart_printf(uint8_t data);
extern void sim_uart_printf_it(uint8_t data);

extern void sim_printf_string(uint8_t *str);
extern void sim_printf_hex(uint8_t data);

#else
#define sim_uart_printf(N)
#define sim_uart_printf_it(N)
#define sim_printf_string(N)
#define sim_printf_hex(N)
#endif

#endif //_MAIN_H_
