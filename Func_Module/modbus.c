/*****************************************************************
*文件名称：modbus.c 
*功能描述：使用硬件串口实现485通信和modbus协议通信
*适用机型：
*作    者：bobde163
*创建时间：2018年8月22日 V1 创建
*修改时间：
*修改说明：
*****************************************************************/
#include "modbus.h"
#include "sys_info.h"
#include "common_func.h"
#include "CommRF.h"
#include "GUI.h"
#include "lcd_drv.h"
#include "task.h"

#define BUS_IDLE_STA      0  //等待接收状态
#define BUS_TX_STA        1  //正在发送状态
#define BUS_RX_STA        2  //正在接收状态
#define BUS_TX_WAIT_STA   3 //等待发送完成

volatile uint8_t u8a_rs485_txbuf1[RS485_1_TXBUF_SIZE][RS485_1_TXBUF_LEN + 1] = {0};  //1号485接口发送缓冲区
volatile uint8_t u8a_rs485_rxbuf1[RS485_1_TXBUF_SIZE][RS485_1_TXBUF_LEN + 1] = {0};  //1号485接口接收缓冲区
volatile uint8_t u8a_rs485_workbuf1[RS485_1_TXBUF_LEN] = {0};   //用于存储正在发送的数据

volatile uint8_t u8a_rs485_txbuf2[RS485_2_TXBUF_SIZE][RS485_2_TXBUF_LEN + 1] = {0};  //2号485接口发送缓冲区
volatile uint8_t u8a_rs485_rxbuf2[RS485_2_TXBUF_SIZE][RS485_2_TXBUF_LEN + 1] = {0};  //2号485接口接收缓冲区
volatile uint8_t u8a_rs485_workbuf2[RS485_1_TXBUF_LEN] = {0};   //用于存储正在发送的数据

volatile uint8_t u8_txbuf1_loadcnt = 0;  //用于记录当前可装载发送数据的缓冲区编号
volatile uint8_t u8_txbuf1_sendcnt = 0;  //用于记录当前可发送数据的缓冲区编号 
volatile uint8_t u8_rxbuf1_loadcnt = 0;  //用于记录当前可装载接收数据的缓冲区编号 
volatile uint8_t u8_rxbuf1_readcnt = 0;  //用于记录当前可读取处理的接收数据的缓冲区编号 

volatile uint8_t u8_txbuf2_loadcnt = 0;  //用于记录当前可装载发送数据的缓冲区编号
volatile uint8_t u8_txbuf2_sendcnt = 0;  //用于记录当前可发送数据的缓冲区编号 
volatile uint8_t u8_rxbuf2_loadcnt = 0;  //用于记录当前可装载接收数据的缓冲区编号 
volatile uint8_t u8_rxbuf2_readcnt = 0;  //用于记录当前可读取处理的接收数据的缓冲区编号 

volatile uint8_t u8_busfree_timecnt1 = 0;//用于记录总线1的空闲时长，同时也作为接收超时计数
volatile uint8_t u8_busfree_timecnt2 = 0;//用于记录总线2的空闲时长，同时也作为接收超时计数

volatile uint8_t u8_bus1_sta = BUS_IDLE_STA; //用于记录总线1状态，为接收空闲，正在发送或者正在接收这3个状态
volatile uint8_t u8_bus2_sta = BUS_IDLE_STA; //用于记录总线2状态，为接收空闲，正在发送或者正在接收这3个状态

volatile uint8_t u8_tx1_datcnt = 0; //用于记录已送的数据个数
volatile uint8_t u8_rx1_datcnt = 0; //用于记录已接收的数据个数

volatile uint8_t u8_tx2_datcnt = 0; //用于记录已发送的数据个数
volatile uint8_t u8_rx2_datcnt = 0; //用于记录已接收的数据个数

volatile uint8_t u8_tx1_datlen = 0; //用于记录要发送的数据长度
volatile uint8_t u8_tx2_datlen = 0; //用于记录要发送的数据长度

volatile uint8_t gu8_modbus_ch1_addr = 0x01;        //用于暂存通信设备的地址，用于数据接收时的判断
volatile uint8_t gu8_modbus_ch2_addr = 0x01;        //用于暂存通信设备的地址，用于数据接收时的判断

uint8_t gu8_modbus_ch1_baudrate;  //用于保存当前通道1波特率，定期判断是否发生变化，变化时要重新初始化波特率
uint8_t gu8_modbus_ch2_baudrate;  //用于保存当前通道1波特率，定期判断是否发生变化，变化时要重新初始化波特率

uint8_t u8a_work_buf[RS485_1_RXBUF_LEN];  //用作在各处理函数中作临时数据缓冲区

volatile uint8_t u8_modbus_wp_en = 1;  //写保护标志，和于防止随意的修改系统关键参数，上位机必须要发送特定数据关闭保护后，在5秒内可写
volatile uint16_t u16_modbus_wp_timeout; //用于关闭写保护时长计数，计数到0后打开写保护


void vRS485_Init(uint8_t u8_485_id , uint8_t u8_baudrate);
void vRS485_RxHandle(void);
void vRS485_DataToBuf(uint8_t u8_485_id , uint8_t *u8p_datptr , uint8_t u8_len);
void vRS485_Tx1ISR(void);
void vRS485_Tx2ISR(void);
void vRS485_Rx1ISR(void);
void vRS485_Rx2ISR(void);
void vRS485_TimerServer(void);
void vRS485_SendFram(void);

uint16_t u16Modbus_Crc16(uint8_t *u8p_datptr , uint8_t u8_len);
void vModbus_ChProc(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
void vModbus_ReadDiscreteInputProc(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);

uint8_t u8Modbus_GetDiscreteInput(uint8_t *u8p_status , uint8_t *u8p_result , uint16_t u16_start_addr , uint8_t u8_bits);
void vModbus_ReadHoldingReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
void vModbus_WriteSingleHoldingReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
void vModbus_WriteMultipleHoldingReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
void vModbus_ReadInputReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);

uint8_t u8RS485_CheckMakeSetting(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);


