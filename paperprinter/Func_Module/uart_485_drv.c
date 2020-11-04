/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :uart_485_drv.c 
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


/*------------------------------- Includes ----------------------------------*/
#include "uart_485_drv.h"


/*------------------- Global Definitions and Declarations -------------------*/


/*---------------------- Constant / Macro Definitions -----------------------*/
#define TXBUF_SIZE      1
#define TXBUF_LEN       99
#define TXBUF_FLAG_POS  TXBUF_LEN

#define RXBUF_SIZE      2
#define RXBUF_LEN       99
#define RXBUF_FLAG_POS  RXBUF_LEN


/*----------------------- Type Declarations ---------------------------------*/
enum
{
    E_RS485_IDLE_STA = 0,  //等待接收状态
    E_RS485_TX_STA,        //正在发送状态
    E_RS485_RX_STA,        //正在接收状态
};


/*----------------------- Variable Declarations -----------------------------*/
uint8_t rs485_txbuf[TXBUF_SIZE][TXBUF_LEN + 1] = {0};  //发送缓冲区 1 *100            

uint8_t rs485_rxbuf[RXBUF_SIZE][RXBUF_LEN + 1] = {0};         //接收缓冲区 1 * 100

static uint8_t txbuf_loadcnt = 0;     //发送缓冲区装载数据帧计数
static uint8_t txbuf_sendcnt = 0;     //发送缓冲区发送数据帧计数
static uint8_t tx_datcnt = 0;         //需要发送的数据个数
static uint8_t *tx_datptr = NULL;     //发送数据过程中用来指向要发送的数据

static uint8_t rxbuf_loadcnt = 0;     //接收缓冲区装载数据帧计数
static uint8_t rxbuf_handlecnt = 0;   //接收缓冲区处理数据帧计数
static uint8_t rx_datcnt = 0;         //接收数据个数计数

static volatile uint8_t RS485_status;  //发送状态标志

volatile uint8_t rs485_busfree_timecnt = 0;  //总线空闲计数，同时作为接收超时判断

static rs485_rx_callback_t *rs485_callback_ptr = NULL;

/*----------------------- Function Prototype --------------------------------*/


/*----------------------- Function Implement --------------------------------*/

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
void RS485_Init(void)
{
    uint8_t i;

    //将接收和发送缓冲中的标志清0
    for(i = 0;i < TXBUF_SIZE;i++)
    {
        rs485_txbuf[i][TXBUF_FLAG_POS] = 0x00;
    }

    for(i = 0;i < RXBUF_SIZE;i++)
    {
        rs485_rxbuf[i][RXBUF_FLAG_POS] = 0x00;
    }
    
    rs485_busfree_timecnt = 2;        //设置总线空闲持续时间为10ms ~ 20ms之间

    RS485_status = E_RS485_IDLE_STA;
    RS485_RX_MODE();  //初始化状态为接收模式
}

/*************************************************************
 函数名：
 功  能：在使用查询法发送数据时，连续发送N个数据
 输  入：
 输  出：
 说  明
 作  者：bobde163
 时  间：年  月  日
*************************************************************/
static void RS485_SendBytes(uint8_t *datptr,uint8_t size)
{
    uint8_t i;
    
    for(i = 0;i < size;i++)
    {
        USART_SendData8(USART1 , *datptr++);
        while(USART_GetFlagStatus(USART1 , USART_FLAG_TC) == RESET);
    }
}

/******************************************************************************
* Name     :RS485_SendBytes_it 
*
* Desc     :通过中断的方式发送数据
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/26, Create this function by CenLinbo
 ******************************************************************************/
static void RS485_SendBytes_it(uint8_t *datptr,uint8_t size)
{
    if((NULL == datptr) || (0 == size))
        return ;
    
    tx_datcnt = size - 1;
    tx_datptr = datptr - 1;

    USART_SendData8(USART1 , *datptr);
    
    USART_ITConfig(USART1 , USART_IT_TC , ENABLE);  //使能发送完成中断
}


/******************************************************/
/* 名称：RS485_RxHandle(void)                         */
/* 功能：对接收到的数据进行检验并执行相应处理         */
/* 输入：无                                           */
/* 返回：无                                           */
/* 作者：bobde163                                     */
/* 日期：2016.9.1                                     */
/* 更改说明：在移植时需要根据实际情况添加处理代码     */
/******************************************************/
static void RS485_RxHandle(void)
{
    uint8_t dat_cnt;

    dat_cnt = rs485_rxbuf[rxbuf_handlecnt][RXBUF_FLAG_POS];
    if(dat_cnt > 0)  //有数据需要处理
    {
        //调用上层处理函数
        if(NULL != rs485_callback_ptr)
            rs485_callback_ptr(&rs485_rxbuf[rxbuf_handlecnt][0] , dat_cnt);
        
        rs485_rxbuf[rxbuf_handlecnt][RXBUF_FLAG_POS] = 0;    //数据处理完成，清缓冲区满标志
        
        rxbuf_handlecnt++;                       //循环检测接收缓冲区
        if(rxbuf_handlecnt >= RXBUF_SIZE)
        {
            rxbuf_handlecnt = 0;
        }
    }
}

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
void RS485_RegisterCallBack(rs485_rx_callback_t *func)
{
    rs485_callback_ptr = func;
}

