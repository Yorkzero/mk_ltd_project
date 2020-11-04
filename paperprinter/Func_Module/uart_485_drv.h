/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :uart_485_drv.h 
* Desc     :���ڵ�Ƭ���ṩ��UART��ʵ��485ͨ������
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
#define RS485_TX_MODE()     GPIO_ResetBits(RS485_CTL_GPIO_Port , RS485_CTL_Pin)  //����485Ϊ����ģʽ
#define RS485_RX_MODE()     GPIO_SetBits(RS485_CTL_GPIO_Port , RS485_CTL_Pin)  //����485Ϊ����ģʽ


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
* Desc     :ע�����ڴ���������ݵĻص����������ϲ�Ӧ�õ���
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
/* ���ƣ�RS485_DataToBuf(uint8_t *datptr,uint8_t size)    */
/* ���ܣ���Ҫ���͵����ݻ��嵽���ͻ�������             */
/* ���룺datptr:ָ�������ݵ�ָ�룬size��Ҫ�����ֽ���*/
/* ���أ���                                           */
/* ���ߣ�bobde163                                     */
/* ���ڣ�2016.9.1                                     */
/* ����˵��������ֲʱ��Ҫ����ʵ��Э������仺����     */
/******************************************************/
extern uint8_t RS485_DataToBuf(uint8_t *datptr,uint8_t size);

/******************************************************************************
* Name     :RS485_TimerServer 
*
* Desc     :RS485ͨ��ģ��ĳ�ʱ��������ʵ�ֽ��ճ�ʱ�жϣ����߿����жϣ���1ms�жϺ����е���
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
* Desc     :ʹ���жϷ���������ʱ����Ҫ�ڷ����жϺ����е���ʵ��һ֡���ݷ���
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
* Desc     :�ڴ��ڽ����ж��е��ã�ʵ�����ݽ��յ�������
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
* Desc     :RS485ͨ��ģ��ʵʱ���к�������Ҫ�ڴ�ѭ���е���
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

