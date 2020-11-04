/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :device_config.c 
* Desc     :处理与配置参数相关的功能
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
//模块名称，有效长度为12，不足12则以空格填充
const char dev_cfg_dev_name[13] = "RF125K-Lk   ";

//系统说明，有效长度为14，不足14则以空格填充
const char dev_cfg_dev_discribe[15] = "HW-2806       ";

//机型(设备小类)，填充空格
const char dev_cfg_sub_dev_type[9] = "        ";

uint8_t dev_cfg_unlock_alarm_en = E_DEV_CONFIG_ENABLE;  //开锁报警使能配置
uint8_t dev_cfg_user_oprate_alarm_en = E_DEV_CONFIG_ENABLE;  //用户操作报警使能配置
uint8_t dev_cfg_hardware_fault = E_DEV_ALARM_NONE;  //硬件故障报警状态
uint8_t dev_cfg_delay_close_time = 3; //延迟关锁时间，单位为秒，最小3秒，最大255秒
uint8_t dev_cfg_led_flash_time = 10;  //LED闪烁时间，单位为秒，最大255秒







/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/


/*---------------------------------------------------------------------------*/

