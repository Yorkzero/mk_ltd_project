/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :device_config.c 
* Desc     :���������ò�����صĹ���
* 
* 
* Author   :CenLinbo
* Date     :2020/08/28
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/28, CenLinbo create this file
*         
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "device_config.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
//ģ�����ƣ���Ч����Ϊ12������12���Կո����
const char dev_cfg_dev_name[13] = "RF125K-Lk   ";

//ϵͳ˵������Ч����Ϊ14������14���Կո����
const char dev_cfg_dev_discribe[15] = "HW-2806       ";

//����(�豸С��)�����ո�
const char dev_cfg_sub_dev_type[9] = "        ";

uint8_t dev_cfg_unlock_alarm_en = E_DEV_CONFIG_ENABLE;  //��������ʹ������
uint8_t dev_cfg_user_oprate_alarm_en = E_DEV_CONFIG_ENABLE;  //�û���������ʹ������
uint8_t dev_cfg_hardware_fault = E_DEV_ALARM_NONE;  //Ӳ�����ϱ���״̬
uint8_t dev_cfg_delay_close_time = 3; //�ӳٹ���ʱ�䣬��λΪ�룬��С3�룬���255��
uint8_t dev_cfg_led_flash_time = 10;  //LED��˸ʱ�䣬��λΪ�룬���255��







/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/


/*---------------------------------------------------------------------------*/

