/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :unlock_module.h 
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
    E_LOCK_STA_CLOSE = 0x00,  //���պ�
    E_LOCK_STA_OPEN = 0x01,//����
};

enum
{
    E_HALL_STA_CLOSE = 0xAA,  //�����պ�
    E_HALL_STA_OPEN = 0x55,   //������
};

enum
{
    E_LOCK_EVT_NONE = 0,
    E_LOCK_EVT_OPEN = 1,//�����¼�
    E_LOCK_EVT_CLOSE = 2, //�����¼�
};

enum
{
    E_HALL_EVT_NONE = 0,
    E_HALL_EVT_OPEN = 1,//����̧���¼�
    E_HALL_EVT_CLOSE = 2, //����ѹ���¼�
};

enum
{
    E_UNLOCK_REASON_CLOSE = 0x00,
    E_UNLOCK_REASON_CMD = 0x01,  //�����
    E_UNLOCK_REASON_CTL = 0x02,  //����������
    E_UNLOCK_REASON_KEY = 0x03,  //Կ�׿���
};

/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
/******************************************************************************
* Name     :unlock_get_hall_status 
*
* Desc     :��ȡ��������״̬�����ⲿ��������
* Param_in :
* Param_out:E_HALL_STA_CLOSE = 0,  //�����պ�
            E_HALL_STA_OPEN,       //������
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
* Desc     :��ȡ���������¼������ⲿ��������
* Param_in :
* Param_out:E_HALL_EVT_NONE = 0,
            E_HALL_EVT_OPEN = 1,//����̧���¼�
            E_HALL_EVT_CLOSE = 2, //����ѹ���¼�
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
* Desc     :��ȡ������״̬�����ⲿ��������
* Param_in :
* Param_out:E_LOCK_STATUS = 0x00,  //���պ�
            E_UNLOCK_STATUS = 0x01,//����
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
* Desc     :��ȡ�������¼������ⲿ��������
* Param_in :
* Param_out:E_LOCK_EVT_NONE = 0,
            E_LOCK_EVT_LOCK = 1,//�����¼�
            E_LOCK_EVT_UNLOCK = 2, //�����¼�
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
extern void unlock_run(void);




#endif //_UNLOCK_MODULE_H_
