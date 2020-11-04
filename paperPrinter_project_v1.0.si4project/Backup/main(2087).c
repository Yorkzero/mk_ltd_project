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


/************************Var Dec*****************************/
volatile static uint8_t sys_sta_flag = 0; //判断系统是否进入停机的标志位



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
	delay_ms_1(1);
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
	GPIO_Init(LS_SIGNAL_GPIO_Port, LS_SIGNAL_Pin, GPIO_MODE_IN_FL_IT);

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
	//开使能开关
	PWR_ENABLE();
	delay_ms_1(10);//等待升压稳定
	LS_CTRL_ENABLE();
	delay_ms_1(10);
	//电机开始转动
	Motor_ENABLE();

	
	
	
}




































































































































































