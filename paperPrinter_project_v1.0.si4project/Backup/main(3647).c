/*************************************************************

Copyright 2020 Shawn Yan, All rights reserved
-------------------------------------------------------------
File Name: main.c

Desc     : Paper_Printer

Author   : Shawn Yan
	
Date     : 2020-10-31

*************************************************************/

#include "main.h"
#include "delay.h"

/********************Function Prototype**********************/
void bsp_io_init(void);
void bsp_sys_clk_init(void);
void bsp_button_it(void);
void user_app_run(void);

/************************Typ Dec*****************************/
enum
{
	E_LS_STA_WAIT = 0,//等待
	E_LS_STA_FIRST,   //第一次信号反馈
	E_LS_STA_COVER,   //持续反射状态
	E_LS_STA_OUTTIM,  //超时状态
};

#define DEF_ROT_TIME 600 //默认转动时间

/************************Var Dec*****************************/
volatile static uint8_t sys_sta_flag = 0; //判断系统是否进入停机的标志位
volatile uint8_t ls_sta_flag = E_LS_STA_WAIT;//判断光电开关此时的运行状态
volatile uint8_t ls_outtime_flag = 0;//判断光电开关检测是否超时的标志位
volatile uint16_t ls_outtime_cnt = 1000;//此处设定1s超时
volatile uint16_t ls_time_cnt = 0;//记录前次反射的时间
volatile uint16_t ls_nxt_cnt = 0;//记录这次反射的时间
volatile uint32_t motor_run_time = DEF_ROT_TIME;//记录此次电机转动时间


/*************************************************************
Function Name       : main
Function Description: 主程序
Param_in            : none
Param_out           : none
Return Type         : int
Note                : 
Author              : Yan
Time                : 2020-10-31
*************************************************************/
int main(void)
{
	//系统时钟初始化
	bsp_sys_clk_init();
	//待上电稳定
	delay_ms_1(10);
	//设置为低速停机唤醒模式
	AWU_Cmd(DISABLE);
	CLK_FastHaltWakeUpCmd(DISABLE);
	CLK_SlowActiveHaltWakeUpCmd(ENABLE);
	//halt状态默认掉电！
	//FLASH_SetLowPowerMode(FLASH_LPMODE_POWERDOWN);//停机flash掉电模式
	
	//io口初始化
	bsp_io_init();
	
	//开启总中断
	BSP_ENABLE_INTERRUPT();

	while (1)
		{
			if (0 == sys_sta_flag)
				{
				//停机准备函数
				enableInterrupts();//开中断
				halt();//进入停机
				}
			if (1 == sys_sta_flag)
				{
				user_app_run();
				}
		}
}

/*************************************************************
Function Name       : bsp_io_init
Function Description: Initialization of the IO port used in this program
Param_in            :  
Param_out           :  
Return Type         : void 
Note                : 
Author              : Yan
Time                : 2020-11-2
*************************************************************/
void bsp_io_init(void)
{
	BSP_DISABLE_INTERRUPT();
	EXTI_SetExtIntSensitivity(Button_EXTI_Pin, EXTI_SENSITIVITY_RISE_ONLY);//仅上升沿触发中断
	//EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_RISE_ONLY);//仅上升沿触发中断
	
	GPIO_Init(Button_GPIO_Port, Button_Pin, GPIO_MODE_IN_FL_IT);	
	GPIO_Init(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(Motor_GPIO_Port, Motor_Pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LS_CTRL_GPIO_Port, LS_CTRL_Pin, GPIO_MODE_OUT_PP_LOW_SLOW);
	GPIO_Init(LS_SIGNAL_GPIO_Port, LS_SIGNAL_Pin, GPIO_MODE_IN_FL_NO_IT);

	//unused pin
	GPIO_Init(GPIOA, PA_UNUSED_Pin, GPIO_MODE_IN_PU_NO_IT);
	GPIO_Init(GPIOB, PB_UNUSED_Pin, GPIO_MODE_IN_PU_NO_IT);
	GPIO_Init(GPIOC, PC_UNUSED_Pin, GPIO_MODE_IN_PU_NO_IT);
	GPIO_Init(GPIOD, PD_UNUSED_Pin, GPIO_MODE_IN_PU_NO_IT);
}

/*************************************************************
Function Name       : bsp_sys_clk_init
Function Description: Init. of CPU clk, and all peripheral clk are disabled.
Param_in            : 
Param_out           :  
Return Type         : void
Note                : 关闭了所有外设时钟，如有需要，请在此接口注释
Author              : Yan
Time                : 2020-11-2
*************************************************************/
void bsp_sys_clk_init(void)
{
	CLK_DeInit();//时钟复位
	CLK_HSICmd(ENABLE);//开启内部高频RC 16M
	CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);//一分频

	//关闭所有外设时钟  
  	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER1,DISABLE);
  	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER2,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER3,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_ADC,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_AWU,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI,DISABLE);
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C,DISABLE);
	
}

