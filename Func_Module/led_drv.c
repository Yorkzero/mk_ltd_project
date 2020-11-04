/******************************************************************************
* Copyright 2019-2024 bobde163
* FileName:led_drv.c 
* Desc    :实现一些LED灯的控制
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
 
 
/*------------------------------- Includes ----------------------------------*/
 #include "led_drv.h"
 #include <string.h>
 #include "timer_set.h"
 
/*------------------- Global Definitions and Declarations -------------------*/
 
 
/*---------------------- Constant / Macro Definitions -----------------------*/

//#define LED_D4_ON()  HAL_GPIO_WritePin(LED1_GPIO_Port , LED1_Pin , GPIO_PIN_RESET)
//#define LED_D4_OFF()  HAL_GPIO_WritePin(LED1_GPIO_Port , LED1_Pin , GPIO_PIN_SET)
//#define LED_D4_TRG()  HAL_GPIO_TogglePin(LED1_GPIO_Port , LED1_Pin);
//
//#define LED_D3_ON()  HAL_GPIO_WritePin(LED2_GPIO_Port , LED2_Pin , GPIO_PIN_RESET)
//#define LED_D3_OFF()  HAL_GPIO_WritePin(LED2_GPIO_Port , LED2_Pin , GPIO_PIN_SET)
//#define LED_D3_TRG()  HAL_GPIO_TogglePin(LED2_GPIO_Port , LED2_Pin);
//
//#define LED_D2_ON()  HAL_GPIO_WritePin(LED3_GPIO_Port , LED3_Pin , GPIO_PIN_RESET)
//#define LED_D2_OFF()  HAL_GPIO_WritePin(LED3_GPIO_Port , LED3_Pin , GPIO_PIN_SET)
//#define LED_D2_TRG()  HAL_GPIO_TogglePin(LED3_GPIO_Port , LED3_Pin);
 
/*----------------------- Type Declarations ---------------------------------*/
typedef struct
{
    GPIO_TypeDef *m_port;
    uint16_t m_pin;
}led_ctrl_pin_t;

//包含有LED控制信息的结构体
typedef struct
{
    //LED灯控制变量
    uint16_t m_freq;     //设定LED灯闪烁频率
    uint16_t m_on_time;  //设定LED灯工作时间长短
    uint8_t m_on_times; //用来记录闪烁的次数
    uint16_t m_timecnt;  //LED灯工作时间计数
    uint8_t m_sta_cur;   //当前LED的开关状态
    uint8_t m_sta_end;   //用于记录在闪烁之后需要设置成的状态
}led_drv_t;

 
/*----------------------- Variable Declarations -----------------------------*/
static volatile uint8_t led_run_flag;  //在10ms定时器中断中设置为1，标志大循环中的LED灯任务可以运行

static const led_ctrl_pin_t led_ctrl_pin_info[E_LED_NUM_MAX] =
{
    {LEDR_GPIO_Port , LEDR_Pin},
    {LEDG_GPIO_Port , LEDG_Pin},
};
    
static led_drv_t led_manage[E_LED_NUM_MAX];
/*----------------------- Function Prototype --------------------------------*/

 
/*----------------------- Function Implement --------------------------------*/
void LED_TimerServer(void)
{
    led_run_flag = 1;
    LED_RealTime();    //还是放到中断里执行，时效性有保障，要是放在大循环里，容易受其他功能模块的延时不确定性影响
}

/******************************************************************************
* Name      :LED_Ctrl 
*
* Desc      :底层用于控制IO口动作，实现LED灯亮灭，同时更新LED的开关状态
* Param(in) :led:led灯编号，范围见头文件中的枚举类型值
             sta：要设置的led的状态
             E_LED_STA_ON:打开
             E_LED_STA_OFF:关闭
             E_LED_MODE_TRG:翻转
* Param(out):
* Return    :
* Global    :
* Note      :移植时需要修改其中的IO驱动函数
* Author    :bobde163
* -------------------------------------
* Log      :2019/04/21, Create this function by bobde163
 ******************************************************************************/
static void LED_Ctrl(uint8_t led , uint8_t sta)
{
    switch(sta)
    {
        case E_LED_STA_ON:
            GPIO_SetBits(led_ctrl_pin_info[led].m_port ,
                              led_ctrl_pin_info[led].m_pin);
            led_manage[led].m_sta_cur = E_LED_STA_ON;
            break;

        case E_LED_STA_OFF:
            GPIO_ResetBits(led_ctrl_pin_info[led].m_port ,
                              led_ctrl_pin_info[led].m_pin );
            led_manage[led].m_sta_cur = E_LED_STA_OFF;
            break;

        case E_LED_STA_TRG:
            GPIO_ToggleBits(led_ctrl_pin_info[led].m_port ,
                               led_ctrl_pin_info[led].m_pin);

            if(E_LED_STA_OFF == led_manage[led].m_sta_cur)
            {
                led_manage[led].m_sta_cur = E_LED_STA_ON;
            }
            else
            {
                led_manage[led].m_sta_cur = E_LED_STA_OFF;
            }
            break;
    }
}


