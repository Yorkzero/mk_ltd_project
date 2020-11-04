/******************************************************************************
* Copyright 2019-2024 bobde163
* FileName:led_drv.h 
* Desc    :����������led���ƺ궨��͹��ܺ���
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

//LED������صķ���������10ms��ʱ���ж��е���
//������ڴ�ѭ���е��ô�����������10ms��ʱ���ж��е���
//extern void LED_TimerServer(void);

//����LED���ǿ����ػ��߷�ת�����жϵ�ǰ����˸
extern void LED_SetStatus(uint8_t led , uint8_t sta);

//��ѯָ��LED�Ƶ�ǰ��״̬
extern uint8_t LED_GetStatus(uint8_t led);

//��ʼ��LED��ģ��
extern void LED_Init(void);

//����LED����˸
extern void LED_Flash(uint8_t    u8_led , uint16_t u16_freq , uint16_t u16_on_time , uint8_t u8_flash_times , uint8_t u8_led_status);

//��ѭ����һֱ���ã�����ʵ��LEDģ�鹦��
extern void LED_RealTime(void);
 
#endif //_LED_DRV_H_
 