/******************************************************/
/* 名称：RS485_DataToBuf(uint8_t *datptr,uint8_t size)    */
/* 功能：将要发送的数据缓冲到发送缓冲区中             */
/* 输入：datptr:指向发送数据的指针，size：要发送字节数*/
/* 返回：无                                           */
/* 作者：bobde163                                     */
/* 日期：2016.9.1                                     */
/* 更改说明：在移植时需要根据实际协议来填充缓冲区     */
/******************************************************/
uint8_t RS485_DataToBuf(uint8_t *datptr,uint8_t size)
{
    uint8_t i;

    if((NULL == datptr) || (0 == size))
        return 1;
    
    if(0 == rs485_txbuf[txbuf_loadcnt][TXBUF_FLAG_POS])
    {
        //拷贝数据到发送缓冲区
        //超长由只发送最大长度的数据
        if(size > TXBUF_LEN)
            size = TXBUF_LEN;
        
        for(i = 0;i < size;i++)                 //填充数据，顺便计算检验和
        {
            rs485_txbuf[txbuf_loadcnt][i] = *datptr++;
        }
        
        //此时总线持续空闲，则可以开始发送数据
        if(rs485_busfree_timecnt == 0)   
        {
            RS485_TX_MODE();                              //设置485芯片为发送模式
            RS485_status = E_RS485_TX_STA;
             
            RS485_SendBytes_it(&rs485_txbuf[txbuf_loadcnt][0] , size);
        }

        return 0;        //返回0，表示添加成功
    }
    else
    {
        return 1;
    }
}

/******************************************************/
/* 名称：RS485_SendFram(void)                         */
/* 功能：在大循环中调用，查询是否有数据要发送         */
/* 输入：无                                           */
/* 返回：无                                           */
/* 作者：bobde163                                     */
/* 日期：2016.9.1                                     */
/* 更改说明：发送方式根据实现情况更改                 */
/******************************************************/
static void RS485_SendFram(void)
{
    uint8_t dat_cnt;
    
    //此时不在发送数据且总线持续空闲，则可以开始发送新一帧数据
    if((E_RS485_IDLE_STA == RS485_status) && (rs485_busfree_timecnt == 0))
    {
        dat_cnt = rs485_txbuf[txbuf_sendcnt][TXBUF_FLAG_POS];
        if(0 != dat_cnt)  //有数据需要发送
        {
            RS485_TX_MODE();                              //设置485芯片为发送模式
            RS485_status = E_RS485_TX_STA;
             
            RS485_SendBytes_it(&rs485_txbuf[txbuf_loadcnt][0] , dat_cnt);
        }
    }
}

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
void RS485_TimerServer(void)
{
    if(rs485_busfree_timecnt > 0)
    {
        rs485_busfree_timecnt--;
    }
    else
    {
        if(E_RS485_RX_STA == RS485_status)
        {
            //接收超时，结束本帧接收，使用新帧开始接收
            rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] = rx_datcnt;
            
            if(++rxbuf_loadcnt >= RXBUF_SIZE)
                rxbuf_loadcnt = 0;

            rx_datcnt = 0;

            RS485_status = E_RS485_IDLE_STA;
        }
    }
}

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
void RS485_TX_IRQHandler(void)
{
    if(tx_datcnt > 0)
    {
        USART_SendData8(USART1 , *tx_datptr++);
        tx_datcnt--;
    }
    else
    {
        RS485_status = E_RS485_IDLE_STA;
        RS485_RX_MODE();

        *tx_datptr = 0x00; //清缓冲区标志
        
        //关闭中断
        USART_ITConfig(USART1 , USART_IT_TC , DISABLE);
    }
}

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
void RS485_RX_IRQHandler(void)
{
    uint8_t temp;

    temp = USART_ReceiveData8(USART1);

    if(E_RS485_TX_STA != RS485_status)
    {
        RS485_status = E_RS485_RX_STA;
        
        //缓冲区为空才接收数据，否则丢弃
        if(rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] == 0)
        {
            if((rs485_busfree_timecnt > 0) || (0 == rx_datcnt))   //未发生接收超时或者接收到第一个字节
            {
                rs485_busfree_timecnt = 0;  //立即停止超时计数
                
                rs485_rxbuf[rxbuf_loadcnt][rx_datcnt] = temp;
                if(++rx_datcnt >= RXBUF_LEN)
                {
                    //缓冲区不够，提前结束接收
                    rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] = rx_datcnt;
                    rx_datcnt = 0;
                    RS485_status = E_RS485_IDLE_STA;
                    rs485_busfree_timecnt = 0x03;  //重新开始计数总线持续空闲时间
                    
                    if(++rxbuf_loadcnt >= RXBUF_SIZE)
                        rxbuf_loadcnt = 0;
                }
                else
                {
                    rs485_busfree_timecnt = 0x03;  //重新开始计数总线持续空闲时间,同时也作为接收超时的判断依据
                }
            }
            else
            {
                //此路径理论上只存在于定时器中断优先级低于串口中断优先级，且接收中断先于定时器中断发生的情况
                rs485_busfree_timecnt = 0;  //立即停止超时计数

                //接收超时，结束本帧接收，使用新帧开始接收
                rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] = rx_datcnt;
                
                if(++rxbuf_loadcnt >= RXBUF_SIZE)
                    rxbuf_loadcnt = 0;

                rs485_rxbuf[rxbuf_loadcnt][0] = temp;
                rx_datcnt = 1;

                rs485_busfree_timecnt = 0x03;  //重新开始计数总线持续空闲时间,同时也作为接收超时的判断依据
            }
        }
    }
}

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
void RS485_run(void)
{
    RS485_RxHandle();
    RS485_SendFram();
}


/*---------------------------------------------------------------------------*/

