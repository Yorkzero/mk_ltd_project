/******************************************************************************
* Copyright 2019-2024 bobde163
* FileName:led_drv.h 
* Desc    :包含基础的led控制宏定义和功能函数
* 
* 
* Author  :bobde163
* Date    :2019/04/21
* Notes   :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2019/04/21, bobde163 create this file
* 
******************************************************************************/
#ifndef _LED_DRV_H_     
#define _LED_DRV_H_    
 
 
/*------------------------------- Includes ----------------------------------*/
 #include "main.h"
 
/*----------------------------- Global Defines ------------------------------*/
 enum led_status
{
    E_LED_STA_OFF = 0,
    E_LED_STA_ON,
    E_LED_STA_TRG,
    E_LED_STA_UNKNOW,
};

enum led_index
{
    E_LED_RED = 0,
    E_LED_GREEN,

    E_LED_NUM_MAX,
};
/*----------------------------- Global Typedefs -----------------------------*/



 
/*----------------------------- External Variables --------------------------*/
 
 
/*------------------------ Global Function Prototypes -----------------------*/

//LED操作相关的服务函数，在10ms定时器中断中调用
//本软件在大循环中调用处理函数，不在10ms定时器中断中调用
//extern void LED_TimerServer(void);

//设置LED灯是开、关或者翻转，会中断当前的闪烁
extern void LED_SetStatus(uint8_t led , uint8_t sta);

//查询指定LED灯当前的状态
extern uint8_t LED_GetStatus(uint8_t led);

//初始化LED灯模块
extern void LED_Init(void);

//控制LED灯闪烁
extern void LED_Flash(uint8_t    u8_led , uint16_t u16_freq , uint16_t u16_on_time , uint8_t u8_flash_times , uint8_t u8_led_status);

//在循环中一直调用，用于实现LED模块功能
extern void LED_RealTime(void);
 
#endif //_LED_DRV_H_
 
