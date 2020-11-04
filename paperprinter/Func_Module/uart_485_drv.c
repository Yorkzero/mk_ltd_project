/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :uart_485_drv.c 
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
    E_RS485_IDLE_STA = 0,  //�ȴ�����״̬
    E_RS485_TX_STA,        //���ڷ���״̬
    E_RS485_RX_STA,        //���ڽ���״̬
};


/*----------------------- Variable Declarations -----------------------------*/
uint8_t rs485_txbuf[TXBUF_SIZE][TXBUF_LEN + 1] = {0};  //���ͻ����� 1 *100            

uint8_t rs485_rxbuf[RXBUF_SIZE][RXBUF_LEN + 1] = {0};         //���ջ����� 1 * 100

static uint8_t txbuf_loadcnt = 0;     //���ͻ�����װ������֡����
static uint8_t txbuf_sendcnt = 0;     //���ͻ�������������֡����
static uint8_t tx_datcnt = 0;         //��Ҫ���͵����ݸ���
static uint8_t *tx_datptr = NULL;     //�������ݹ���������ָ��Ҫ���͵�����

static uint8_t rxbuf_loadcnt = 0;     //���ջ�����װ������֡����
static uint8_t rxbuf_handlecnt = 0;   //���ջ�������������֡����
static uint8_t rx_datcnt = 0;         //�������ݸ�������

static volatile uint8_t RS485_status;  //����״̬��־

volatile uint8_t rs485_busfree_timecnt = 0;  //���߿��м�����ͬʱ��Ϊ���ճ�ʱ�ж�

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

    //�����պͷ��ͻ����еı�־��0
    for(i = 0;i < TXBUF_SIZE;i++)
    {
        rs485_txbuf[i][TXBUF_FLAG_POS] = 0x00;
    }

    for(i = 0;i < RXBUF_SIZE;i++)
    {
        rs485_rxbuf[i][RXBUF_FLAG_POS] = 0x00;
    }
    
    rs485_busfree_timecnt = 2;        //�������߿��г���ʱ��Ϊ10ms ~ 20ms֮��

    RS485_status = E_RS485_IDLE_STA;
    RS485_RX_MODE();  //��ʼ��״̬Ϊ����ģʽ
}

/*************************************************************
 ��������
 ��  �ܣ���ʹ�ò�ѯ����������ʱ����������N������
 ��  �룺
 ��  ����
 ˵  ��
 ��  �ߣ�bobde163
 ʱ  �䣺��  ��  ��
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
* Desc     :ͨ���жϵķ�ʽ��������
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
    
    USART_ITConfig(USART1 , USART_IT_TC , ENABLE);  //ʹ�ܷ�������ж�
}


/******************************************************/
/* ���ƣ�RS485_RxHandle(void)                         */
/* ���ܣ��Խ��յ������ݽ��м��鲢ִ����Ӧ����         */
/* ���룺��                                           */
/* ���أ���                                           */
/* ���ߣ�bobde163                                     */
/* ���ڣ�2016.9.1                                     */
/* ����˵��������ֲʱ��Ҫ����ʵ�������Ӵ������     */
/******************************************************/
static void RS485_RxHandle(void)
{
    uint8_t dat_cnt;

    dat_cnt = rs485_rxbuf[rxbuf_handlecnt][RXBUF_FLAG_POS];
    if(dat_cnt > 0)  //��������Ҫ����
    {
        //�����ϲ㴦����
        if(NULL != rs485_callback_ptr)
            rs485_callback_ptr(&rs485_rxbuf[rxbuf_handlecnt][0] , dat_cnt);
        
        rs485_rxbuf[rxbuf_handlecnt][RXBUF_FLAG_POS] = 0;    //���ݴ�����ɣ��建��������־
        
        rxbuf_handlecnt++;                       //ѭ�������ջ�����
        if(rxbuf_handlecnt >= RXBUF_SIZE)
        {
            rxbuf_handlecnt = 0;
        }
    }
}

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
void RS485_RegisterCallBack(rs485_rx_callback_t *func)
{
    rs485_callback_ptr = func;
}

