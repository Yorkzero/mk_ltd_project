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

#define DEF_ROT_TIME 600 //Ĭ��ת��ʱ��

/************************Var Dec*****************************/
volatile static uint8_t sys_sta_flag = 0; //�ж�ϵͳ�Ƿ����ͣ���ı�־λ
volatile uint8_t ls_sta_flag = E_LS_STA_WAIT;//�жϹ�翪�ش�ʱ������״̬
volatile uint8_t ls_outtime_flag = 0;//�жϹ�翪�ؼ���Ƿ�ʱ�ı�־λ
volatile uint16_t ls_outtime_cnt = 1000;//�˴��趨1s��ʱ
volatile uint16_t ls_time_cnt = 0;//��¼ǰ�η����ʱ��
volatile uint16_t ls_nxt_cnt = 0;//��¼��η����ʱ��
volatile uint32_t motor_run_time = DEF_ROT_TIME;//��¼�˴ε��ת��ʱ��


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
	delay_ms_1(10);
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

	if (E_LS_STA_WAIT == ls_sta_flag)
		{
		while (!LS_SIGNAL_Read())
			{
			//�˴���ʼ�жϳ�ʱ
			delay_ms_1(1);
			ls_outtime_cnt--;
			if (0 == ls_outtime_cnt)
				{
				ls_outtime_flag = 1;
				ls_sta_flag = E_LS_STA_OUTTIM;
				}
			}
		if (ls_nxt_cnt == ls_time_cnt)//�����ϵ綯��
			{
			ls_nxt_cnt = 1000 - ls_outtime_cnt;
			ls_time_cnt = 1000 - ls_outtime_cnt;
			}
		ls_nxt_cnt = 1000 - ls_outtime_cnt;
		ls_outtime_cnt = 1000;//����Ϊ��ʼֵ
		delay_ms_1(1);
		if ((LS_SIGNAL_Read())  && (!ls_outtime_flag))
			{
			ls_sta_flag = E_LS_STA_FIRST;
			}
		}
	if (E_LS_STA_OUTTIM == ls_sta_flag)
		{
		ls_outtime_flag = 0;//����Ϊ��ʼֵ
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
	//״̬���ã��ص�ͣ���ж�
	Motor_DISABLE();
	LS_CTRL_DISABLE();
	PWR_DISABLE();
	Button_ENABLE();
	ls_time_cnt = ls_nxt_cnt;//��һ�εĲ���ʱ�伴Ϊ��һ�εĲο�ʱ��
	ls_nxt_cnt = 0;
	sys_sta_flag = 0;
	ls_sta_flag = E_LS_STA_WAIT;
	
}




































































































































































