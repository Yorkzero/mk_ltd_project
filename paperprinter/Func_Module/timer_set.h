/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :timer_set.h 
* Desc     :
* 
* 
* Author   :CenLinbo
* Date     :2020/08/22
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/22, CenLinbo create this file
*         
******************************************************************************/
#ifndef _TIMER_SET_H_     
#define _TIMER_SET_H_    


/*------------------------------- Includes ----------------------------------*/
#include "main.h"

/*----------------------------- Global Defines ------------------------------*/
/**
 * A timer.
 *
 * This structure is used for declaring a timer. The timer must be set
 * with timer_set() before it can be used.
 *
 * \hideinitializer
 */
typedef struct
{
    uint32_t start;
    uint32_t interval;
}soft_timet_t;


/*----------------------------- Global Typedefs -----------------------------*/


/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/
extern void timer_set(soft_timet_t *t, uint32_t interval);
extern void timer_restart(soft_timet_t *t);
extern uint8_t timer_expired(soft_timet_t *t);

#endif /* __TIMER_H__ */