/******************************************************************************
* Name      :LED_SetMode 
*
* Desc      :设置LED灯模式，长亮、长灭或者翻转，会停止当前正在闪烁的任务
* Param(in) :
* Param(out):
* Return    :
* Global    :
* Note      :
* Author    :bobde163
* -------------------------------------
* Log      :2019/04/21, Create this function by bobde163
 ******************************************************************************/
void LED_SetStatus(uint8_t led , uint8_t sta)
{
    if(led >= E_LED_NUM_MAX)
        return;

    led_manage[led].m_freq = 0;
    LED_Ctrl(led , sta);
}

uint8_t LED_GetStatus(uint8_t led)
{
    if(led >= E_LED_NUM_MAX)
        return E_LED_STA_UNKNOW;
    
    return led_manage[led].m_sta_cur;
}


void LED_Init(void)
{
    uint8_t index;

    for(index = 0;index < E_LED_NUM_MAX;index++)
    {
        LED_SetStatus(index , E_LED_STA_OFF);
    }
}


/******************************************************************************
* Name      :LED_Flash 
*
* Desc      :控制LED灯闪烁，或者长亮、长灭
* Param(in) :led：要操作的LED灯，可以同时以相同的参数设置多个LED灯的运行，如LED1|LED2;
             freq：闪烁的周期，freq*10ms，如果为0，则如果on_time = 0灯熄灭，否则灯长亮；
             on_time：一个周期中亮的时间长度，类似于占空比；
             flash_time：要闪烁的次数，如果为0则表示无限次闪烁；
             end_status：指定闪烁结束后，灯的最终的状态是亮是灭，可取值为LED_OFF、LED_ON
* Param(out):
* Return    :
* Global    :
* Note      :
* Author    :bobde163
* -------------------------------------
* Log      :2019/04/21, Create this function by bobde163
 ******************************************************************************/
void LED_Flash(uint8_t    u8_led , uint16_t u16_freq , uint16_t u16_on_time , uint8_t u8_flash_times , uint8_t u8_end_status)
{
    led_drv_t *p_led = &led_manage[u8_led];

    if(u8_led >= E_LED_NUM_MAX)
    {
        return;
    }
    
    p_led->m_timecnt = 0;
    p_led->m_freq = 0;
    p_led->m_on_time = 0;
    p_led->m_on_times = 0;

    //长灭
    if(0 == u16_on_time)
    {
        LED_Ctrl(u8_led , E_LED_STA_OFF);
    }
    //长亮
    if(u16_on_time >= u16_freq)
    {
        LED_Ctrl(u8_led , E_LED_STA_ON);
    }
    else
    {
        //如果闪烁次数为0则表示无限次闪烁，否则补偿1次，使得闪烁能够有限次结束
        if(u8_flash_times > 0)
        {
            p_led->m_on_times = u8_flash_times + 1;
        }
        else
        {
            p_led->m_on_times = 0;
        }

        p_led->m_on_time = u16_on_time;
        p_led->m_sta_end = u8_end_status;
        p_led->m_freq = u16_freq;

        LED_Ctrl(u8_led , E_LED_STA_ON);
    }
}

void LED_RealTime(void)
{
    uint8_t i;
    led_drv_t *p_led;

    static soft_timet_t led_timer = {0 , 0};

    //定时10ms执行一次
    if(!timer_expired(&led_timer))
    {
        return;
    }
    else
    {
        timer_set(&led_timer , 10);
    }

    for(i = 0;i < E_LED_NUM_MAX;i++)
    {
        p_led = &led_manage[i];
        if(p_led->m_freq > 0)
        {
            p_led->m_timecnt++;

            //亮的时间到，进入关闭阶段
            if(p_led->m_timecnt == p_led->m_on_time)
            {
                LED_Ctrl(i , E_LED_STA_OFF);
            }
            else if(p_led->m_timecnt >= p_led->m_freq)
            {
                //无限次闪烁
                if(0 == p_led->m_on_times)
                {
                    p_led->m_timecnt = 0;   //计数值清0，重新新周期计数
                    LED_Ctrl(i , E_LED_STA_ON);
                }
                //有限次闪烁
                else
                {
                    p_led->m_on_times--;
                    //闪烁到达最后一次
                    if(1 == p_led->m_on_times)
                    {
                        p_led->m_freq = 0;   //停止周期闪烁
                        p_led->m_on_time = 0;

                        LED_Ctrl(i , p_led->m_sta_end);
                    }
                    else
                    {
                        p_led->m_timecnt = 0;   //计数值清0，重新新周期计数
                        LED_Ctrl(i , E_LED_STA_ON);
                    }
                }
            }
        }
    }
}
 
/*---------------------------------------------------------------------------*/

