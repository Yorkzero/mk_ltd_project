/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :unlock_module.c 
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


/*------------------------------- Includes ----------------------------------*/
#include "unlock_module.h"
#include "led_drv.h"
#include "timer_set.h"

/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/
//用于记录当前锁开关状态和事件
static uint8_t lock_status = E_LOCK_STA_CLOSE , lock_prev_status = E_LOCK_STA_CLOSE , lock_evt = E_LOCK_EVT_NONE;
static uint8_t lock_run_sta = E_UNLOCK_IDLE;
static soft_timet_t unlock_timer = {0 , 0};

//用于记录当前霍尔开关状态和事件
static uint8_t hall_status = E_HALL_STA_CLOSE , hall_prev_status = E_HALL_STA_CLOSE , hall_evt = E_HALL_EVT_NONE;

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name     :unlock_hall_read 
*
* Desc     :使用状态机方式，像读取按键值一样获取霍尔开关状态，需要在大循环中调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
static void unlock_hall_read(void)
{
    static uint8_t hall_read_sta = 0;//使用状态机方式读取
    static soft_timet_t hall_timer = {0,0};//用作滤波计时用
    static uint8_t cnt = 0;//用于滤波计数

    switch(hall_read_sta)
    {
        case 0:
            if(RESET == GPIO_ReadInputDataBit(HALL_GPIO_Port , HALL_Pin))
            {
                cnt = 0;
                timer_set(&hall_timer , 10);

                hall_status = 0;

                hall_read_sta = 1;
            }
            break;

        //滤波，确定触发
        case 1:
            if(timer_expired(&hall_timer))
            {
                if(RESET == GPIO_ReadInputDataBit(HALL_GPIO_Port , HALL_Pin))
                {
                    cnt++;
                    if(cnt > 5)
                    {
                        cnt = 0;

                        hall_status = 1;

                        hall_read_sta = 2;
                    }
                    
                    timer_set(&hall_timer , 10);
                }
                else
                {
                    //判断为干扰
                    hall_read_sta = 0;
                }
            }
            break;

        //等待释放
        case 2:
            if(timer_expired(&hall_timer))
            {
                if(RESET != GPIO_ReadInputDataBit(HALL_GPIO_Port , HALL_Pin))
                {
                    cnt++;
                    if(cnt > 5)
                    {
                        cnt = 5;

                        hall_status = 0;  //判断为释放
                    }
                }

                timer_set(&hall_timer , 10);
            }
            break;

        default:
            hall_read_sta = 0;
            cnt = 0;
            hall_status = 0;
            break;
    }
}

/******************************************************************************
* Name     :unlock_get_hall_status 
*
* Desc     :获取霍尔开关状态，供外部函数调用
* Param_in :
* Param_out:0：闭合，1：打开
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
uint8_t unlock_get_hall_status(void)
{
    return hall_status;
}

/******************************************************************************
* Name     :unlock_get_hall_evt 
*
* Desc     :获取霍尔开关事件，供外部函数调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
uint8_t unlock_get_hall_evt(void)
{
    return hall_evt;
}

/******************************************************************************
* Name     :unlock_get_lock_status 
*
* Desc     :获取锁开关状态，供外部函数调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
uint8_t unlock_get_lock_status(void)
{
    return lock_status;
}

/******************************************************************************
* Name     :unlock_get_lock_evt 
*
* Desc     :获取锁开关事件，供外部函数调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/24, Create this function by CenLinbo
 ******************************************************************************/
uint8_t unlock_get_lock_evt(void)
{
    return lock_evt;
}


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
void unlock_run(void)
{
    unlock_hall_read();
    
    switch(lock_run_sta)
    {
        //空闲关锁状态
        case E_UNLOCK_IDLE:
            
            break;

        //等待锁柄抬起
        case E_UNLOCK_WAIT_UNLOCK:
            //控制开锁，待编码

            //开启灯光提示和超时机制
            LED_Flash(E_LED_GREEN , 100 , 50 , 0 , E_LED_STA_OFF);
            timer_set(&unlock_timer , 15000);
            break;

        //等待超时
        case E_UNLOCK_WAIT_TIMEOUT:
            if(timer_expired(&unlock_timer))
            {
                LED_SetStatus(E_LED_GREEN , E_LED_STA_OFF);
                lock_run_sta = E_UNLOCK_IDLE;
                lock_status = E_LOCK_STA_CLOSE;

                //控制关锁，待编码
            }
            else
            {
                //如果锁柄抬起，则跳转到等待关锁状态
                if(1 == hall_status)
                {
                    LED_SetStatus(E_LED_GREEN , E_LED_STA_ON);  //绿灯长亮
                    lock_status = E_LOCK_STA_OPEN;
                    
                    lock_run_sta = E_UNLOCK_WAIT_LOCK;
                }
            }
            break;

        //等待关锁，锁柄要压回
        case E_UNLOCK_WAIT_LOCK:
            //如果锁柄压回，则执行关锁操作
            if(E_HALL_STA_CLOSE == hall_status)
            {
                //控制关锁，待编码

                
                LED_SetStatus(E_LED_GREEN , E_LED_STA_OFF);  //绿灯关闭
                lock_status = E_LOCK_STA_CLOSE;
                
                lock_run_sta = E_UNLOCK_IDLE;
            }
            break;

        //非法卡开锁
        case E_UNLOCK_INVALID_UNLOCK:
            LED_Flash(E_LED_RED , 100 , 50 , 10 , E_LED_STA_OFF);
            lock_run_sta = E_UNLOCK_IDLE;
            lock_status = E_LOCK_STA_CLOSE;
            break;

        default:
            lock_run_sta = E_UNLOCK_IDLE;
            lock_status = E_LOCK_STA_CLOSE;
    }

    //处理锁状态事件
    if((E_LOCK_STA_CLOSE == lock_prev_status) && (E_LOCK_STA_OPEN == lock_status))
    {
        lock_evt = E_LOCK_EVT_OPEN;
        lock_prev_status = lock_status;
    }
    else if((E_LOCK_STA_OPEN == lock_prev_status) && (E_LOCK_STA_CLOSE == lock_status))
    {
        lock_evt = E_LOCK_EVT_CLOSE;
        lock_prev_status = lock_status;
    }
    else
    {
        lock_evt = E_LOCK_EVT_NONE;
    }

    //处理霍尔状态事件
    if((E_HALL_STA_CLOSE == hall_prev_status) && (E_HALL_STA_OPEN == hall_prev_status))
    {
        hall_evt = E_HALL_EVT_OPEN;
        hall_prev_status = hall_status;
    }
    else if((E_HALL_STA_OPEN == lock_prev_status) && (E_HALL_STA_CLOSE == lock_prev_status))
    {
        hall_evt = E_HALL_EVT_CLOSE;
        hall_prev_status = hall_status;
    }
    else
    {
        hall_evt = E_HALL_EVT_NONE;
    }
}

/*---------------------------------------------------------------------------*/