//使用不到，注释掉
#if 0
void vModbus_WriteSingleCoil(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
void vModbus_WriteMultipleCoils(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
#endif



/*************************************************************
 *函数名：
 *功  能：初始化指定的485接口为指定的波特率
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_Init(uint8_t u8_485_id , uint8_t u8_baudrate)
{
    if(RS485_1 == u8_485_id)
    {
        RS485_1_RXMODE();     //初始化状态为接收状态

        RC1STAbits.SPEN = 0;  //先禁用串口
        
        //重映射发送和接收中断服务函数
        // disable interrupts before changing states
        PIE3bits.RC1IE = 0;
        EUSART1_SetRxInterruptHandler(vRS485_Rx1ISR);
        PIE3bits.TX1IE = 0;
        EUSART1_SetTxInterruptHandler(vRS485_Tx1ISR);
        
        //根据选择的波特率设置
        BAUD1CON = 0x08;  //BRG16 = 1
        TX1STAbits.BRGH = 1;  //高速晶振模式

        if(RS485_BAUDRATE_1200 == u8_baudrate)
        {
            // SP1BRGL 20; 
            SP1BRGL = 0x14;

            // SP1BRGH 52; 
            SP1BRGH = 0x34;
        }
        
        if(RS485_BAUDRATE_9600 == u8_baudrate)
        {
            // SP1BRGL 130; 
            SP1BRGL = 0x82;

            // SP1BRGH 6; 
            SP1BRGH = 0x06;
        }
        else if(RS485_BAUDRATE_2400 == u8_baudrate)
        {
            // SP1BRGL; 
            SP1BRGL = 0x0A;

            // SP1BRGH; 
            SP1BRGH = 0x1A;
        }
        else if(RS485_BAUDRATE_4800 == u8_baudrate)
        {
            // SP1BRGL; 
            SP1BRGL = 0x04;

            // SP1BRGH; 
            SP1BRGH = 0x0D;
        }
        else if(RS485_BAUDRATE_19200 == u8_baudrate)
        {
            // SP1BRGL; 
            SP1BRGL = 0x40;

            // SP1BRGH; 
            SP1BRGH = 0x03;
        }
        else if(RS485_BAUDRATE_38400 == u8_baudrate)
        {
            // SP1BRGL; 
            SP1BRGL = 0xA0;

            // SP1BRGH; 
            SP1BRGH = 0x01;
        }
        
        // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
        RC1STA = 0x90;

        // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave; 
        TX1STA = 0x24;
        
        RS485_1_RXMODE();   //初始化完成后默认为接收模式
        u8_bus1_sta = BUS_IDLE_STA;
        gu8_modbus_ch1_baudrate = u8_baudrate;
        
        PIE3bits.RC1IE = 1;  //打开接收中断使能
    }
    else if(RS485_2 == u8_485_id)
    {
        RS485_2_RXMODE();     //初始化状态为接收状态

        RC2STAbits.SPEN = 0;  //先禁用串口
        
        //重映射发送和接收中断服务函数
        // disable interrupts before changing states
        PIE3bits.RC2IE = 0;
        EUSART2_SetRxInterruptHandler(vRS485_Rx2ISR);
        PIE3bits.TX2IE = 0;
        EUSART2_SetTxInterruptHandler(vRS485_Tx2ISR);
        
        //根据选择的波特率设置
        BAUD2CON = 0x08;  //BRG16 = 1
        TX2STAbits.BRGH = 1;  //高速晶振模式

        if(RS485_BAUDRATE_1200 == u8_baudrate)
        {
            // SP1BRGL 20; 
            SP2BRGL = 0x14;

            // SP1BRGH 52; 
            SP2BRGH = 0x34;
        }
        else if(RS485_BAUDRATE_9600 == u8_baudrate)
        {
            // SP2BRGL 130; 
            SP2BRGL = 0x82;

            // SP2BRGH 6; 
            SP2BRGH = 0x06;
        }
        else if(RS485_BAUDRATE_2400 == u8_baudrate)
        {
            // SP1BRGL; 
            SP2BRGL = 0x0A;

            // SP1BRGH; 
            SP2BRGH = 0x1A;
        }
        else if(RS485_BAUDRATE_4800 == u8_baudrate)
        {
            // SP2BRGL; 
            SP2BRGL = 0x04;

            // SP2BRGH; 
            SP2BRGH = 0x0D;
        }
        else if(RS485_BAUDRATE_19200 == u8_baudrate)
        {
            // SP2BRGL; 
            SP2BRGL = 0x40;

            // SP2BRGH; 
            SP2BRGH = 0x03;
        }
        else if(RS485_BAUDRATE_38400 == u8_baudrate)
        {
            // SP1BRGL; 
            SP2BRGL = 0xA0;

            // SP1BRGH; 
            SP2BRGH = 0x01;
        }
        
        // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
        RC2STA = 0x90;

        // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave; 
        TX2STA = 0x24;
        
        RS485_2_RXMODE();   //初始化完成后默认为接收模式
        u8_bus2_sta = BUS_IDLE_STA;
        gu8_modbus_ch2_baudrate = u8_baudrate;
        
        PIE3bits.RC2IE = 1;  //打开接收中断使能
    }

    if(RS485_USED_SLAVE == RS485_1)
    {
        gu8_modbus_ch1_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //获取从机地址
    }
    else
    {
        gu8_modbus_ch2_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //获取从机地址
    }
    
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
void vRS485_RxHandle(void)
{
    //总线1
    if(0 != u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN])  //有数据需要处理
    {
        if(RS485_1 == RS485_USED_SLAVE)
        {
            //不启用生产模式或者在启用生产模式下未收到上位机的特殊命令时才会正常处理数据
            if(0 == u8RS485_CheckMakeSetting(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][0] , u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN]))
            {
                vModbus_ChProc(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][0] , u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN]);

                //如果上位机修改了波特率，则要重新初始化2个串口
                if(gu8_modbus_ch1_baudrate != gu8a_sys_config_info[BAUDRATE_POS])
                {
                    gu8_modbus_ch1_baudrate = gu8a_sys_config_info[BAUDRATE_POS];
                    vRS485_Init(RS485_1 , gu8_modbus_ch1_baudrate);
                    vRS485_Init(RS485_2 , gu8_modbus_ch1_baudrate);
                }

                //刷新从机地址
                if(RS485_USED_SLAVE == RS485_1)
                {
                    gu8_modbus_ch1_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //获取从机地址
                }
                else
                {
                    gu8_modbus_ch2_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //获取从机地址
                }
            }
        }
        
        u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN] = 0;  //清缓冲满标志

        //循环切换缓冲区
        if(++u8_rxbuf1_readcnt >= RS485_1_RXBUF_SIZE)
        {
            u8_rxbuf1_readcnt = 0;
        }
    }
    
    //总线2
    if(0 != u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN])  //有数据需要处理
    {
        if(RS485_2 == RS485_USED_SLAVE)
        {
            //不启用生产模式或者在启用生产模式下未收到上位机的特殊命令时才会正常处理数据
            if(0 == u8RS485_CheckMakeSetting(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][0] , u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN]))
            {
                vModbus_ChProc(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][0] , u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN]);
            }
            
            //如果上位机修改了波特率，则要重新初始化2个串口
            if(gu8_modbus_ch2_baudrate != gu8a_sys_config_info[BAUDRATE_POS])
            {
                gu8_modbus_ch2_baudrate = gu8a_sys_config_info[BAUDRATE_POS];
                vRS485_Init(RS485_1 , gu8_modbus_ch1_baudrate);
                vRS485_Init(RS485_2 , gu8_modbus_ch1_baudrate);
            }

            //刷新从机地址
            if(RS485_USED_SLAVE == RS485_1)
            {
                gu8_modbus_ch1_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //获取从机地址
            }
            else
            {
                gu8_modbus_ch2_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //获取从机地址
            }
        }
        
        u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN] = 0;  //清缓冲满标志
    
        //循环切换缓冲区
        if(++u8_rxbuf2_readcnt >= RS485_2_RXBUF_SIZE)
        {
            u8_rxbuf2_readcnt = 0;
        }
    }
    
}


/*************************************************************
 *函数名：
 *功  能：将要发送的数据缓冲到发送缓冲区中 
 *输  入：datptr:指向发送数据的指针，len：要发送字节数
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_DataToBuf(uint8_t u8_485_id , uint8_t *u8p_datptr , uint8_t u8_len)
{
    uint8_t i;
    
    if(RS485_1 == u8_485_id)
    {
        if(u8a_rs485_txbuf1[u8_txbuf1_loadcnt][RS485_1_TXBUF_LEN] == 0)
        {
            //填充发送缓冲区
            for(i = 0;i < u8_len;i++)                 //填充数据，顺便计算检验和
            {
                u8a_rs485_txbuf1[u8_txbuf1_loadcnt][i] = *u8p_datptr++;
            }
            u8a_rs485_txbuf1[u8_txbuf1_loadcnt][RS485_1_TXBUF_LEN] = u8_len;  //缓冲区最后一个字节保存装载的数据长度

            if(++u8_txbuf1_loadcnt >= RS485_1_TXBUF_SIZE)
            {
                u8_txbuf1_loadcnt = 0;
            }
        }
    }
    else if(RS485_2 == u8_485_id)
    {
        if(u8a_rs485_txbuf2[u8_txbuf2_loadcnt][RS485_2_TXBUF_LEN] == 0)
        {
            //填充发送缓冲区
            for(i = 0;i < u8_len;i++)                 //填充数据，顺便计算检验和
            {
                u8a_rs485_txbuf2[u8_txbuf2_loadcnt][i] = *u8p_datptr++;
            }
            u8a_rs485_txbuf2[u8_txbuf2_loadcnt][RS485_2_TXBUF_LEN] = u8_len;  //缓冲区最后一个字节保存装载的数据长度

            if(++u8_txbuf2_loadcnt >= RS485_2_TXBUF_SIZE)
            {
                u8_txbuf2_loadcnt = 0;
            }
        }
    }
}

/*************************************************************
 *函数名：
 *功  能：在大循环中调用，查询是否有数据要发送 
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_SendFram(void)
{
    uint8_t i;
    
    //此时不在发送数据且总线持续空闲，则可以开始发送新一帧数据
    if((BUS_IDLE_STA == u8_bus1_sta) && (u8_busfree_timecnt1 == 0))   
    {
        if(0 != u8a_rs485_txbuf1[u8_txbuf1_sendcnt][RS485_1_TXBUF_LEN])  //有数据需要发送
        {
            RS485_1_TXMODE();  //设置485芯片为发送模式
            u8_bus1_sta = BUS_TX_STA;   //置正在发送标志
            
            //将要发送的数据转移到发送工作缓冲区中
            u8_tx1_datlen = u8a_rs485_txbuf1[u8_txbuf1_sendcnt][RS485_1_TXBUF_LEN];
            for(i = 0;i < u8_tx1_datlen;i++)                           
            {
                u8a_rs485_workbuf1[i] = u8a_rs485_txbuf1[u8_txbuf1_sendcnt][i];
            }
            u8_tx1_datcnt = 0;
            
            //循环使用缓冲区
            u8a_rs485_txbuf1[u8_txbuf1_sendcnt][RS485_1_TXBUF_LEN] = 0; //清缓冲区满标志
            if(++u8_txbuf1_sendcnt >= RS485_1_TXBUF_SIZE)   
            {
                u8_txbuf1_sendcnt = 0;
            }
            
            PIE3bits.TX1IE = 1;        //打开发送中断使能，启动发送
        }
    }
    
    if(BUS_TX_WAIT_STA == u8_bus1_sta)
    {
        //最后一位数据发送出去之后才能算是完成
        if(1 == TX1STAbits.TRMT)
        {
            //数据发送完成
            u8_bus1_sta = BUS_IDLE_STA;   //清正在发送标志位
            u8_busfree_timecnt1 = 2;       //重新计算总线空闲时长，至少10ms
            RS485_1_RXMODE();             //切换到接收模式
        }
    }
    
    //总线2
    if((BUS_IDLE_STA == u8_bus2_sta) && (u8_busfree_timecnt2 == 0))   
    {
        if(0 != u8a_rs485_txbuf2[u8_txbuf2_sendcnt][RS485_2_TXBUF_LEN])  //有数据需要发送
        {
            RS485_2_TXMODE();  //设置485芯片为发送模式
            u8_bus2_sta = BUS_TX_STA;   //置正在发送标志
            
            //将要发送的数据转移到发送工作缓冲区中
            u8_tx2_datlen = u8a_rs485_txbuf2[u8_txbuf2_sendcnt][RS485_2_TXBUF_LEN];
            for(i = 0;i < u8_tx2_datlen;i++)                           
            {
                u8a_rs485_workbuf2[i] = u8a_rs485_txbuf2[u8_txbuf2_sendcnt][i];
            }
            u8_tx2_datcnt = 0;
            
            //循环使用缓冲区
            u8a_rs485_txbuf2[u8_txbuf2_sendcnt][RS485_2_TXBUF_LEN] = 0; //清缓冲区满标志
            if(++u8_txbuf2_sendcnt >= RS485_2_TXBUF_SIZE)   
            {
                u8_txbuf2_sendcnt = 0;
            }
            
            PIE3bits.TX2IE = 1;        //打开发送中断使能，启动发送
        }
    }
    
    if(BUS_TX_WAIT_STA == u8_bus2_sta)
    {
        //最后一位数据发送出去之后才能算是完成
        if(1 == TX2STAbits.TRMT)
        {
            //数据发送完成
            u8_bus2_sta = BUS_IDLE_STA;   //清正在发送标志位
            u8_busfree_timecnt2 = 2;       //重新计算总线空闲时长，至少10ms
            RS485_2_RXMODE();             //切换到接收模式
        }
    }
}

/*************************************************************
 *函数名：
 *功  能：串口1的发送中断服务函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_Tx1ISR(void)
{
    if(BUS_TX_STA == u8_bus1_sta)
    {
        if(u8_tx1_datcnt < u8_tx1_datlen)
        {
            TX1REG = u8a_rs485_workbuf1[u8_tx1_datcnt++];
        }
        else
        {
            PIE3bits.TX1IE = 0;        //关闭发送中断使能
            u8_bus1_sta = BUS_TX_WAIT_STA;
        }
    }
}

/*************************************************************
 *函数名：
 *功  能：串口2的发送中断服务函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_Tx2ISR(void)
{
    if(BUS_TX_STA == u8_bus2_sta)
    {
        if(u8_tx2_datcnt < u8_tx2_datlen)
        {
            TX2REG = u8a_rs485_workbuf2[u8_tx2_datcnt++];
        }
        else
        {
            PIE3bits.TX2IE = 0;        //关闭发送中断使能
            u8_bus2_sta = BUS_TX_WAIT_STA;
        }
    }
}

/*************************************************************
 *函数名：
 *功  能：串口1的接收中断服务函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_Rx1ISR(void)
{
    uint8_t u8_temp = RC1REG;   //无论是否要保存，先读取一次以清除中断标志位

    //清重写错误标志，防止发生数据覆盖时，不清标志位导致无法再接收新数据的问题
    if(1 == RC1STAbits.OERR)
    {
        // EUSART1 error - restart

        RC1STAbits.CREN = 0;
        RC1STAbits.CREN = 1;
    }
    
    if(BUS_TX_STA != u8_bus1_sta)
    {
        u8_bus1_sta = BUS_RX_STA;   //切换为接收状态
        
        //只有缓冲有空的时候才保存数据
        if(0 == u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][RS485_1_RXBUF_LEN])
        {
            u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][u8_rx1_datcnt++] = u8_temp;
            
            if(u8_rx1_datcnt >= RS485_1_RXBUF_LEN)
            {
                //接收到最大数量数据，完成接收
                u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][RS485_1_RXBUF_LEN] = RS485_1_RXBUF_LEN;
                
                //循环切换缓冲区
                if(++u8_rxbuf1_loadcnt >= RS485_1_RXBUF_SIZE)
                {
                    u8_rxbuf1_loadcnt = 0;
                }
                
                //本次接收的数据作为下一帧
                
                u8_rx1_datcnt = 0;            //接收完一帧，清接收数据个数
                u8_bus1_sta = BUS_IDLE_STA;   //接收完一帧，则恢复到待接收状态
            }
        }
        
        u8_busfree_timecnt1 = 2;   //同时用作接收超时判断
    }
}

/*************************************************************
 *函数名：
 *功  能：串口2的接收中断服务函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vRS485_Rx2ISR(void)
{
    uint8_t u8_temp = RC2REG;   //无论是否要保存，先读取一次以清除中断标志位

    //清重写错误标志，防止发生数据覆盖时，不清标志位导致无法再接收新数据的问题
    if(1 == RC2STAbits.OERR)
    {
        // EUSART1 error - restart

        RC2STAbits.CREN = 0;
        RC2STAbits.CREN = 1;
    }
    
    if(BUS_TX_STA != u8_bus2_sta)
    {
        u8_bus2_sta = BUS_RX_STA;   //切换为接收状态
        
        //只有缓冲有空的时候才保存数据
        if(0 == u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][RS485_2_RXBUF_LEN])
        {
            u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][u8_rx2_datcnt++] = u8_temp;
            
            if(u8_rx2_datcnt >= RS485_2_RXBUF_LEN)
            {
                //接收到最大数量数据，完成接收
                u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][RS485_2_RXBUF_LEN] = RS485_2_RXBUF_LEN;
                
                //循环切换缓冲区
                if(++u8_rxbuf2_loadcnt >= RS485_2_RXBUF_SIZE)
                {
                    u8_rxbuf2_loadcnt = 0;
                }
                
                u8_rx2_datcnt = 0;            //接收完一帧，清接收数据个数
                u8_bus2_sta = BUS_IDLE_STA;   //接收完一帧，则恢复到待接收状态
            }
        }
        
        u8_busfree_timecnt2 = 2;   //同时用作接收超时判断
    }
}

/******************************************************/
/* 名称：                       */
/* 功能：RS485接口超时服务函数，在10ms定时器中断函数中调用*/
/* 输入：无                                           */
/* 返回：无                                           */
/* 作者：bobde163                                     */
/* 日期：2018.8.23                                     */
/* 更改说明：       */
/******************************************************/
void vRS485_TimerServer(void)
{
    //总线1空闲计数
    if(u8_busfree_timecnt1 > 0)
    {
        u8_busfree_timecnt1--;
        
        //超过至少10ms未接收到数据，则认为接收超时并且总线空闲
        if(0 == u8_busfree_timecnt1)
        {
            if(BUS_RX_STA == u8_bus1_sta)
            {
                //在接收状态发生超时，需要断帧
                if(u8_rx1_datcnt < 4)
                {
                    //发生接收超时时接收到的数据不够4字节，则抛弃
                    u8_rx1_datcnt = 0;
                }
                else
                {
                    //认为接收到一帧
                    u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][RS485_1_RXBUF_LEN] = u8_rx1_datcnt;
                
                    //循环切换缓冲区
                    if(++u8_rxbuf1_loadcnt >= RS485_1_RXBUF_SIZE)
                    {
                        u8_rxbuf1_loadcnt = 0;
                    }
                    
                    u8_rx1_datcnt = 0;            //接收完一帧，清接收数据个数
                    u8_bus1_sta = BUS_IDLE_STA;   //接收完一帧，则恢复到待接收状态
                }
            }
        }
    }
    
    //总线2空闲计数
    if(u8_busfree_timecnt2 > 0)
    {
        u8_busfree_timecnt2--;
        
        //超过至少10ms未接收到数据，则认为接收超时并且总线空闲
        if(0 == u8_busfree_timecnt2)
        {
            if(BUS_RX_STA == u8_bus2_sta)
            {
                //在接收状态发生超时，需要断帧
                if(u8_rx2_datcnt < 4)
                {
                    //发生接收超时时接收到的数据不够4字节，则抛弃
                    u8_rx2_datcnt = 0;
                }
                else
                {
                    //认为接收到一帧
                    u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][RS485_2_RXBUF_LEN] = u8_rx2_datcnt;
                
                    //循环切换缓冲区
                    if(++u8_rxbuf2_loadcnt >= RS485_2_RXBUF_SIZE)
                    {
                        u8_rxbuf2_loadcnt = 0;
                    }
                    
                    u8_rx2_datcnt = 0;            //接收完一帧，清接收数据个数
                    u8_bus2_sta = BUS_IDLE_STA;   //接收完一帧，则恢复到待接收状态
                }
            }
        }
    }

    //关闭写保护时长计数
    if(u16_modbus_wp_timeout)
    {
        u16_modbus_wp_timeout--;
    }
    else
    {
        u8_modbus_wp_en = 1;  //关闭写保护时间计数结束，重新打开写保护
    }
}

