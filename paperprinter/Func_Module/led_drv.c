/******************************************************************************
* Copyright 2019-2024 bobde163
* FileName:led_drv.c 
* Desc    :ʵ��һЩLED�ƵĿ���
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

//������LED������Ϣ�Ľṹ��
typedef struct
{
    //LED�ƿ��Ʊ���
    uint16_t m_freq;     //�趨LED����˸Ƶ��
    uint16_t m_on_time;  //�趨LED�ƹ���ʱ�䳤��
    uint8_t m_on_times; //������¼��˸�Ĵ���
    uint16_t m_timecnt;  //LED�ƹ���ʱ�����
    uint8_t m_sta_cur;   //��ǰLED�Ŀ���״̬
    uint8_t m_sta_end;   //���ڼ�¼����˸֮����Ҫ���óɵ�״̬
}led_drv_t;

 
/*----------------------- Variable Declarations -----------------------------*/
static volatile uint8_t led_run_flag;  //��10ms��ʱ���ж�������Ϊ1����־��ѭ���е�LED�������������

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
    LED_RealTime();    //���Ƿŵ��ж���ִ�У�ʱЧ���б��ϣ�Ҫ�Ƿ��ڴ�ѭ�����������������ģ�����ʱ��ȷ����Ӱ��
}

/******************************************************************************
* Name      :LED_Ctrl 
*
* Desc      :�ײ����ڿ���IO�ڶ�����ʵ��LED������ͬʱ����LED�Ŀ���״̬
* Param(in) :led:led�Ʊ�ţ���Χ��ͷ�ļ��е�ö������ֵ
             sta��Ҫ���õ�led��״̬
             E_LED_STA_ON:��
             E_LED_STA_OFF:�ر�
             E_LED_MODE_TRG:��ת
* Param(out):
* Return    :
* Global    :
* Note      :��ֲʱ��Ҫ�޸����е�IO��������
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
* Desc      :����LED��ģʽ��������������߷�ת����ֹͣ��ǰ������˸������
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
* Desc      :����LED����˸�����߳���������
* Param(in) :led��Ҫ������LED�ƣ�����ͬʱ����ͬ�Ĳ������ö��LED�Ƶ����У���LED1|LED2;
             freq����˸�����ڣ�freq*10ms�����Ϊ0�������on_time = 0��Ϩ�𣬷���Ƴ�����
             on_time��һ������������ʱ�䳤�ȣ�������ռ�ձȣ�
             flash_time��Ҫ��˸�Ĵ��������Ϊ0���ʾ���޴���˸��
             end_status��ָ����˸�����󣬵Ƶ����յ�״̬�������𣬿�ȡֵΪLED_OFF��LED_ON
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

    //����
    if(0 == u16_on_time)
    {
        LED_Ctrl(u8_led , E_LED_STA_OFF);
    }
    //����
    if(u16_on_time >= u16_freq)
    {
        LED_Ctrl(u8_led , E_LED_STA_ON);
    }
    else
    {
        //�����˸����Ϊ0���ʾ���޴���˸�����򲹳�1�Σ�ʹ����˸�ܹ����޴ν���
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

    //��ʱ10msִ��һ��
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

            //����ʱ�䵽������رս׶�
            if(p_led->m_timecnt == p_led->m_on_time)
            {
                LED_Ctrl(i , E_LED_STA_OFF);
            }
            else if(p_led->m_timecnt >= p_led->m_freq)
            {
                //���޴���˸
                if(0 == p_led->m_on_times)
                {
                    p_led->m_timecnt = 0;   //����ֵ��0�����������ڼ���
                    LED_Ctrl(i , E_LED_STA_ON);
                }
                //���޴���˸
                else
                {
                    p_led->m_on_times--;
                    //��˸�������һ��
                    if(1 == p_led->m_on_times)
                    {
                        p_led->m_freq = 0;   //ֹͣ������˸
                        p_led->m_on_time = 0;

                        LED_Ctrl(i , p_led->m_sta_end);
                    }
                    else
                    {
                        p_led->m_timecnt = 0;   //����ֵ��0�����������ڼ���
                        LED_Ctrl(i , E_LED_STA_ON);
                    }
                }
            }
        }
    }
}
 
/*---------------------------------------------------------------------------*/