/*************************************************************
Function Name       : bsp_button_it
Function Description: 按键检测函数，中断调用
Param_in            :  
Param_out           :  
Return Type         : void
Note                : 
Author              : Yan
Time                : 2020-11-2
*************************************************************/
void bsp_button_it(void)
{
	delay_ms_1(20);
	if (Button_Read())
		sys_sta_flag = 1;
}

/*************************************************************
Function Name       : user_app_run
Function Description: 执行主要电机动作
Param_in            :  
Param_out           :  
Return Type         : void
Note                : 
Author              : Yan
Time                : 2020-11-3
*************************************************************/
void user_app_run(void)
{
	Button_DISABLE();//先关按键中断，避免反复触发
	//开使能开关
	PWR_ENABLE();
	delay_ms_1(10);//等待升压稳定
	LS_CTRL_ENABLE();
	delay_ms_1(10);
	//电机开始转动
	Motor_ENABLE();

	if (E_LS_STA_WAIT == ls_sta_flag)
		{
		while (!LS_SIGNAL_Read())
			{
			//此处开始判断超时
			delay_ms_1(1);
			ls_outtime_cnt--;
			if (0 == ls_outtime_cnt)
				{
				ls_outtime_flag = 1;
				ls_sta_flag = E_LS_STA_OUTTIM;
				}
			}
		if (ls_nxt_cnt == ls_time_cnt)//初次上电动作
			{
			ls_nxt_cnt = 1000 - ls_outtime_cnt;
			ls_time_cnt = 1000 - ls_outtime_cnt;
			}
		ls_nxt_cnt = 1000 - ls_outtime_cnt;
		ls_outtime_cnt = 1000;//重置为初始值
		delay_ms_1(1);
		if ((LS_SIGNAL_Read())  && (!ls_outtime_flag))
			{
			ls_sta_flag = E_LS_STA_FIRST;
			}
		}
	if (E_LS_STA_OUTTIM == ls_sta_flag)
		{
		ls_outtime_flag = 0;//重置为初始值
		Motor_DISABLE();
		LS_CTRL_DISABLE();
		PWR_DISABLE();
		Button_ENABLE();
		ls_time_cnt = 0;
		ls_nxt_cnt = 0;
		sys_sta_flag = 0;
		ls_sta_flag = E_LS_STA_WAIT;
		return;
		}
	if (E_LS_STA_FIRST == ls_sta_flag)
		{
		motor_run_time *= ls_nxt_cnt;
		motor_run_time /= ls_time_cnt;
		ls_sta_flag = E_LS_STA_COVER;
		}
	if (E_LS_STA_COVER == ls_sta_flag)
		{
		delay_ms_1((uint16_t) motor_run_time);
		}
	//状态重置，回到停机中断
	Motor_DISABLE();
	LS_CTRL_DISABLE();
	PWR_DISABLE();
	Button_ENABLE();
	ls_time_cnt = ls_nxt_cnt;//这一次的测量时间即为下一次的参考时间
	ls_nxt_cnt = 0;
	sys_sta_flag = 0;
	ls_sta_flag = E_LS_STA_WAIT;
	
}




































































































































