/*************************************************************
*函数名：
*功  能：用于MODBUS-RTU协议的CRC-16检验算法，多项式为0xA001
*输  入：
*输  出：
*说  明：
*作  者：bobde163
*时  间：年  月  日
*************************************************************/
uint16_t u16Modbus_Crc16(uint8_t *u8p_datptr , uint8_t u8_len)
{
    uint8_t i;
    
    uint16_t crc16 = 0xFFFF;
    
    while(u8_len--)
    {
        crc16 ^= *u8p_datptr++;
        for(i = 0 ; i < 8 ; i++)
        {
            if(crc16 & 0x0001)
            {
                crc16 >>= 1;
                crc16 ^= 0xA001;
            }
            else
            {
                crc16 >>= 1;
            }
        }
    }
    
    return crc16;
}

/*************************************************************
 *函数名：
 *功  能：modbus协议层回复通信错误信息
 *输  入：u8_485_id：指定使用的通信接口
 *       u8_addr：节点地址
 *       u8_cmd:命令码
 *       u8_err：错误代码
 *输  出：
 *说  明：
 *************************************************************/
void vModbus_AckErr(uint8_t u8_485_id , uint8_t u8_addr , uint8_t u8_cmd , uint8_t u8_err)
{
    uint8_t u8a_buf[5];
    uint16_t u16_crc16;
    u8a_buf[0] = u8_addr;
    u8a_buf[1] = 0x80u | u8_cmd;
    u8a_buf[2] = u8_err;
    
    u16_crc16 = u16Modbus_Crc16(u8a_buf , 3);  //计算CRC16检验值
    u8a_buf[3] = (uint8_t)u16_crc16;
    u8a_buf[4] = (uint8_t)(u16_crc16 >> 8);
    
    vRS485_DataToBuf(u8_485_id , u8a_buf , 5);  //将数据装载到发送缓冲区中
}

