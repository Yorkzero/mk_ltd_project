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
	E_LS_STA_WAIT = 0,//�ȴ�
	E_LS_STA_FIRST,   //��һ���źŷ���
	E_LS_STA_COVER,   //��������״̬
	E_LS_STA_OUTTIM,  //��ʱ״̬
};

/************************Var Dec*****************************/
volatile static uint8_t sys_sta_flag = 0; //�ж�ϵͳ�Ƿ����ͣ���ı�־λ
volatile uint8_t ls_sta_flag = E_LS_STA_WAIT;//�жϹ�翪�ش�ʱ������״̬


/*************************************************************
Function Name       : main
Function Description: ������
Param_in            : none
Param_out           : none
Return Type         : int
Note                : 
Author              : Yan
Time                : 2020-10-31
*************************************************************/
int main(void)
{
	//ϵͳʱ�ӳ�ʼ��
	bsp_sys_clk_init();
	//���ϵ��ȶ�
	delay_ms_1(1);
	//����Ϊ����ͣ������ģʽ
	AWU_Cmd(DISABLE);
	CLK_FastHaltWakeUpCmd(DISABLE);
	CLK_SlowActiveHaltWakeUpCmd(ENABLE);
	//halt״̬Ĭ�ϵ��磡
	//FLASH_SetLowPowerMode(FLASH_LPMODE_POWERDOWN);//ͣ��flash����ģʽ
	
	//io�ڳ�ʼ��
	bsp_io_init();
	
	//�������ж�
	BSP_ENABLE_INTERRUPT();

	while (1)
		{
			if (0 == sys_sta_flag)
				{
				//ͣ��׼������
				enableInterrupts();//���ж�
				halt();//����ͣ��
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
	EXTI_SetExtIntSensitivity(Button_EXTI_Pin, EXTI_SENSITIVITY_RISE_ONLY);//�������ش����ж�
	//EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_RISE_ONLY);//�������ش����ж�
	
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
Note                : �ر�����������ʱ�ӣ�������Ҫ�����ڴ˽ӿ�ע��
Author              : Yan
Time                : 2020-11-2
*************************************************************/
void bsp_sys_clk_init(void)
{
	CLK_DeInit();//ʱ�Ӹ�λ
	CLK_HSICmd(ENABLE);//�����ڲ���ƵRC 16M
	CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV1);//һ��Ƶ

	//�ر���������ʱ��  
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
Function Description: ������⺯�����жϵ���
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
Function Name       : bsp_tim4_init
Function Description: 8λTIM���ϼ���������ϵͳʱ��
Param_in            : period 
Param_out           :  
Return Type         : void
Note                : 
Author              : Yan
Time                : 2020-11-3
*************************************************************/
void bsp_tim4_init(u8 period)
{
	TIM4_DeInit();//����tim4�Ĵ���
	CLK_PeripheralClockConfig(CLK_PERIPHERAL_TIMER4, ENABLE);
	
}


/*************************************************************
Function Name       : user_app_run
Function Description: ִ����Ҫ�������
Param_in            :  
Param_out           :  
Return Type         : void
Note                : 
Author              : Yan
Time                : 2020-11-3
*************************************************************/
void user_app_run(void)
{
	Button_DISABLE();//�ȹذ����жϣ����ⷴ������
	//��ʹ�ܿ���
	PWR_ENABLE();
	delay_ms_1(10);//�ȴ���ѹ�ȶ�
	LS_CTRL_ENABLE();
	delay_ms_1(10);
	//�����ʼת��
	Motor_ENABLE();
	//����״̬��
	switch (ls_sta_flag)
		{
		case E_LS_STA_WAIT:
			$end$
			break;
		
		
		}
	
	
	
}



































































































































































