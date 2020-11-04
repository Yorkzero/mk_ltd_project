/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :user_app.h 
* Desc     :Ӧ�ô���
* 
* 
* Author   :CenLinbo
* Date     :2020/09/18
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/09/18, CenLinbo create this file
*         
******************************************************************************/
#ifndef _USER_APP_H_     
#define _USER_APP_H_    


/*------------------------------- Includes ----------------------------------*/
#include "main.h"

/*----------------------------- Global Defines ------------------------------*/


/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/
extern volatile uint8_t vib_sta;
extern volatile uint16_t vib_flag;

/*------------------------ Global Function Prototypes -----------------------*/
/******************************************************************************
* Name     :user_app_vib_detect_it 
*
* Desc     :�������ж��е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/18, Create this function by CenLinbo
 ******************************************************************************/
extern void user_app_vib_detect_it(void);

/******************************************************************************
* Name     :user_app_vib_detect_run 
*
* Desc     :�ڴ�ѭ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/18, Create this function by CenLinbo
 ******************************************************************************/
extern void user_app_vib_detect_run(void);

/******************************************************************************
* Name     :user_app_run 
*
* Desc     :Ӧ�ò㴦�������ڴ�ѭ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
extern void user_app_run(void);



#endif //_USER_APP_H_
