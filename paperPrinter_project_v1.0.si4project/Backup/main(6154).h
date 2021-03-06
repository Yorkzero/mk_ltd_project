/*************************************************************
Copyright 2020 Shawn Yan, All rights reserved

-------------------------------------------------------------
File Name: main.h

Desc     :  

Author   : Shawn Yan

Date     : 2020-11-2

*************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

/***********************Includes*****************************/
#include "stm8s.h"
#include <stddef.h>

/********************Global Defines**************************/
//@                  GPIO Defines

//Button_Signal
#define Button_GPIO_Port        GPIOD
#define Button_Pin              GPIO_PIN_6
#define Button_EXTI_Pin         EXTI_PORT_GPIOD

#define Button_Read()           (Button_GPIO_Port->IDR & Button_Pin)
#define Button_ENABLE()         GPIO_Init(Button_GPIO_Port, Button_Pin, GPIO_MODE_IN_FL_IT)
#define Button_DISABLE()        GPIO_Init(Button_GPIO_Port, Button_Pin, GPIO_MODE_IN_FL_NO_IT)

//DCDC_Ctrl
#define PWR_EN_GPIO_Port        GPIOC
#define PWR_EN_Pin              GPIO_PIN_6

#define PWR_ENABLE()            (PWR_EN_GPIO_Port->ODR |= PWR_EN_Pin)
#define PWR_DISABLE()           (PWR_EN_GPIO_Port->ODR &= ~PWR_EN_Pin)

//Motor_Ctrl
#define Motor_GPIO_Port         GPIOC
#define Motor_Pin               GPIO_PIN_5

#define Motor_ENABLE()          (Motor_GPIO_Port->ODR |= Motor_Pin)
#define Motor_DISABLE()         (Motor_GPIO_Port->ODR &= ~Motor_Pin)

//@Light_Switch_IO
//LS_Ctrl
#define LS_CTRL_GPIO_Port       GPIOC
#define LS_CTRL_Pin             GPIO_PIN_7

#define LS_CTRL_ENABLE()		(LS_CTRL_GPIO_Port->ODR |= LS_CTRL_Pin)
#define LS_CTRL_DISABLE()   	(LS_CTRL_GPIO_Port->ODR &= ~LS_CTRL_Pin)

//LS_Signal
#define LS_SIGNAL_GPIO_Port     GPIOD
#define LS_SIGNAL_Pin           GPIO_PIN_2

#define LS_SIGNAL_Read()        (LS_SIGNAL_GPIO_Port->IDR & LS_SIGNAL_Pin)

//Unused Pin
#define PA_UNUSED_Pin           (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)
#define PB_UNUSED_Pin           (GPIO_PIN_4 | GPIO_PIN_5)
#define PC_UNUSED_Pin           (GPIO_PIN_3 | GPIO_PIN_4)
#define PD_UNUSED_Pin           (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)

//@                 Interrupt Defines
extern volatile uint8_t global_interrupt_flag;  //用于记录当前的总中断标志

#define BSP_ENABLE_INTERRUPT()     {rim();global_interrupt_flag = 1;}
#define BSP_DISABLE_INTERRUPT()     {sim();global_interrupt_flag = 0;}
















































