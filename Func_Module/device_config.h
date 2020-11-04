/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :device_config.h 
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
#ifndef _DEVICE_CONFIG_H_     
#define _DEVICE_CONFIG_H_    


/*------------------------------- Includes ----------------------------------*/
#include "main.h"

/*----------------------------- Global Defines ------------------------------*/
enum
{
    E_SUPPORT_TAG = 0x55,  //支持加载
    E_NOT_SUPPORT_TAG = 0xAA, //不支持加载
};

enum
{
    E_DEV_CONFIG_ENABLE = 0x55,  //使能配置
    E_DEV_CONFIG_DISABLE = 0xAA, //禁用配置
};

enum
{
    E_DEV_ALARM_NONE = 0x55,
    E_DEV_ALARM_OCCUR = 0xAA,  //发生报警
    E_DEV_ALARM_INVALID = 0xFF,  //无效
};

enum
{
    E_POWER_12V = 0x01,
    E_POWER_24V = 0x01,
};
    
#define DEV_CFG_PROTOCOL_VER    0x0003
#define DEV_CFG_SOFT_VER        0x0100
#define DEV_CFG_PCB_VER         'A'
#define DEV_CFG_DEV_TYPE        0x0003
#define DEV_CFG_MANUFACTUTER    0x0002
#define DEV_CFG_LOAD            E_NOT_SUPPORT_TAG
#define DEV_CFG_PLC_VER         0x0000
#define DEV_CFG_COMPILED_DATE_YEAR  0x0020
#define DEV_CFG_COMPILED_DATE_MONTH 0x0008
#define DEV_CFG_COMPILED_DATE_DAY   0x0028
#define DEV_CFG_DEV_ATTR         0x0001
#define DEV_CFG_SUB_SOFT_VER     0x00FF

#define DEV_CFG_BOARD_ID         0x00000003
#define DEV_CFG_POWER_TYPE       E_POWER_12V
#define DEV_CFG_AUTO_CLOSE       E_NOT_SUPPORT_TAG
#define DEV_CFG_LEVEL_UNLOCK       E_NOT_SUPPORT_TAG
#define DEV_CFG_UNLOCK_MONITOR   E_SUPPORT_TAG
#define DEV_CFG_LED              E_SUPPORT_TAG
#define DEV_CFG_RFID_UNLOCK      E_SUPPORT_TAG
#define DEV_CFG_KEY_UNLOCK       E_SUPPORT_TAG
#define DEV_CFG_TAG_LEN          4   //电子标签长度，暂时不知道何意



/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/
//模块名称，有效长度为12，不足12则以空格填充
extern const char sys_device_name[13];

//系统说明，有效长度为14，不足14则以空格填充
extern const char sys_device_discribe[15];

//机型(设备小类)，填充空格
extern const char dev_cfg_sub_dev_type[9];


/*------------------------ Global Function Prototypes -----------------------*/



#endif //_DEVICE_CONFIG_H_
