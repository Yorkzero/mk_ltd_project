/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :unlock_module.c 
* Desc     :���仪Ϊ����������ģ�鹦��
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
//���ڼ�¼��ǰ������״̬���¼�
static uint8_t lock_status = E_LOCK_STA_CLOSE , lock_prev_status = E_LOCK_STA_CLOSE , lock_evt = E_LOCK_EVT_NONE;
static uint8_t lock_run_sta = E_UNLOCK_IDLE;
static soft_timet_t unlock_timer = {0 , 0};

//���ڼ�¼��ǰ��������״̬���¼�
static uint8_t hall_status = E_HALL_STA_CLOSE , hall_prev_status = E_HALL_STA_CLOSE , hall_evt = E_HALL_EVT_NONE;

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/
/******************************************************************************
* Name     :unlock_hall_read 
*
* Desc     :ʹ��״̬����ʽ�����ȡ����ֵһ����ȡ��������״̬����Ҫ�ڴ�ѭ���е���
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
    static uint8_t hall_read_sta = 0;//ʹ��״̬����ʽ��ȡ
    static soft_timet_t hall_timer = {0,0};//�����˲���ʱ��
    static uint8_t cnt = 0;//�����˲�����

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

        //�˲���ȷ������
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
                    //�ж�Ϊ����
                    hall_read_sta = 0;
                }
            }
            break;

        //�ȴ��ͷ�
        case 2:
            if(timer_expired(&hall_timer))
            {
                if(RESET != GPIO_ReadInputDataBit(HALL_GPIO_Port , HALL_Pin))
                {
                    cnt++;
                    if(cnt > 5)
                    {
                        cnt = 5;

                        hall_status = 0;  //�ж�Ϊ�ͷ�
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
* Desc     :��ȡ��������״̬�����ⲿ��������
* Param_in :
* Param_out:0���պϣ�1����
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
* Desc     :��ȡ���������¼������ⲿ��������
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
* Desc     :��ȡ������״̬�����ⲿ��������
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
* Desc     :��ȡ�������¼������ⲿ��������
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
* Desc     :����ģ�鴦�������ڴ�ѭ���е���
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
        //���й���״̬
        case E_UNLOCK_IDLE:
            
            break;

        //�ȴ�����̧��
        case E_UNLOCK_WAIT_UNLOCK:
            //���ƿ�����������

            //�����ƹ���ʾ�ͳ�ʱ����
            LED_Flash(E_LED_GREEN , 100 , 50 , 0 , E_LED_STA_OFF);
            timer_set(&unlock_timer , 15000);
            break;

        //�ȴ���ʱ
        case E_UNLOCK_WAIT_TIMEOUT:
            if(timer_expired(&unlock_timer))
            {
                LED_SetStatus(E_LED_GREEN , E_LED_STA_OFF);
                lock_run_sta = E_UNLOCK_IDLE;
                lock_status = E_LOCK_STA_CLOSE;

                //���ƹ�����������
            }
            else
            {
                //�������̧������ת���ȴ�����״̬
                if(1 == hall_status)
                {
                    LED_SetStatus(E_LED_GREEN , E_LED_STA_ON);  //�̵Ƴ���
                    lock_status = E_LOCK_STA_OPEN;
                    
                    lock_run_sta = E_UNLOCK_WAIT_LOCK;
                }
            }
            break;

        //�ȴ�����������Ҫѹ��
        case E_UNLOCK_WAIT_LOCK:
            //�������ѹ�أ���ִ�й�������
            if(E_HALL_STA_CLOSE == hall_status)
            {
                //���ƹ�����������

                
                LED_SetStatus(E_LED_GREEN , E_LED_STA_OFF);  //�̵ƹر�
                lock_status = E_LOCK_STA_CLOSE;
                
                lock_run_sta = E_UNLOCK_IDLE;
            }
            break;

        //�Ƿ�������
        case E_UNLOCK_INVALID_UNLOCK:
            LED_Flash(E_LED_RED , 100 , 50 , 10 , E_LED_STA_OFF);
            lock_run_sta = E_UNLOCK_IDLE;
            lock_status = E_LOCK_STA_CLOSE;
            break;

        default:
            lock_run_sta = E_UNLOCK_IDLE;
            lock_status = E_LOCK_STA_CLOSE;
    }

    //������״̬�¼�
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

    //�������״̬�¼�
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