/******************************************************/
/* ���ƣ�RS485_DataToBuf(uint8_t *datptr,uint8_t size)    */
/* ���ܣ���Ҫ���͵����ݻ��嵽���ͻ�������             */
/* ���룺datptr:ָ�������ݵ�ָ�룬size��Ҫ�����ֽ���*/
/* ���أ���                                           */
/* ���ߣ�bobde163                                     */
/* ���ڣ�2016.9.1                                     */
/* ����˵��������ֲʱ��Ҫ����ʵ��Э������仺����     */
/******************************************************/
uint8_t RS485_DataToBuf(uint8_t *datptr,uint8_t size)
{
    uint8_t i;

    if((NULL == datptr) || (0 == size))
        return 1;
    
    if(0 == rs485_txbuf[txbuf_loadcnt][TXBUF_FLAG_POS])
    {
        //�������ݵ����ͻ�����
        //������ֻ������󳤶ȵ�����
        if(size > TXBUF_LEN)
            size = TXBUF_LEN;
        
        for(i = 0;i < size;i++)                 //������ݣ�˳���������
        {
            rs485_txbuf[txbuf_loadcnt][i] = *datptr++;
        }
        
        //��ʱ���߳������У�����Կ�ʼ��������
        if(rs485_busfree_timecnt == 0)   
        {
            RS485_TX_MODE();                              //����485оƬΪ����ģʽ
            RS485_status = E_RS485_TX_STA;
             
            RS485_SendBytes_it(&rs485_txbuf[txbuf_loadcnt][0] , size);
        }

        return 0;        //����0����ʾ��ӳɹ�
    }
    else
    {
        return 1;
    }
}

/******************************************************/
/* ���ƣ�RS485_SendFram(void)                         */
/* ���ܣ��ڴ�ѭ���е��ã���ѯ�Ƿ�������Ҫ����         */
/* ���룺��                                           */
/* ���أ���                                           */
/* ���ߣ�bobde163                                     */
/* ���ڣ�2016.9.1                                     */
/* ����˵�������ͷ�ʽ����ʵ���������                 */
/******************************************************/
static void RS485_SendFram(void)
{
    uint8_t dat_cnt;
    
    //��ʱ���ڷ������������߳������У�����Կ�ʼ������һ֡����
    if((E_RS485_IDLE_STA == RS485_status) && (rs485_busfree_timecnt == 0))
    {
        dat_cnt = rs485_txbuf[txbuf_sendcnt][TXBUF_FLAG_POS];
        if(0 != dat_cnt)  //��������Ҫ����
        {
            RS485_TX_MODE();                              //����485оƬΪ����ģʽ
            RS485_status = E_RS485_TX_STA;
             
            RS485_SendBytes_it(&rs485_txbuf[txbuf_loadcnt][0] , dat_cnt);
        }
    }
}

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
            //���ճ�ʱ��������֡���գ�ʹ����֡��ʼ����
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

        *tx_datptr = 0x00; //�建������־
        
        //�ر��ж�
        USART_ITConfig(USART1 , USART_IT_TC , DISABLE);
    }
}

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
void RS485_RX_IRQHandler(void)
{
    uint8_t temp;

    temp = USART_ReceiveData8(USART1);

    if(E_RS485_TX_STA != RS485_status)
    {
        RS485_status = E_RS485_RX_STA;
        
        //������Ϊ�ղŽ������ݣ�������
        if(rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] == 0)
        {
            if((rs485_busfree_timecnt > 0) || (0 == rx_datcnt))   //δ�������ճ�ʱ���߽��յ���һ���ֽ�
            {
                rs485_busfree_timecnt = 0;  //����ֹͣ��ʱ����
                
                rs485_rxbuf[rxbuf_loadcnt][rx_datcnt] = temp;
                if(++rx_datcnt >= RXBUF_LEN)
                {
                    //��������������ǰ��������
                    rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] = rx_datcnt;
                    rx_datcnt = 0;
                    RS485_status = E_RS485_IDLE_STA;
                    rs485_busfree_timecnt = 0x03;  //���¿�ʼ�������߳�������ʱ��
                    
                    if(++rxbuf_loadcnt >= RXBUF_SIZE)
                        rxbuf_loadcnt = 0;
                }
                else
                {
                    rs485_busfree_timecnt = 0x03;  //���¿�ʼ�������߳�������ʱ��,ͬʱҲ��Ϊ���ճ�ʱ���ж�����
                }
            }
            else
            {
                //��·��������ֻ�����ڶ�ʱ���ж����ȼ����ڴ����ж����ȼ����ҽ����ж����ڶ�ʱ���жϷ��������
                rs485_busfree_timecnt = 0;  //����ֹͣ��ʱ����

                //���ճ�ʱ��������֡���գ�ʹ����֡��ʼ����
                rs485_rxbuf[rxbuf_loadcnt][RXBUF_FLAG_POS] = rx_datcnt;
                
                if(++rxbuf_loadcnt >= RXBUF_SIZE)
                    rxbuf_loadcnt = 0;

                rs485_rxbuf[rxbuf_loadcnt][0] = temp;
                rx_datcnt = 1;

                rs485_busfree_timecnt = 0x03;  //���¿�ʼ�������߳�������ʱ��,ͬʱҲ��Ϊ���ճ�ʱ���ж�����
            }
        }
    }
}

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
void RS485_run(void)
{
    RS485_RxHandle();
    RS485_SendFram();
}


/*---------------------------------------------------------------------------*/