/*************************************************************
 *函数名：
 *功  能：Modbus协议处理函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vModbus_ChProc(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint16_t u16_temp;
    uint8_t u8_addr_temp;
    
    //先计算并判断CRC16校验是否正确
    u16_temp = CHARS_TO_INT16(u8p_dat[u8_len - 1] , u8p_dat[u8_len - 2]);
    if(u16_temp != u16Modbus_Crc16(u8p_dat , u8_len - 2u))
    {
        //校验不通过，直接返回
        return;
    }

    if(RS485_1 == u8_485_id)
    {
        u8_addr_temp = gu8_modbus_ch1_addr;
    }
    else if(RS485_2 == u8_485_id)
    {
        u8_addr_temp = gu8_modbus_ch2_addr;
    }
    else
    {
        //通道号不正确，直接返回
        return;
    }
    
    //判断地址是否正确
    if(u8p_dat[MOD_ADDR_OFFSET] != u8_addr_temp)
    {
        //地址不正确，不处理此帧，直接返回
        return;
    }
    
    //判断命令类型执行对应的操作
    switch(u8p_dat[MOD_CMD_OFFSET])
    {
        //读离散量输入状态值
        case READ_DISCRETE_INPUTS:
        {
            vModbus_ReadDiscreteInputProc(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;
        
        //读取配置数据
        case READ_HOLDING_REG:
        {
            vModbus_ReadHoldingReg(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

		//写单个保持寄存器（系统配置数据）
		case WRITE_SINGLE_REG:
		{
			vModbus_WriteSingleHoldingReg(u8_485_id , u8p_dat , u8_len - 2u);
		}
		break;

        //写多个保持寄存器（系统配置数据）
        case WRITE_MULTIPLE_REG:
        {
            vModbus_WriteMultipleHoldingReg(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

        //读取单个、多个输入寄存器数据（测量数据和软硬件版本）
        case READ_INPUT_REG:
        {
            vModbus_ReadInputReg(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

        //使用不到，注释掉
#if 0
        //写单个线圈（实际只用于修改无线测温相关的报警标志）
        case WRITE_SINGLE_COIL:
        {
            vModbus_WriteSingleCoil(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

        //写多个线圈（实际只用于修改无线测温相关的报警标志）
        case WRITE_MULTIPLE_COILS:
        {
            vModbus_WriteMultipleCoils(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;
#endif

        default:
        {
            //不支持的命令，则返回非法命令错误代码
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        }
        break;
    }
}

/*************************************************************
 *函数名：
 *功  能：读取离散量输入状态操作处理函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
 void vModbus_ReadDiscreteInputProc(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
 {
     uint16_t u16_temp1;
     uint16_t u16_temp2;
     uint8_t u8_addr_temp;

     if(RS485_1 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch1_addr;
     }
     else if(RS485_2 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch2_addr;
     }
     else
     {
         //通道号不正确，直接返回
         return;
     }
     
     //判断命令长度是否符合
     if(6 != u8_len)
     {
        //不符合本命令的固定长度，返回功能错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }
     
     //判断起始地址是否正确
     u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
     if(u16_temp1 < DISCRETE_INPUT_START_ADDR)
     {
         //小于数据起始地址，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
         
         return;
     }
     
     //判断数量是否会超出数据结束地址或者一帧数据容纳不下
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取数量值
     if((u16_temp2 > DISCRETE_INPUT_NUM_MAX) || ((u16_temp1 + u16_temp2) > (DISCRETE_INPUT_END_ADDR + 1)))
     {
         //要读取的数量大于可读取的数量，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }
     
     //起始地址正确，读取数量也正常，则开始读取数据
     u8a_work_buf[0] = u8_addr_temp;
     u8a_work_buf[1] = READ_DISCRETE_INPUTS;
     u8a_work_buf[2] = u8Modbus_GetDiscreteInput((uint8_t *)gx_sys_status_info , &u8a_work_buf[3] , u16_temp1 , (uint8_t)u16_temp2);
     
     u16_temp1 = u16Modbus_Crc16(u8a_work_buf , u8a_work_buf[2] + 3u);  //计算CRC16校验值
     u8a_work_buf[u8a_work_buf[2] + 3u] = (uint8_t)(u16_temp1 & 0xFF);
     u8a_work_buf[u8a_work_buf[2] + 4u] = (uint8_t)(u16_temp1 >> 8);
     
     vRS485_DataToBuf(u8_485_id , u8a_work_buf , u8a_work_buf[2] + 5u);  //发送读取到的数据
 }

//使用不到，注释掉
#if 0  
/*************************************************************
*函数名：
*功  能：使用写线圈命令写离散量输入状态操作处理函数
*输  入：
*输  出：
*说  明：
*************************************************************/
void vModbus_WriteSingleCoil(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint16_t u16_temp1;
    uint8_t u8_pos;
    uint8_t u8_addr_temp;

    if(RS485_1 == u8_485_id)
    {
        u8_addr_temp = gu8_modbus_ch1_addr;
    }
    else if(RS485_2 == u8_485_id)
    {
        u8_addr_temp = gu8_modbus_ch2_addr;
    }
    else
    {
        //通道号不正确，直接返回
        return;
    }

    //判断命令长度是否符合
    if(6 != u8_len)
    {
        //不符合本命令的固定长度，返回功能错误信息
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
    }

    //判断起始地址是否正确，只有可写的地址才会执行
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
    if((u16_temp1 < WP_DISCRETE_INPUT_START_ADDR) || (u16_temp1 > WP_DISCRETE_INPUT_END_ADDR))
    {
        //小于数据起始地址，返回错误信息
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);

        return;
    }

    //处于写保护地址，需要判断是否可写
    if(1 == u8_modbus_wp_en)
    {
        //返回写保护提示
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
        return;
    }

    u8_pos = u16_temp1 - DISCRETE_INPUT_START_ADDR;  //获取偏移地址

    if(0x00 == u8p_dat[MOD_DATA_OFFSET + 3])
    {
        if(0xFF == u8p_dat[MOD_DATA_OFFSET + 2])
        {
            //0xFF00是置1
            gx_sys_status_info[u8_pos / 8u].data |= (1u << (u8_pos % 8u));
        }
        else if(0x00 ==u8p_dat[MOD_DATA_OFFSET + 2])
        {
            //0x0000是清0
            gx_sys_status_info[u8_pos / 8u].data &= ~(1u << (u8_pos % 8u));
        }
        else
        {
            //返回数据错误
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
            return;
        }
    }
    else
    {
        //返回数据错误
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
        return;
    }

    //响应原数据
    vRS485_DataToBuf(u8_485_id , u8p_dat , 8u);
}

 /*************************************************************
 *函数名：
 *功  能：使用写多个线圈命令写离散量输入状态操作处理函数
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
 void vModbus_WriteMultipleCoils(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
 {
     uint16_t u16_temp1;
     uint16_t u16_temp2;
     volatile uint8_t u8_temp;
     volatile uint8_t u8_data_temp;
     uint8_t i;
     uint8_t u8_addr_temp;
 
     if(RS485_1 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch1_addr;
     }
     else if(RS485_2 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch2_addr;
     }
     else
     {
         //通道号不正确，直接返回
         return;
     }
 
     //判断起始地址是否正确，只有可写的地址才会执行
     u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
     if((u16_temp1 < WP_DISCRETE_INPUT_START_ADDR) || (u16_temp1 > WP_DISCRETE_INPUT_END_ADDR))
     {
         //小于数据起始地址，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
 
         return;
     }

     //判断数量是否会超出数据结束地址
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取地址数量值
     if((u16_temp1 + u16_temp2) > (WP_DISCRETE_INPUT_END_ADDR + 1))
     {
         //要读取的数量大于可一次性读取的数量或者导致结束地址超出，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }

     //处于写保护地址，需要判断是否可写
     if(1 == u8_modbus_wp_en)
     {
         //返回写保护提示
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
         return;
     }

    u8_temp = u16_temp1 - DISCRETE_INPUT_START_ADDR;  //获取偏移地址
     if(0 == (u8_temp % 8u))
     {
        u8_temp /= 8u;
     }
     else
     {
        u8_temp = u8_temp / 8u + 1u;
     }

     //判断命令长度是否符合
     if((u8_temp + 7u) != u8_len)
     {
        //不符合本命令的固定长度，返回功能错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }

     //判断字节数是否符合
     if(u8_temp != u8p_dat[MOD_DATA_OFFSET + 4])
     {
         //返回数量数量错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         return;
     }

     u8_temp = u16_temp1 - DISCRETE_INPUT_START_ADDR;
     u8_data_temp = MOD_DATA_OFFSET + 5;
     for(i = 0;i < u16_temp2;i++)
     {
        if(8u == (i % 8u))  //每8位一个字节操作完后，使用下一个字节开始操作
        {
            u8_data_temp++;
        }

        if(0x01 & u8p_dat[u8_data_temp])
        {
            //置1
            gx_sys_status_info[u8_temp / 8u].data |= (1u << (u8_temp % 8u));
        }
        else
        {
            //置0
            gx_sys_status_info[u8_temp / 8u].data &= ~(1u << (u8_temp % 8u));
        }
        u8_temp++;
        u8p_dat[u8_data_temp] >>= 1u;
     }
 
     //响应原数据
     vRS485_DataToBuf(u8_485_id , u8p_dat , 5u);
 }
#endif
 
 /*************************************************************
 *函数名：
 *功  能：从系统状态信息中按输入参数取出离散输入状态数据
 *输  入：
 *输  出：返回实际获取到的字节数
 *说  明：
 *************************************************************/
 uint8_t u8Modbus_GetDiscreteInput(uint8_t *u8p_status , uint8_t *u8p_result , uint16_t u16_start_addr , uint8_t u8_bits)
 {
     uint8_t u8_start_byte; //保存从系统状态信息中哪个字节开始取数据
     uint8_t u8_start_bit;  //保存开始字节中从哪一位开始取数据
     uint8_t u8_dat_cnt = 0; //用来保存获取到的字节数
     
     u8_start_byte = (u16_start_addr - DISCRETE_INPUT_START_ADDR) / 8u;  //计算起始字节地址
     u8_start_bit = (u16_start_addr - DISCRETE_INPUT_START_ADDR) % 8u;  //计算起始字节地址
     
     //起始位置是8的倍数，字节对齐的情况
     if(0 == u8_start_bit)
     {
         while(0 != u8_bits)
         {
             if(u8_bits >= 8u)
             {
                 //以完整的字节进行处理
                 u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte];
                 
                 u8_dat_cnt++;
                 u8_start_byte++;
                 u8_bits -= 8u;
             }
             else
             {
                 //处理最后的几位数据
                 u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte] << (8u - u8_bits);
                 u8p_result[u8_dat_cnt] >>= (8u - u8_bits);
                 
                 u8_dat_cnt++;
                 u8_bits = 0;
             }
         }
     }
     //起始位不是字节对齐的情况
     else
     {
         //要跨字节处理
         while(u8_bits > 8)
         {
             //先处理能组成完整字节的数据
             u8p_result[u8_dat_cnt] = (u8p_status[u8_start_byte] >> u8_start_bit) + (u8p_status[u8_start_byte + 1] << (8u - u8_start_bit));
                 
             u8_dat_cnt++;
             u8_start_byte++;
             u8_bits -= 8u;
         }
         
         //处理余下的不足8位的数据
         if(0 == u8_bits)
         {
             //没有多余的数了，完成
         }
         else if(u8_bits == (8u - u8_start_bit))
         {
             //当前字节余下的bit数正好等于最后要取的bit数
             u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte] >> u8_start_bit;
             u8_dat_cnt++;
         }
         else if(u8_bits > (8u - u8_start_bit))
         {
             //当前字节余下的bit数还不够要取的bit数，只能再跨取下个字节的bit
             u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte + 1u] << (16u - u8_bits - u8_start_bit); //左移，目的是将下一个字节的有效数据位移到最左边，以清空低位
             u8p_result[u8_dat_cnt] >>= (8u - u8_bits); //右移，以清空无效的高位
             u8p_result[u8_dat_cnt] += (u8p_status[u8_start_byte] >> u8_start_bit);
             
             u8_dat_cnt++;
         }
         else
         {
             //当前字节余下的bit数比最后要取的bit数要多，则对本字节进行处理
             u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte] << (8u - u8_start_bit - u8_bits); //左移，目的是清除多余的高位数据
             u8p_result[u8_dat_cnt] >>= (8u - u8_bits);  //右移，目的是只保留要取的数据从最低位开始
             
             u8_dat_cnt++;
         }
     }
     
     return u8_dat_cnt;
 }

  /*************************************************************
 *函数名：
 *功  能：从系统配置信息中读取指定的配置数据
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
 void vModbus_ReadHoldingReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
 {
     uint8_t i;
     uint16_t u16_temp1;
     uint16_t u16_temp2;
     uint8_t u8_addr_temp;
     
     uint8_t *u8p_temp;

     if(RS485_1 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch1_addr;
     }
     else if(RS485_2 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch2_addr;
     }
     else
     {
         //通道号不正确，直接返回
         return;
     }

     //判断命令长度是否符合
     if(6 != u8_len)
     {
        //不符合本命令的固定长度，返回功能错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }
     
     //判断起始地址是否正确
     u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
     if(u16_temp1 < RW_REG_START_ADDR)
     {
         //小于数据起始地址，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
         
         return;
     }
     
     //判断数量是否会超出数据结束地址或者一帧数据容纳不下
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取数量值
     if((u16_temp2 > RW_REG_NUM_MAX) || ((u16_temp1 + u16_temp2) > (RW_REG_END_ADDR + 1)))
     {
         //要读取的数量大于可一次性读取的数量或者导致结束地址超出，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }
     
     //起始地址正确，读取数量也正常，则开始读取并发送数据
     u8a_work_buf[0] = u8_addr_temp;
     u8a_work_buf[1] = READ_HOLDING_REG;
     u8a_work_buf[2] = (uint8_t)u16_temp2 * 2u;  //字节数=寄存器数量x2
     
     i = (uint8_t)u16_temp2;
     u8p_dat = (uint8_t *)&gu8a_sys_config_info[u16_temp1 - RW_REG_START_ADDR];  //获取指向配置区对应起始数据的指针
     u8p_temp = &u8a_work_buf[3];
     for(i = 0;i < (uint8_t)u16_temp2;i++)
     {
         *u8p_temp++ = 0;           //高字节恒为0
         *u8p_temp++ = *u8p_dat++;
     }
     
     u16_temp1 = u16Modbus_Crc16(u8a_work_buf , u8a_work_buf[2] + 3u);  //计算CRC16校验值
     u8a_work_buf[u8a_work_buf[2] + 3u] = (uint8_t)(u16_temp1 & 0xFF);
     u8a_work_buf[u8a_work_buf[2] + 4u] = (uint8_t)(u16_temp1 >> 8);
     
     vRS485_DataToBuf(u8_485_id , u8a_work_buf , u8a_work_buf[2] + 5u);  //发送读取到的数据
 }
 
   /*************************************************************
 *函数名：
 *功  能：写单个寄存器
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vModbus_WriteSingleHoldingReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint16_t u16_temp1;
    uint16_t u16_temp2;
    uint8_t u8_temp , u8_temp1;
    uint8_t u8_addr_temp;

     if(RS485_1 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch1_addr;
     }
     else if(RS485_2 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch2_addr;
     }
     else
     {
         //通道号不正确，直接返回
         return;
     }

     //判断命令长度是否符合
     if(6 != u8_len)
     {
        //不符合本命令的固定长度，返回功能错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }

    //判断起始地址是否正确
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
    if((u16_temp1 < RW_REG_START_ADDR) || (u16_temp1 > RW_REG_END_ADDR))
    {
        //小于数据起始地址，返回错误信息
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);

        return;
    }

    //判断是否是写保护的地址
    if((u16_temp1 >= WP_RW_REG_START_ADDR) && (u16_temp1 <= WP_RW_REG_END_ADDR))
    {
        //处于写保护地址，需要判断是否可写
        if(1 == u8_modbus_wp_en)
        {
            //写保护，返回写保护提示
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
            return;
        }
    }
    
    u8_temp = u16_temp1 - RW_REG_START_ADDR;  //计算得到偏移量

    //判断要写入的值是否在合理范围
    u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取数据值

    //对应要写入的地址不需要进行值范围检查，则需要判断是否小于255
    if(u16_temp1 > RW_REG_VAL_CHECK_END_ADDR)
    {
        if(u16_temp2 > 255u)
        {
            //数据为无效值
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

            return;
        }
    }
    //需要进行规定的值范围检查
    else
    {
        if((u16_temp2 > gu8a_sys_config_val_max[u8_temp]) || (u16_temp2 < gu8a_sys_config_val_min[u8_temp]))
        {
            //数据为无效值
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

            return;
        }
    }

    //如果修改工作模式，则需要判断是否溢出
    if(WORK_MODE_POS == u8_temp)
    {
        if(u16_temp2 >= WORK_MODE_MAX)
        {
            //数据为无效值
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

            return;
        }
    }
    //如果是温湿度值的话，需要检查是否符合上下限逻辑
    else if(0 == u8SYS_CheckTempRhVal((uint8_t)u16_temp2 , u8_temp))
    {
       vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

       return;
    }
    else
    {
        //判断是不是要改频
        u8_temp1 = u8RF_FreqSetCheck(u8_temp , (uint8_t)u16_temp2);
        if(1 == u8_temp1)
        {
            //提示模块电流低，无法改频
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_MODULE_SLEEP);
            return;
        }
        else if(2 == u8_temp1)
        {
            //提示正在扫改频，无法改频
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_FREQ_SWITCHING);
            return;
        }
        else
        {
            //判断是不是改模块日期
            if(0 == u8RF_SetDateCheck(u8_temp , (uint8_t)(u16_temp2)))
            {
                //判断是不是修改ID
                if(0 == u8RF_SetIdCheck(u8_temp , (uint8_t)(u16_temp2)))
                {
                    //如果值和旧值不相等，则更新
                   if(gu8a_sys_config_info[u8_temp] != (uint8_t)u16_temp2)
                   {
                       gu8a_sys_config_info[u8_temp] = (uint8_t)u16_temp2;
                        gu8_sys_config_info_update = 1;  //置系统配置更新标志位
                   }
                }
            }
        }
    }

   //回复主机一样的数据
   vRS485_DataToBuf(u8_485_id , u8p_dat , 8u); 
}
 
   /*************************************************************
 *函数名：
 *功  能：写单个寄存器
 *输  入：
 *输  出：
 *说  明：
 *************************************************************/
