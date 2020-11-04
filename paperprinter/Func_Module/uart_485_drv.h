/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :uart_485_drv.h 
* Desc     :基于单片机提供的UART库实现485通信驱动
* 
* 
* Author   :CenLinbo
* Date     :2020/08/26
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/26, CenLinbo create this file
*         
******************************************************************************/
#ifndef _UART_485_DRV_H_     
#define _UART_485_DRV_H_    


/*------------------------------- Includes ----------------------------------*/
#include "main.h"


/*----------------------------- Global Defines ------------------------------*/
#define RS485_TX_MODE()     GPIO_ResetBits(RS485_CTL_GPIO_Port , RS485_CTL_Pin)  //设置485为发送模式
#define RS485_RX_MODE()     GPIO_SetBits(RS485_CTL_GPIO_Port , RS485_CTL_Pin)  //设置485为接收模式


/*----------------------------- Global Typedefs -----------------------------*/
typedef void rs485_rx_callback_t(uint8_t *datptr , uint8_t size);

/*----------------------------- External Variables --------------------------*/


/*------------------------ Global Function Prototypes -----------------------*/


/******************************************************************************
* Name     :RS485_Init 
*
* Desc     :
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
extern void RS485_Init(void);

/******************************************************************************
* Name     :RS485_RegisterCallBack 
*
* Desc     :注册用于处理接收数据的回调函数，供上层应用调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
extern void RS485_RegisterCallBack(rs485_rx_callback_t *func);

/******************************************************/
/* 名称：RS485_DataToBuf(uint8_t *datptr,uint8_t size)    */
/* 功能：将要发送的数据缓冲到发送缓冲区中             */
/* 输入：datptr:指向发送数据的指针，size：要发送字节数*/
/* 返回：无                                           */
/* 作者：bobde163                                     */
/* 日期：2016.9.1                                     */
/* 更改说明：在移植时需要根据实际协议来填充缓冲区     */
/******************************************************/
extern uint8_t RS485_DataToBuf(uint8_t *datptr,uint8_t size);

/******************************************************************************
* Name     :RS485_TimerServer 
*
* Desc     :RS485通信模块的超时服务函数，实现接收超时判断，总线空闲判断，在1ms中断函数中调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
extern void RS485_TimerServer(void);

/******************************************************************************
* Name     :RS485_TX_IRQHandler 
*
* Desc     :使用中断法发送数据时，需要在发送中断函数中调用实现一帧数据发送
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
extern void RS485_TX_IRQHandler(void);

/******************************************************************************
* Name     :RS485_IRQHandler 
*
* Desc     :在串口接收中断中调用，实现数据接收到缓冲区
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
extern void RS485_RX_IRQHandler(void);

/******************************************************************************
* Name     :RS485_run 
*
* Desc     :RS485通信模块实时运行函数，需要在大循环中调用
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
extern void RS485_run(void);

#endif //_RS485_DRV_H_

