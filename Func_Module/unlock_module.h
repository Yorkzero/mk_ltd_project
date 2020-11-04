/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :unlock_module.h 
* Desc     :适配华为把手锁开锁模块功能
* 
* 
* Author   :CenLinbo
* Date     :2020/08/24
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/24, CenLinbo create this file
*         
******************************************************************************/
#ifndef _UNLOCK_MODULE_H_     
#define _UNLOCK_MODULE_H_    


/*------------------------------- Includes ----------------------------------*/
#include "main.h"

/*----------------------------- Global Defines ------------------------------*/


/*----------------------------- Global Typedefs -----------------------------*/
enum
{
    E_UNLOCK_IDLE = 0,
    E_UNLOCK_WAIT_UNLOCK,
    E_UNLOCK_WAIT_TIMEOUT,
    E_UNLOCK_WAIT_LOCK,
    E_UNLOCK_INVALID_UNLOCK,
};

enum
{
    E_LOCK_STA_CLOSE = 0x00,  //锁闭合
    E_LOCK_STA_OPEN = 0x01,//锁打开
};

enum
{
    E_HALL_STA_CLOSE = 0xAA,  //霍尔闭合
    E_HALL_STA_OPEN = 0x55,   //霍尔打开
};

enum
{
    E_LOCK_EVT_NONE = 0,
    E_LOCK_EVT_OPEN = 1,//开锁事件
    E_LOCK_EVT_CLOSE = 2, //关锁事件
};

enum
{
    E_HALL_EVT_NONE = 0,
    E_HALL_EVT_OPEN = 1,//把手抬起事件
    E_HALL_EVT_CLOSE = 2, //把手压下事件
};

enum
{
    E_UNLOCK_REASON_CLOSE = 0x00,
    E_UNLOCK_REASON_CMD = 0x01,  //命令开锁
    E_UNLOCK_REASON_CTL = 0x02,  //控制量开锁
    E_UNLOCK_REASON_KEY = 0x03,  //钥匙开锁
};

/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
/******************************************************************************
* Name     :unlock_get_hall_status 
*
* Desc     :获取霍尔开关状态，供外部函数调用
* Param_in :
* Param_out:E_HALL_STA_CLOSE = 0,  //霍尔闭合
            E_HALL_STA_OPEN,       //霍尔打开
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
extern uint8_t unlock_get_hall_status(void);

/******************************************************************************
* Name     :unlock_get_hall_evt 
*
* Desc     :获取霍尔开关事件，供外部函数调用
* Param_in :
* Param_out:E_HALL_EVT_NONE = 0,
            E_HALL_EVT_OPEN = 1,//把手抬起事件
            E_HALL_EVT_CLOSE = 2, //把手压下事件
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
extern uint8_t unlock_get_hall_evt(void);

/******************************************************************************
* Name     :unlock_get_lock_status 
*
* Desc     :获取锁开关状态，供外部函数调用
* Param_in :
* Param_out:E_LOCK_STATUS = 0x00,  //锁闭合
            E_UNLOCK_STATUS = 0x01,//锁打开
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
extern uint8_t unlock_get_lock_status(void);

/******************************************************************************
* Name     :unlock_get_lock_evt 
*
* Desc     :获取锁开关事件，供外部函数调用
* Param_in :
* Param_out:E_LOCK_EVT_NONE = 0,
            E_LOCK_EVT_LOCK = 1,//开锁事件
            E_LOCK_EVT_UNLOCK = 2, //关锁事件
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
extern uint8_t unlock_get_lock_evt(void);


/******************************************************************************
* Name     :unlock_run 
*
* Desc     :开锁模块处理函数，在大循环中调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
extern void unlock_run(void);




#endif //_UNLOCK_MODULE_H_