void vModbus_WriteMultipleHoldingReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint8_t i;
    uint16_t u16_temp1;
    uint16_t u16_temp2;
    uint16_t u16_temp3;
    uint8_t u8_temp , u8_temp1;
    uint8_t *u8p_temp;
    uint8_t u8_addr_temp;

     if(RS485_1 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch1_addr;
     }
     else if(RS485_2 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch2_addr;
     }
     else
     {
         //通道号不正确，直接返回
         return;
     }

    //判断起始地址是否正确
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
    if(u16_temp1 < RW_REG_START_ADDR)
    {
        //小于数据起始地址，返回错误信息
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);

        return;
    }
    u8_temp = u16_temp1 - RW_REG_START_ADDR;  //计算得到偏移量
    
    //判断数量是否会超出数据结束地址
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取寄存器数量值
     u16_temp2 += u16_temp1;
     u16_temp2 -= 1u; 
     if(u16_temp2 > RW_REG_END_ADDR)
     {
         //要读取的数量大于可一次性读取的数量或者导致结束地址超出，返回错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }

     //判断是否是写保护的地址
    if(((u16_temp1 >= WP_RW_REG_START_ADDR) && (u16_temp1 <= WP_RW_REG_END_ADDR))
        || ((u16_temp2 >= WP_RW_REG_START_ADDR) && (u16_temp2 <= WP_RW_REG_END_ADDR))
        || ((u16_temp1 < WP_RW_REG_START_ADDR) && (u16_temp2 > WP_RW_REG_END_ADDR)))
    {
        //处于写保护地址，需要判断是否可写
        if(1 == u8_modbus_wp_en)
        {
            //写保护，返回写保护提示
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
            return;
        }
    }

    u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取寄存器数量值

     //判断命令长度是否符合
     if(((u16_temp2 * 2u) + 7u) != u8_len)
     {
        //不符合本命令的固定长度，返回功能错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }

     //判断字节数是否等于寄存器数量乘2
     if((u16_temp2 * 2) != u8p_dat[MOD_DATA_OFFSET + 4])
     {
         //返回数量数量错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         return;
     }

    //判断要写入的值是否在合理范围
    //对应要写入的地址不需要进行值范围检查，则需要判断是否小于255
    u8p_temp = u8p_dat + 7;  //指向数据开始的位置
    for(i = 0 ; i < u16_temp2 ; i++)
    {
        u16_temp3 = CHARS_TO_INT16(u8p_temp[0] , u8p_temp[1]); //获取数据值
        u8p_temp += 2;
        
        if(u16_temp1 > RW_REG_VAL_CHECK_END_ADDR)
        {
            if(u16_temp3 > 255u)
            {
                //数据为无效值
                break;
            }

            //如果修改工作模式，则需要判断是否溢出
            if(WORK_MODE_POS == (u8_temp + i))
            {
                if(u16_temp3 >= WORK_MODE_MAX)
                {
                    //数据为无效值
                    break;
                }
            }
        }
        else
        {
            if((u16_temp3 > gu8a_sys_config_val_max[u8_temp + i]) || (u16_temp3 < gu8a_sys_config_val_min[u8_temp + i]))
            {
                //数据为无效值
                break;
            }
        }
    }
    //数据中有的值范围检查不通过，判为无效数据
    if(i < u16_temp2)
    {
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
         
         return;
    }
    //数据都检查通过，更新对应位置
    else
    {
        u8p_temp = u8p_dat + 7;  //指向数据开始的位置

        //先检测其中的温湿度假设更新后是否符合逻辑
        if(0 == u8SYS_CheckTempRhValAll(u8p_temp , u8_temp , (uint8_t)u16_temp2))
        {
            //表示温湿度逻辑检查不通过
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
         
            return;
        }

        //再检测假如需要改频时，能不能改频
        for(i = 0 ; i < u16_temp2 ; i++)
        {
            u16_temp3 = CHARS_TO_INT16(u8p_temp[0] , u8p_temp[1]); //获取数据值
            u8p_temp += 2;

            //判断是不是要改频
            u8_temp1 = u8RF_FreqSetCheck(u8_temp + i , (uint8_t)u16_temp3);
            if(1 == u8_temp1)
            {
                //提示模块电流低，无法改频
                vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_MODULE_SLEEP);
                return;
            }
            else if(2 == u8_temp1)
            {
                //提示正在扫改频，无法改频
                vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_FREQ_SWITCHING);
                return;
            }
        }

        //能执行到此处，表示如果需要更新温湿度上下限值的话是符合上下限逻辑的，而且如果需要改频的话也完成了，
        //也就是说所有待更新的数据都是合法的，可以直接更新
        u8p_temp = u8p_dat + 7;  //指向数据开始的位置
        for(i = 0 ; i < u16_temp2 ; i++)
        {
            u16_temp3 = CHARS_TO_INT16(u8p_temp[0] , u8p_temp[1]); //获取数据值
            u8p_temp += 2;

            //判断是不是改模块日期
            if(0 == u8RF_SetDateCheck(u8_temp + i , (uint8_t)u16_temp3))
            {
                //判断是不是修改ID
                if(0 == u8RF_SetIdCheck(u8_temp + i , (uint8_t)u16_temp3))
                {
                    gu8a_sys_config_info[u8_temp + i] = (uint8_t)u16_temp3;
                }
            }
        }
        
        gu8_sys_config_info_update = 1;  //置系统配置待更新标志位
    }
    //回复主机数据
    u8a_work_buf[0] = u8_addr_temp;
    u8a_work_buf[1] = WRITE_MULTIPLE_REG;
    u8a_work_buf[2] = u8p_dat[2];
    u8a_work_buf[3] = u8p_dat[3];  //起始地址
    u8a_work_buf[4] = u8p_dat[4];
    u8a_work_buf[5] = u8p_dat[5];  //寄存器数量
    
    u16_temp1 = u16Modbus_Crc16(u8a_work_buf , 6);
    u8a_work_buf[6] = (uint8_t)(u16_temp1 & 0xFF);
    u8a_work_buf[7] = (uint8_t)(u16_temp1 >> 8);
    
    vRS485_DataToBuf(u8_485_id , u8a_work_buf , 8u); 
} 

 /*************************************************************
*函数名：
*功  能：读取测量数据和软硬件版本数据
*输  入：u8p_dat：指向命令帧数据的指针
*输  出：
*说  明：
*************************************************************/
void vModbus_ReadInputReg(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint8_t i;
    uint16_t u16_temp1;
    uint16_t u16_temp2;
    uint8_t u8_addr_temp;
    uint8_t *u8p_temp;

     if(RS485_1 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch1_addr;
     }
     else if(RS485_2 == u8_485_id)
     {
         u8_addr_temp = gu8_modbus_ch2_addr;
     }
     else
     {
         //通道号不正确，直接返回
         return;
     }

     //判断命令长度是否符合
     if(6 != u8_len)
     {
        //不符合本命令的固定长度，返回功能错误信息
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }
    
    //判断起始地址是否正确
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //获取起始地址值
    if(u16_temp1 < RO_REG_START_ADDR)
    {
        //小于数据起始地址，返回错误信息
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
        
        return;
    }
    
    //判断数量是否会超出数据结束地址或者一帧数据容纳不下
    u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //获取数量值
    if((u16_temp2 > RW_REG_NUM_MAX) || ((u16_temp1 + u16_temp2) > (RO_REG_END_ADDR + 1)))
    {
        //要读取的数量大于可一次性读取的数量或者导致结束地址超出，返回错误信息
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
        
        return;
    }
    
    //起始地址正确，读取数量也正常，则开始读取并发送数据
    u8a_work_buf[0] = u8_addr_temp;
    u8a_work_buf[1] = READ_INPUT_REG;
    u8a_work_buf[2] = (uint8_t)u16_temp2 * 2u;  //字节数=寄存器数量x2
    
    i = (uint8_t)u16_temp2;
    u8p_dat = (uint8_t *)&gu8a_sys_measure_info[u16_temp1 - RO_REG_START_ADDR];  //获取指向配置区对应起始数据的指针
    u8p_temp = &u8a_work_buf[3];
    for(i = 0;i < (uint8_t)u16_temp2;i++)
    {
        if(((u16_temp1 >= RO_REG_SIGNED_START_ADDR1) && (u16_temp1 <= RO_REG_SIGNED_END_ADDR1))
            || ((u16_temp1 >= RO_REG_SIGNED_START_ADDR2) && (u16_temp1 <= RO_REG_SIGNED_END_ADDR2)))
        {
            if(*u8p_dat > 128)
            {
                //负温度或都负湿度，则最高字节为0xFF
                *u8p_temp++ = 0xFF;
            }
            else
            {
                *u8p_temp++ = 0;           //高字节恒为0
            }

            *u8p_temp++ = *u8p_dat++;
        }
        else
        {
            *u8p_temp++ = 0;           //高字节恒为0

            //要读取无线模块通讯错误次数
            if((u16_temp1 >= RO_REG_RF_COMM_ERR_START_ADDR) && (u16_temp1 <= RO_REG_RF_COMM_ERR_END_ADDR))
            {
                *u8p_temp++ = RF_err_cnt[u16_temp1 - RO_REG_RF_COMM_ERR_START_ADDR];
            }
            else
            {
                *u8p_temp++ = *u8p_dat++;
            }
        }
        u16_temp1 += 1;
    }
    
    u16_temp1 = u16Modbus_Crc16(u8a_work_buf , u8a_work_buf[2] + 3u);  //计算CRC16校验值
    u8a_work_buf[u8a_work_buf[2] + 3u] = (uint8_t)(u16_temp1 & 0xFF);
    u8a_work_buf[u8a_work_buf[2] + 4u] = (uint8_t)(u16_temp1 >> 8);
    
    vRS485_DataToBuf(u8_485_id , u8a_work_buf , u8a_work_buf[2] + 5u);  //发送读取到的数据
}

/*************************************************************
 *函数名：
 *功  能：在RS485处理函数中调用，以判断是否执行特殊的命令
 *输  入：
 *输  出：0：不是特殊命令，1：特殊命令
 *说  明：
 *************************************************************/
uint8_t u8RS485_CheckMakeSetting(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint8_t u8_temp;
    
    if(0 == u8CompareBytes((uint8_t *)"*SETID" , u8p_dat , 6))
    {
        //上位机设置ID号
        if(u8_len >= 30)
        {
            //至少要是30字节才是完整的命令帧
            SetWLTP_ID_Beg = 1;
            Memory_Copy(u8p_dat , gu8a_rf_cmd_buf , 30);

            Conect_STA = 1;//立即触发通讯
            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*REDID" , u8p_dat , 6))
    {
        //上位机设置ID号
        if(u8_len >= 18)
        {
            //至少要是18字节才是完整的命令帧
            ReadWLTP_ID_Beg = 1;
            Memory_Copy(u8p_dat , gu8a_rf_cmd_buf , 18);

            Conect_STA = 1;//立即触发通讯
            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*RDLCDSTA" , u8p_dat , 9))
    {
        //读取液晶屏状态值，用于调试LCD死机问题
        if(9 <= u8_len)
        {
            //至少要是9字节才是完整的命令帧
            Memory_Copy("*LCDSTA:" , gu8a_rf_cmd_buf , 8);

            u8_temp = u8LCD_ReadStatus();  //获取LCD状态值
            u8Conv_ByteToBitString(&u8_temp , &gu8a_rf_cmd_buf[8] , 1); //转换成二进制字符串
            vRS485_DataToBuf(u8_485_id , gu8a_rf_cmd_buf , 16);  //发送给上位机

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*RSTLCD" , u8p_dat , 7))
    {
        //复位LCD，用于调试LCD死机问题
        if(7 <= u8_len)
        {
            //至少要是7字节才是完整的命令帧
            vGUI_Init();
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD背光持续打开，若无按键操作，则60秒后关闭
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*RST_LCD_OK" , 11);  //发送给上位机

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETLCDDISPALL" , u8p_dat , 14))
    {
        //显示所有像素点，用于测试LCD
        if(14 <= u8_len)
        {
            //至少要是14字节才是完整的命令帧
            gu8_gui_test_lcd_flag = 1;   //测试状态
            vLCD_DispAllWithRam();
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD背光持续打开，若无按键操作，则60秒后关闭
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*DISP_LCD_OK" , 12);  //发送给上位机

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETLCDCLRALL" , u8p_dat , 13))
    {
        //不显示所有像素点，用于测试LCD
        if(13 <= u8_len)
        {
            //至少要是13字节才是完整的命令帧
            gu8_gui_test_lcd_flag = 1;   //测试状态
            vLCD_ClearAllWithRam();
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD背光持续打开，若无按键操作，则60秒后关闭
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLR_LCD_OK" , 11);  //发送给上位机

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*RESTORELCDALL" , u8p_dat , 14))
    {
        //恢复正常显示，用于测试LCD
        if(14 <= u8_len)
        {
            //至少要是13字节才是完整的命令帧
            gu8_gui_test_lcd_flag = 0;   //立即恢复正常状态
            vGUI_RestorDisp();
            RESTART_LCD_FRESH();         //界面发生切换，重置刷屏时间
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD背光持续打开，若无按键操作，则60秒后关闭
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*RESTORE_LCD_OK" , 15);  //发送给上位机

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*REQWR" , u8p_dat , 6))
    {
        //请求写保护寄存器
        if(6 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            u8_modbus_wp_en = 0;
            u16_modbus_wp_timeout = 500;  //重置计时器

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*REQOK" , 6);  //发送给上位机同意请求写保护寄存器

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETLAOHUA" , u8p_dat , 10))
    {
        //设置老化标志
        if(10 <= u8_len)
        {
            //至少要是10字节才是完整的命令帧
            LaoHua_sta = 1;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRLAOHUA" , u8p_dat , 10))
    {
        //清除老化标志
        if(10 <= u8_len)
        {
            //至少要是10字节才是完整的命令帧
            LaoHua_sta = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*REDLAOHUA" , u8p_dat , 10))
    {
        //读取老化标志
        if(10 <= u8_len)
        {
            //至少要是10字节才是完整的命令帧
            if(0 == LaoHua_sta)
            {
                vRS485_DataToBuf(u8_485_id , (uint8_t *)"*REDOK:CLR" , 10);
            }
            else
            {
                vRS485_DataToBuf(u8_485_id , (uint8_t *)"*REDOK:SET" , 10);
            }

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRCOMMALARM" , u8p_dat , 13))
    {
        //清通信失败报警标志
        if(13 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            AlarmRF_CommFailFlag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRABSALARM" , u8p_dat , 12))
    {
        //清超温报警标志
        if(12 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            AlarmAbsFlag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRRELALARM" , u8p_dat , 12))
    {
        //清温升报警标志
        if(12 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            AlarmRelFlag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRBALANCEALARM" , u8p_dat , 16))
    {
        //清相间温差报警标志
        if(16 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            Alarm_Blance_Flag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETCOMMALARM" , u8p_dat , 13))
    {
        //设置通信失败报警标志
        if(13 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            AlarmRF_CommFailFlag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETABSALARM" , u8p_dat , 12))
    {
        //设置超温报警标志
        if(12 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            AlarmAbsFlag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETRELALARM" , u8p_dat , 12))
    {
        //设置温升报警标志
        if(12 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            AlarmRelFlag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETBALANCEALARM" , u8p_dat , 16))
    {
        //设置相间温差报警标志
        if(16 <= u8_len)
        {
            //至少要是6字节才是完整的命令帧
            Alarm_Blance_Flag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //设置30秒内不执行无线报警标志更新操作

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*OPENFAN" , u8p_dat , 8))
    {
        //强制打开风扇
        gu8_test_fan_flag = 1;
        FAN = 0;                //打开风扇

        vRS485_DataToBuf(u8_485_id , (uint8_t *)"*OPENOK" , 7);
        
        return 1;
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLOSEFAN" , u8p_dat , 9))
    {
        //强制关闭风扇
        gu8_test_fan_flag = 0;
        FAN = 1;                //关闭风扇
        vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLOSEOK" , 8);

        return 1;
    }

    return 0;
}


