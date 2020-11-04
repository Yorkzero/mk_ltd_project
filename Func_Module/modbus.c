/*****************************************************************
*�ļ����ƣ�modbus.c 
*����������ʹ��Ӳ������ʵ��485ͨ�ź�modbusЭ��ͨ��
*���û��ͣ�
*��    �ߣ�bobde163
*����ʱ�䣺2018��8��22�� V1 ����
*�޸�ʱ�䣺
*�޸�˵����
*****************************************************************/
#include "modbus.h"
#include "sys_info.h"
#include "common_func.h"
#include "CommRF.h"
#include "GUI.h"
#include "lcd_drv.h"
#include "task.h"

#define BUS_IDLE_STA      0  //�ȴ�����״̬
#define BUS_TX_STA        1  //���ڷ���״̬
#define BUS_RX_STA        2  //���ڽ���״̬
#define BUS_TX_WAIT_STA   3 //�ȴ��������

volatile uint8_t u8a_rs485_txbuf1[RS485_1_TXBUF_SIZE][RS485_1_TXBUF_LEN + 1] = {0};  //1��485�ӿڷ��ͻ�����
volatile uint8_t u8a_rs485_rxbuf1[RS485_1_TXBUF_SIZE][RS485_1_TXBUF_LEN + 1] = {0};  //1��485�ӿڽ��ջ�����
volatile uint8_t u8a_rs485_workbuf1[RS485_1_TXBUF_LEN] = {0};   //���ڴ洢���ڷ��͵�����

volatile uint8_t u8a_rs485_txbuf2[RS485_2_TXBUF_SIZE][RS485_2_TXBUF_LEN + 1] = {0};  //2��485�ӿڷ��ͻ�����
volatile uint8_t u8a_rs485_rxbuf2[RS485_2_TXBUF_SIZE][RS485_2_TXBUF_LEN + 1] = {0};  //2��485�ӿڽ��ջ�����
volatile uint8_t u8a_rs485_workbuf2[RS485_1_TXBUF_LEN] = {0};   //���ڴ洢���ڷ��͵�����

volatile uint8_t u8_txbuf1_loadcnt = 0;  //���ڼ�¼��ǰ��װ�ط������ݵĻ��������
volatile uint8_t u8_txbuf1_sendcnt = 0;  //���ڼ�¼��ǰ�ɷ������ݵĻ�������� 
volatile uint8_t u8_rxbuf1_loadcnt = 0;  //���ڼ�¼��ǰ��װ�ؽ������ݵĻ�������� 
volatile uint8_t u8_rxbuf1_readcnt = 0;  //���ڼ�¼��ǰ�ɶ�ȡ����Ľ������ݵĻ�������� 

volatile uint8_t u8_txbuf2_loadcnt = 0;  //���ڼ�¼��ǰ��װ�ط������ݵĻ��������
volatile uint8_t u8_txbuf2_sendcnt = 0;  //���ڼ�¼��ǰ�ɷ������ݵĻ�������� 
volatile uint8_t u8_rxbuf2_loadcnt = 0;  //���ڼ�¼��ǰ��װ�ؽ������ݵĻ�������� 
volatile uint8_t u8_rxbuf2_readcnt = 0;  //���ڼ�¼��ǰ�ɶ�ȡ����Ľ������ݵĻ�������� 

volatile uint8_t u8_busfree_timecnt1 = 0;//���ڼ�¼����1�Ŀ���ʱ����ͬʱҲ��Ϊ���ճ�ʱ����
volatile uint8_t u8_busfree_timecnt2 = 0;//���ڼ�¼����2�Ŀ���ʱ����ͬʱҲ��Ϊ���ճ�ʱ����

volatile uint8_t u8_bus1_sta = BUS_IDLE_STA; //���ڼ�¼����1״̬��Ϊ���տ��У����ڷ��ͻ������ڽ�����3��״̬
volatile uint8_t u8_bus2_sta = BUS_IDLE_STA; //���ڼ�¼����2״̬��Ϊ���տ��У����ڷ��ͻ������ڽ�����3��״̬

volatile uint8_t u8_tx1_datcnt = 0; //���ڼ�¼���͵����ݸ���
volatile uint8_t u8_rx1_datcnt = 0; //���ڼ�¼�ѽ��յ����ݸ���

volatile uint8_t u8_tx2_datcnt = 0; //���ڼ�¼�ѷ��͵����ݸ���
volatile uint8_t u8_rx2_datcnt = 0; //���ڼ�¼�ѽ��յ����ݸ���

volatile uint8_t u8_tx1_datlen = 0; //���ڼ�¼Ҫ���͵����ݳ���
volatile uint8_t u8_tx2_datlen = 0; //���ڼ�¼Ҫ���͵����ݳ���

volatile uint8_t gu8_modbus_ch1_addr = 0x01;        //�����ݴ�ͨ���豸�ĵ�ַ���������ݽ���ʱ���ж�
volatile uint8_t gu8_modbus_ch2_addr = 0x01;        //�����ݴ�ͨ���豸�ĵ�ַ���������ݽ���ʱ���ж�

uint8_t gu8_modbus_ch1_baudrate;  //���ڱ��浱ǰͨ��1�����ʣ������ж��Ƿ����仯���仯ʱҪ���³�ʼ��������
uint8_t gu8_modbus_ch2_baudrate;  //���ڱ��浱ǰͨ��1�����ʣ������ж��Ƿ����仯���仯ʱҪ���³�ʼ��������

uint8_t u8a_work_buf[RS485_1_RXBUF_LEN];  //�����ڸ�������������ʱ���ݻ�����

volatile uint8_t u8_modbus_wp_en = 1;  //д������־�����ڷ�ֹ������޸�ϵͳ�ؼ���������λ������Ҫ�����ض����ݹرձ�������5���ڿ�д
volatile uint16_t u16_modbus_wp_timeout; //���ڹر�д����ʱ��������������0���д����


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


//ʹ�ò�����ע�͵�
#if 0
void vModbus_WriteSingleCoil(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
void vModbus_WriteMultipleCoils(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len);
#endif



/*************************************************************
 *��������
 *��  �ܣ���ʼ��ָ����485�ӿ�Ϊָ���Ĳ�����
 *��  �룺
 *��  ����
 *˵  ����
 *************************************************************/
void vRS485_Init(uint8_t u8_485_id , uint8_t u8_baudrate)
{
    if(RS485_1 == u8_485_id)
    {
        RS485_1_RXMODE();     //��ʼ��״̬Ϊ����״̬

        RC1STAbits.SPEN = 0;  //�Ƚ��ô���
        
        //��ӳ�䷢�ͺͽ����жϷ�����
        // disable interrupts before changing states
        PIE3bits.RC1IE = 0;
        EUSART1_SetRxInterruptHandler(vRS485_Rx1ISR);
        PIE3bits.TX1IE = 0;
        EUSART1_SetTxInterruptHandler(vRS485_Tx1ISR);
        
        //����ѡ��Ĳ���������
        BAUD1CON = 0x08;  //BRG16 = 1
        TX1STAbits.BRGH = 1;  //���پ���ģʽ

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
        
        RS485_1_RXMODE();   //��ʼ����ɺ�Ĭ��Ϊ����ģʽ
        u8_bus1_sta = BUS_IDLE_STA;
        gu8_modbus_ch1_baudrate = u8_baudrate;
        
        PIE3bits.RC1IE = 1;  //�򿪽����ж�ʹ��
    }
    else if(RS485_2 == u8_485_id)
    {
        RS485_2_RXMODE();     //��ʼ��״̬Ϊ����״̬

        RC2STAbits.SPEN = 0;  //�Ƚ��ô���
        
        //��ӳ�䷢�ͺͽ����жϷ�����
        // disable interrupts before changing states
        PIE3bits.RC2IE = 0;
        EUSART2_SetRxInterruptHandler(vRS485_Rx2ISR);
        PIE3bits.TX2IE = 0;
        EUSART2_SetTxInterruptHandler(vRS485_Tx2ISR);
        
        //����ѡ��Ĳ���������
        BAUD2CON = 0x08;  //BRG16 = 1
        TX2STAbits.BRGH = 1;  //���پ���ģʽ

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
        
        RS485_2_RXMODE();   //��ʼ����ɺ�Ĭ��Ϊ����ģʽ
        u8_bus2_sta = BUS_IDLE_STA;
        gu8_modbus_ch2_baudrate = u8_baudrate;
        
        PIE3bits.RC2IE = 1;  //�򿪽����ж�ʹ��
    }

    if(RS485_USED_SLAVE == RS485_1)
    {
        gu8_modbus_ch1_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //��ȡ�ӻ���ַ
    }
    else
    {
        gu8_modbus_ch2_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //��ȡ�ӻ���ַ
    }
    
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
void vRS485_RxHandle(void)
{
    //����1
    if(0 != u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN])  //��������Ҫ����
    {
        if(RS485_1 == RS485_USED_SLAVE)
        {
            //����������ģʽ��������������ģʽ��δ�յ���λ������������ʱ�Ż�������������
            if(0 == u8RS485_CheckMakeSetting(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][0] , u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN]))
            {
                vModbus_ChProc(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][0] , u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN]);

                //�����λ���޸��˲����ʣ���Ҫ���³�ʼ��2������
                if(gu8_modbus_ch1_baudrate != gu8a_sys_config_info[BAUDRATE_POS])
                {
                    gu8_modbus_ch1_baudrate = gu8a_sys_config_info[BAUDRATE_POS];
                    vRS485_Init(RS485_1 , gu8_modbus_ch1_baudrate);
                    vRS485_Init(RS485_2 , gu8_modbus_ch1_baudrate);
                }

                //ˢ�´ӻ���ַ
                if(RS485_USED_SLAVE == RS485_1)
                {
                    gu8_modbus_ch1_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //��ȡ�ӻ���ַ
                }
                else
                {
                    gu8_modbus_ch2_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //��ȡ�ӻ���ַ
                }
            }
        }
        
        u8a_rs485_rxbuf1[u8_rxbuf1_readcnt][RS485_1_RXBUF_LEN] = 0;  //�建������־

        //ѭ���л�������
        if(++u8_rxbuf1_readcnt >= RS485_1_RXBUF_SIZE)
        {
            u8_rxbuf1_readcnt = 0;
        }
    }
    
    //����2
    if(0 != u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN])  //��������Ҫ����
    {
        if(RS485_2 == RS485_USED_SLAVE)
        {
            //����������ģʽ��������������ģʽ��δ�յ���λ������������ʱ�Ż�������������
            if(0 == u8RS485_CheckMakeSetting(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][0] , u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN]))
            {
                vModbus_ChProc(RS485_USED_SLAVE , (uint8_t *)&u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][0] , u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN]);
            }
            
            //�����λ���޸��˲����ʣ���Ҫ���³�ʼ��2������
            if(gu8_modbus_ch2_baudrate != gu8a_sys_config_info[BAUDRATE_POS])
            {
                gu8_modbus_ch2_baudrate = gu8a_sys_config_info[BAUDRATE_POS];
                vRS485_Init(RS485_1 , gu8_modbus_ch1_baudrate);
                vRS485_Init(RS485_2 , gu8_modbus_ch1_baudrate);
            }

            //ˢ�´ӻ���ַ
            if(RS485_USED_SLAVE == RS485_1)
            {
                gu8_modbus_ch1_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //��ȡ�ӻ���ַ
            }
            else
            {
                gu8_modbus_ch2_addr = gu8a_sys_config_info[SLAVE_ADDR_POS];  //��ȡ�ӻ���ַ
            }
        }
        
        u8a_rs485_rxbuf2[u8_rxbuf2_readcnt][RS485_2_RXBUF_LEN] = 0;  //�建������־
    
        //ѭ���л�������
        if(++u8_rxbuf2_readcnt >= RS485_2_RXBUF_SIZE)
        {
            u8_rxbuf2_readcnt = 0;
        }
    }
    
}


/*************************************************************
 *��������
 *��  �ܣ���Ҫ���͵����ݻ��嵽���ͻ������� 
 *��  �룺datptr:ָ�������ݵ�ָ�룬len��Ҫ�����ֽ���
 *��  ����
 *˵  ����
 *************************************************************/
void vRS485_DataToBuf(uint8_t u8_485_id , uint8_t *u8p_datptr , uint8_t u8_len)
{
    uint8_t i;
    
    if(RS485_1 == u8_485_id)
    {
        if(u8a_rs485_txbuf1[u8_txbuf1_loadcnt][RS485_1_TXBUF_LEN] == 0)
        {
            //��䷢�ͻ�����
            for(i = 0;i < u8_len;i++)                 //������ݣ�˳���������
            {
                u8a_rs485_txbuf1[u8_txbuf1_loadcnt][i] = *u8p_datptr++;
            }
            u8a_rs485_txbuf1[u8_txbuf1_loadcnt][RS485_1_TXBUF_LEN] = u8_len;  //���������һ���ֽڱ���װ�ص����ݳ���

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
            //��䷢�ͻ�����
            for(i = 0;i < u8_len;i++)                 //������ݣ�˳���������
            {
                u8a_rs485_txbuf2[u8_txbuf2_loadcnt][i] = *u8p_datptr++;
            }
            u8a_rs485_txbuf2[u8_txbuf2_loadcnt][RS485_2_TXBUF_LEN] = u8_len;  //���������һ���ֽڱ���װ�ص����ݳ���

            if(++u8_txbuf2_loadcnt >= RS485_2_TXBUF_SIZE)
            {
                u8_txbuf2_loadcnt = 0;
            }
        }
    }
}

/*************************************************************
 *��������
 *��  �ܣ��ڴ�ѭ���е��ã���ѯ�Ƿ�������Ҫ���� 
 *��  �룺
 *��  ����
 *˵  ����
 *************************************************************/
void vRS485_SendFram(void)
{
    uint8_t i;
    
    //��ʱ���ڷ������������߳������У�����Կ�ʼ������һ֡����
    if((BUS_IDLE_STA == u8_bus1_sta) && (u8_busfree_timecnt1 == 0))   
    {
        if(0 != u8a_rs485_txbuf1[u8_txbuf1_sendcnt][RS485_1_TXBUF_LEN])  //��������Ҫ����
        {
            RS485_1_TXMODE();  //����485оƬΪ����ģʽ
            u8_bus1_sta = BUS_TX_STA;   //�����ڷ��ͱ�־
            
            //��Ҫ���͵�����ת�Ƶ����͹�����������
            u8_tx1_datlen = u8a_rs485_txbuf1[u8_txbuf1_sendcnt][RS485_1_TXBUF_LEN];
            for(i = 0;i < u8_tx1_datlen;i++)                           
            {
                u8a_rs485_workbuf1[i] = u8a_rs485_txbuf1[u8_txbuf1_sendcnt][i];
            }
            u8_tx1_datcnt = 0;
            
            //ѭ��ʹ�û�����
            u8a_rs485_txbuf1[u8_txbuf1_sendcnt][RS485_1_TXBUF_LEN] = 0; //�建��������־
            if(++u8_txbuf1_sendcnt >= RS485_1_TXBUF_SIZE)   
            {
                u8_txbuf1_sendcnt = 0;
            }
            
            PIE3bits.TX1IE = 1;        //�򿪷����ж�ʹ�ܣ���������
        }
    }
    
    if(BUS_TX_WAIT_STA == u8_bus1_sta)
    {
        //���һλ���ݷ��ͳ�ȥ֮������������
        if(1 == TX1STAbits.TRMT)
        {
            //���ݷ������
            u8_bus1_sta = BUS_IDLE_STA;   //�����ڷ��ͱ�־λ
            u8_busfree_timecnt1 = 2;       //���¼������߿���ʱ��������10ms
            RS485_1_RXMODE();             //�л�������ģʽ
        }
    }
    
    //����2
    if((BUS_IDLE_STA == u8_bus2_sta) && (u8_busfree_timecnt2 == 0))   
    {
        if(0 != u8a_rs485_txbuf2[u8_txbuf2_sendcnt][RS485_2_TXBUF_LEN])  //��������Ҫ����
        {
            RS485_2_TXMODE();  //����485оƬΪ����ģʽ
            u8_bus2_sta = BUS_TX_STA;   //�����ڷ��ͱ�־
            
            //��Ҫ���͵�����ת�Ƶ����͹�����������
            u8_tx2_datlen = u8a_rs485_txbuf2[u8_txbuf2_sendcnt][RS485_2_TXBUF_LEN];
            for(i = 0;i < u8_tx2_datlen;i++)                           
            {
                u8a_rs485_workbuf2[i] = u8a_rs485_txbuf2[u8_txbuf2_sendcnt][i];
            }
            u8_tx2_datcnt = 0;
            
            //ѭ��ʹ�û�����
            u8a_rs485_txbuf2[u8_txbuf2_sendcnt][RS485_2_TXBUF_LEN] = 0; //�建��������־
            if(++u8_txbuf2_sendcnt >= RS485_2_TXBUF_SIZE)   
            {
                u8_txbuf2_sendcnt = 0;
            }
            
            PIE3bits.TX2IE = 1;        //�򿪷����ж�ʹ�ܣ���������
        }
    }
    
    if(BUS_TX_WAIT_STA == u8_bus2_sta)
    {
        //���һλ���ݷ��ͳ�ȥ֮������������
        if(1 == TX2STAbits.TRMT)
        {
            //���ݷ������
            u8_bus2_sta = BUS_IDLE_STA;   //�����ڷ��ͱ�־λ
            u8_busfree_timecnt2 = 2;       //���¼������߿���ʱ��������10ms
            RS485_2_RXMODE();             //�л�������ģʽ
        }
    }
}

/*************************************************************
 *��������
 *��  �ܣ�����1�ķ����жϷ�����
 *��  �룺
 *��  ����
 *˵  ����
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
            PIE3bits.TX1IE = 0;        //�رշ����ж�ʹ��
            u8_bus1_sta = BUS_TX_WAIT_STA;
        }
    }
}

/*************************************************************
 *��������
 *��  �ܣ�����2�ķ����жϷ�����
 *��  �룺
 *��  ����
 *˵  ����
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
            PIE3bits.TX2IE = 0;        //�رշ����ж�ʹ��
            u8_bus2_sta = BUS_TX_WAIT_STA;
        }
    }
}

/*************************************************************
 *��������
 *��  �ܣ�����1�Ľ����жϷ�����
 *��  �룺
 *��  ����
 *˵  ����
 *************************************************************/
void vRS485_Rx1ISR(void)
{
    uint8_t u8_temp = RC1REG;   //�����Ƿ�Ҫ���棬�ȶ�ȡһ��������жϱ�־λ

    //����д�����־����ֹ�������ݸ���ʱ�������־λ�����޷��ٽ��������ݵ�����
    if(1 == RC1STAbits.OERR)
    {
        // EUSART1 error - restart

        RC1STAbits.CREN = 0;
        RC1STAbits.CREN = 1;
    }
    
    if(BUS_TX_STA != u8_bus1_sta)
    {
        u8_bus1_sta = BUS_RX_STA;   //�л�Ϊ����״̬
        
        //ֻ�л����пյ�ʱ��ű�������
        if(0 == u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][RS485_1_RXBUF_LEN])
        {
            u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][u8_rx1_datcnt++] = u8_temp;
            
            if(u8_rx1_datcnt >= RS485_1_RXBUF_LEN)
            {
                //���յ�����������ݣ���ɽ���
                u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][RS485_1_RXBUF_LEN] = RS485_1_RXBUF_LEN;
                
                //ѭ���л�������
                if(++u8_rxbuf1_loadcnt >= RS485_1_RXBUF_SIZE)
                {
                    u8_rxbuf1_loadcnt = 0;
                }
                
                //���ν��յ�������Ϊ��һ֡
                
                u8_rx1_datcnt = 0;            //������һ֡����������ݸ���
                u8_bus1_sta = BUS_IDLE_STA;   //������һ֡����ָ���������״̬
            }
        }
        
        u8_busfree_timecnt1 = 2;   //ͬʱ�������ճ�ʱ�ж�
    }
}

/*************************************************************
 *��������
 *��  �ܣ�����2�Ľ����жϷ�����
 *��  �룺
 *��  ����
 *˵  ����
 *************************************************************/
void vRS485_Rx2ISR(void)
{
    uint8_t u8_temp = RC2REG;   //�����Ƿ�Ҫ���棬�ȶ�ȡһ��������жϱ�־λ

    //����д�����־����ֹ�������ݸ���ʱ�������־λ�����޷��ٽ��������ݵ�����
    if(1 == RC2STAbits.OERR)
    {
        // EUSART1 error - restart

        RC2STAbits.CREN = 0;
        RC2STAbits.CREN = 1;
    }
    
    if(BUS_TX_STA != u8_bus2_sta)
    {
        u8_bus2_sta = BUS_RX_STA;   //�л�Ϊ����״̬
        
        //ֻ�л����пյ�ʱ��ű�������
        if(0 == u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][RS485_2_RXBUF_LEN])
        {
            u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][u8_rx2_datcnt++] = u8_temp;
            
            if(u8_rx2_datcnt >= RS485_2_RXBUF_LEN)
            {
                //���յ�����������ݣ���ɽ���
                u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][RS485_2_RXBUF_LEN] = RS485_2_RXBUF_LEN;
                
                //ѭ���л�������
                if(++u8_rxbuf2_loadcnt >= RS485_2_RXBUF_SIZE)
                {
                    u8_rxbuf2_loadcnt = 0;
                }
                
                u8_rx2_datcnt = 0;            //������һ֡����������ݸ���
                u8_bus2_sta = BUS_IDLE_STA;   //������һ֡����ָ���������״̬
            }
        }
        
        u8_busfree_timecnt2 = 2;   //ͬʱ�������ճ�ʱ�ж�
    }
}

/******************************************************/
/* ���ƣ�                       */
/* ���ܣ�RS485�ӿڳ�ʱ����������10ms��ʱ���жϺ����е���*/
/* ���룺��                                           */
/* ���أ���                                           */
/* ���ߣ�bobde163                                     */
/* ���ڣ�2018.8.23                                     */
/* ����˵����       */
/******************************************************/
void vRS485_TimerServer(void)
{
    //����1���м���
    if(u8_busfree_timecnt1 > 0)
    {
        u8_busfree_timecnt1--;
        
        //��������10msδ���յ����ݣ�����Ϊ���ճ�ʱ�������߿���
        if(0 == u8_busfree_timecnt1)
        {
            if(BUS_RX_STA == u8_bus1_sta)
            {
                //�ڽ���״̬������ʱ����Ҫ��֡
                if(u8_rx1_datcnt < 4)
                {
                    //�������ճ�ʱʱ���յ������ݲ���4�ֽڣ�������
                    u8_rx1_datcnt = 0;
                }
                else
                {
                    //��Ϊ���յ�һ֡
                    u8a_rs485_rxbuf1[u8_rxbuf1_loadcnt][RS485_1_RXBUF_LEN] = u8_rx1_datcnt;
                
                    //ѭ���л�������
                    if(++u8_rxbuf1_loadcnt >= RS485_1_RXBUF_SIZE)
                    {
                        u8_rxbuf1_loadcnt = 0;
                    }
                    
                    u8_rx1_datcnt = 0;            //������һ֡����������ݸ���
                    u8_bus1_sta = BUS_IDLE_STA;   //������һ֡����ָ���������״̬
                }
            }
        }
    }
    
    //����2���м���
    if(u8_busfree_timecnt2 > 0)
    {
        u8_busfree_timecnt2--;
        
        //��������10msδ���յ����ݣ�����Ϊ���ճ�ʱ�������߿���
        if(0 == u8_busfree_timecnt2)
        {
            if(BUS_RX_STA == u8_bus2_sta)
            {
                //�ڽ���״̬������ʱ����Ҫ��֡
                if(u8_rx2_datcnt < 4)
                {
                    //�������ճ�ʱʱ���յ������ݲ���4�ֽڣ�������
                    u8_rx2_datcnt = 0;
                }
                else
                {
                    //��Ϊ���յ�һ֡
                    u8a_rs485_rxbuf2[u8_rxbuf2_loadcnt][RS485_2_RXBUF_LEN] = u8_rx2_datcnt;
                
                    //ѭ���л�������
                    if(++u8_rxbuf2_loadcnt >= RS485_2_RXBUF_SIZE)
                    {
                        u8_rxbuf2_loadcnt = 0;
                    }
                    
                    u8_rx2_datcnt = 0;            //������һ֡����������ݸ���
                    u8_bus2_sta = BUS_IDLE_STA;   //������һ֡����ָ���������״̬
                }
            }
        }
    }

    //�ر�д����ʱ������
    if(u16_modbus_wp_timeout)
    {
        u16_modbus_wp_timeout--;
    }
    else
    {
        u8_modbus_wp_en = 1;  //�ر�д����ʱ��������������´�д����
    }
}

/*************************************************************
*��������
*��  �ܣ�����MODBUS-RTUЭ���CRC-16�����㷨������ʽΪ0xA001
*��  �룺
*��  ����
*˵  ����
*��  �ߣ�bobde163
*ʱ  �䣺��  ��  ��
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
 *��������
 *��  �ܣ�modbusЭ���ظ�ͨ�Ŵ�����Ϣ
 *��  �룺u8_485_id��ָ��ʹ�õ�ͨ�Žӿ�
 *       u8_addr���ڵ��ַ
 *       u8_cmd:������
 *       u8_err���������
 *��  ����
 *˵  ����
 *************************************************************/
void vModbus_AckErr(uint8_t u8_485_id , uint8_t u8_addr , uint8_t u8_cmd , uint8_t u8_err)
{
    uint8_t u8a_buf[5];
    uint16_t u16_crc16;
    u8a_buf[0] = u8_addr;
    u8a_buf[1] = 0x80u | u8_cmd;
    u8a_buf[2] = u8_err;
    
    u16_crc16 = u16Modbus_Crc16(u8a_buf , 3);  //����CRC16����ֵ
    u8a_buf[3] = (uint8_t)u16_crc16;
    u8a_buf[4] = (uint8_t)(u16_crc16 >> 8);
    
    vRS485_DataToBuf(u8_485_id , u8a_buf , 5);  //������װ�ص����ͻ�������
}

/*************************************************************
 *��������
 *��  �ܣ�ModbusЭ�鴦����
 *��  �룺
 *��  ����
 *˵  ����
 *************************************************************/
void vModbus_ChProc(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint16_t u16_temp;
    uint8_t u8_addr_temp;
    
    //�ȼ��㲢�ж�CRC16У���Ƿ���ȷ
    u16_temp = CHARS_TO_INT16(u8p_dat[u8_len - 1] , u8p_dat[u8_len - 2]);
    if(u16_temp != u16Modbus_Crc16(u8p_dat , u8_len - 2u))
    {
        //У�鲻ͨ����ֱ�ӷ���
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
        //ͨ���Ų���ȷ��ֱ�ӷ���
        return;
    }
    
    //�жϵ�ַ�Ƿ���ȷ
    if(u8p_dat[MOD_ADDR_OFFSET] != u8_addr_temp)
    {
        //��ַ����ȷ���������֡��ֱ�ӷ���
        return;
    }
    
    //�ж���������ִ�ж�Ӧ�Ĳ���
    switch(u8p_dat[MOD_CMD_OFFSET])
    {
        //����ɢ������״ֵ̬
        case READ_DISCRETE_INPUTS:
        {
            vModbus_ReadDiscreteInputProc(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;
        
        //��ȡ��������
        case READ_HOLDING_REG:
        {
            vModbus_ReadHoldingReg(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

		//д�������ּĴ�����ϵͳ�������ݣ�
		case WRITE_SINGLE_REG:
		{
			vModbus_WriteSingleHoldingReg(u8_485_id , u8p_dat , u8_len - 2u);
		}
		break;

        //д������ּĴ�����ϵͳ�������ݣ�
        case WRITE_MULTIPLE_REG:
        {
            vModbus_WriteMultipleHoldingReg(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

        //��ȡ�������������Ĵ������ݣ��������ݺ���Ӳ���汾��
        case READ_INPUT_REG:
        {
            vModbus_ReadInputReg(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

        //ʹ�ò�����ע�͵�
#if 0
        //д������Ȧ��ʵ��ֻ�����޸����߲�����صı�����־��
        case WRITE_SINGLE_COIL:
        {
            vModbus_WriteSingleCoil(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;

        //д�����Ȧ��ʵ��ֻ�����޸����߲�����صı�����־��
        case WRITE_MULTIPLE_COILS:
        {
            vModbus_WriteMultipleCoils(u8_485_id , u8p_dat , u8_len - 2u);
        }
        break;
#endif

        default:
        {
            //��֧�ֵ�����򷵻طǷ�����������
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        }
        break;
    }
}

/*************************************************************
 *��������
 *��  �ܣ���ȡ��ɢ������״̬����������
 *��  �룺
 *��  ����
 *˵  ����
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
         //ͨ���Ų���ȷ��ֱ�ӷ���
         return;
     }
     
     //�ж�������Ƿ����
     if(6 != u8_len)
     {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }
     
     //�ж���ʼ��ַ�Ƿ���ȷ
     u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
     if(u16_temp1 < DISCRETE_INPUT_START_ADDR)
     {
         //С��������ʼ��ַ�����ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
         
         return;
     }
     
     //�ж������Ƿ�ᳬ�����ݽ�����ַ����һ֡�������ɲ���
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ����ֵ
     if((u16_temp2 > DISCRETE_INPUT_NUM_MAX) || ((u16_temp1 + u16_temp2) > (DISCRETE_INPUT_END_ADDR + 1)))
     {
         //Ҫ��ȡ���������ڿɶ�ȡ�����������ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }
     
     //��ʼ��ַ��ȷ����ȡ����Ҳ��������ʼ��ȡ����
     u8a_work_buf[0] = u8_addr_temp;
     u8a_work_buf[1] = READ_DISCRETE_INPUTS;
     u8a_work_buf[2] = u8Modbus_GetDiscreteInput((uint8_t *)gx_sys_status_info , &u8a_work_buf[3] , u16_temp1 , (uint8_t)u16_temp2);
     
     u16_temp1 = u16Modbus_Crc16(u8a_work_buf , u8a_work_buf[2] + 3u);  //����CRC16У��ֵ
     u8a_work_buf[u8a_work_buf[2] + 3u] = (uint8_t)(u16_temp1 & 0xFF);
     u8a_work_buf[u8a_work_buf[2] + 4u] = (uint8_t)(u16_temp1 >> 8);
     
     vRS485_DataToBuf(u8_485_id , u8a_work_buf , u8a_work_buf[2] + 5u);  //���Ͷ�ȡ��������
 }

//ʹ�ò�����ע�͵�
#if 0  
/*************************************************************
*��������
*��  �ܣ�ʹ��д��Ȧ����д��ɢ������״̬����������
*��  �룺
*��  ����
*˵  ����
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
        //ͨ���Ų���ȷ��ֱ�ӷ���
        return;
    }

    //�ж�������Ƿ����
    if(6 != u8_len)
    {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
    }

    //�ж���ʼ��ַ�Ƿ���ȷ��ֻ�п�д�ĵ�ַ�Ż�ִ��
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
    if((u16_temp1 < WP_DISCRETE_INPUT_START_ADDR) || (u16_temp1 > WP_DISCRETE_INPUT_END_ADDR))
    {
        //С��������ʼ��ַ�����ش�����Ϣ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);

        return;
    }

    //����д������ַ����Ҫ�ж��Ƿ��д
    if(1 == u8_modbus_wp_en)
    {
        //����д������ʾ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
        return;
    }

    u8_pos = u16_temp1 - DISCRETE_INPUT_START_ADDR;  //��ȡƫ�Ƶ�ַ

    if(0x00 == u8p_dat[MOD_DATA_OFFSET + 3])
    {
        if(0xFF == u8p_dat[MOD_DATA_OFFSET + 2])
        {
            //0xFF00����1
            gx_sys_status_info[u8_pos / 8u].data |= (1u << (u8_pos % 8u));
        }
        else if(0x00 ==u8p_dat[MOD_DATA_OFFSET + 2])
        {
            //0x0000����0
            gx_sys_status_info[u8_pos / 8u].data &= ~(1u << (u8_pos % 8u));
        }
        else
        {
            //�������ݴ���
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
            return;
        }
    }
    else
    {
        //�������ݴ���
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
        return;
    }

    //��Ӧԭ����
    vRS485_DataToBuf(u8_485_id , u8p_dat , 8u);
}

 /*************************************************************
 *��������
 *��  �ܣ�ʹ��д�����Ȧ����д��ɢ������״̬����������
 *��  �룺
 *��  ����
 *˵  ����
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
         //ͨ���Ų���ȷ��ֱ�ӷ���
         return;
     }
 
     //�ж���ʼ��ַ�Ƿ���ȷ��ֻ�п�д�ĵ�ַ�Ż�ִ��
     u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
     if((u16_temp1 < WP_DISCRETE_INPUT_START_ADDR) || (u16_temp1 > WP_DISCRETE_INPUT_END_ADDR))
     {
         //С��������ʼ��ַ�����ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
 
         return;
     }

     //�ж������Ƿ�ᳬ�����ݽ�����ַ
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ��ַ����ֵ
     if((u16_temp1 + u16_temp2) > (WP_DISCRETE_INPUT_END_ADDR + 1))
     {
         //Ҫ��ȡ���������ڿ�һ���Զ�ȡ���������ߵ��½�����ַ���������ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }

     //����д������ַ����Ҫ�ж��Ƿ��д
     if(1 == u8_modbus_wp_en)
     {
         //����д������ʾ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
         return;
     }

    u8_temp = u16_temp1 - DISCRETE_INPUT_START_ADDR;  //��ȡƫ�Ƶ�ַ
     if(0 == (u8_temp % 8u))
     {
        u8_temp /= 8u;
     }
     else
     {
        u8_temp = u8_temp / 8u + 1u;
     }

     //�ж�������Ƿ����
     if((u8_temp + 7u) != u8_len)
     {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }

     //�ж��ֽ����Ƿ����
     if(u8_temp != u8p_dat[MOD_DATA_OFFSET + 4])
     {
         //������������������Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         return;
     }

     u8_temp = u16_temp1 - DISCRETE_INPUT_START_ADDR;
     u8_data_temp = MOD_DATA_OFFSET + 5;
     for(i = 0;i < u16_temp2;i++)
     {
        if(8u == (i % 8u))  //ÿ8λһ���ֽڲ������ʹ����һ���ֽڿ�ʼ����
        {
            u8_data_temp++;
        }

        if(0x01 & u8p_dat[u8_data_temp])
        {
            //��1
            gx_sys_status_info[u8_temp / 8u].data |= (1u << (u8_temp % 8u));
        }
        else
        {
            //��0
            gx_sys_status_info[u8_temp / 8u].data &= ~(1u << (u8_temp % 8u));
        }
        u8_temp++;
        u8p_dat[u8_data_temp] >>= 1u;
     }
 
     //��Ӧԭ����
     vRS485_DataToBuf(u8_485_id , u8p_dat , 5u);
 }
#endif
 
 /*************************************************************
 *��������
 *��  �ܣ���ϵͳ״̬��Ϣ�а��������ȡ����ɢ����״̬����
 *��  �룺
 *��  ��������ʵ�ʻ�ȡ�����ֽ���
 *˵  ����
 *************************************************************/
 uint8_t u8Modbus_GetDiscreteInput(uint8_t *u8p_status , uint8_t *u8p_result , uint16_t u16_start_addr , uint8_t u8_bits)
 {
     uint8_t u8_start_byte; //�����ϵͳ״̬��Ϣ���ĸ��ֽڿ�ʼȡ����
     uint8_t u8_start_bit;  //���濪ʼ�ֽ��д���һλ��ʼȡ����
     uint8_t u8_dat_cnt = 0; //���������ȡ�����ֽ���
     
     u8_start_byte = (u16_start_addr - DISCRETE_INPUT_START_ADDR) / 8u;  //������ʼ�ֽڵ�ַ
     u8_start_bit = (u16_start_addr - DISCRETE_INPUT_START_ADDR) % 8u;  //������ʼ�ֽڵ�ַ
     
     //��ʼλ����8�ı������ֽڶ�������
     if(0 == u8_start_bit)
     {
         while(0 != u8_bits)
         {
             if(u8_bits >= 8u)
             {
                 //���������ֽڽ��д���
                 u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte];
                 
                 u8_dat_cnt++;
                 u8_start_byte++;
                 u8_bits -= 8u;
             }
             else
             {
                 //�������ļ�λ����
                 u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte] << (8u - u8_bits);
                 u8p_result[u8_dat_cnt] >>= (8u - u8_bits);
                 
                 u8_dat_cnt++;
                 u8_bits = 0;
             }
         }
     }
     //��ʼλ�����ֽڶ�������
     else
     {
         //Ҫ���ֽڴ���
         while(u8_bits > 8)
         {
             //�ȴ�������������ֽڵ�����
             u8p_result[u8_dat_cnt] = (u8p_status[u8_start_byte] >> u8_start_bit) + (u8p_status[u8_start_byte + 1] << (8u - u8_start_bit));
                 
             u8_dat_cnt++;
             u8_start_byte++;
             u8_bits -= 8u;
         }
         
         //�������µĲ���8λ������
         if(0 == u8_bits)
         {
             //û�ж�������ˣ����
         }
         else if(u8_bits == (8u - u8_start_bit))
         {
             //��ǰ�ֽ����µ�bit�����õ������Ҫȡ��bit��
             u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte] >> u8_start_bit;
             u8_dat_cnt++;
         }
         else if(u8_bits > (8u - u8_start_bit))
         {
             //��ǰ�ֽ����µ�bit��������Ҫȡ��bit����ֻ���ٿ�ȡ�¸��ֽڵ�bit
             u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte + 1u] << (16u - u8_bits - u8_start_bit); //���ƣ�Ŀ���ǽ���һ���ֽڵ���Ч����λ�Ƶ�����ߣ�����յ�λ
             u8p_result[u8_dat_cnt] >>= (8u - u8_bits); //���ƣ��������Ч�ĸ�λ
             u8p_result[u8_dat_cnt] += (u8p_status[u8_start_byte] >> u8_start_bit);
             
             u8_dat_cnt++;
         }
         else
         {
             //��ǰ�ֽ����µ�bit�������Ҫȡ��bit��Ҫ�࣬��Ա��ֽڽ��д���
             u8p_result[u8_dat_cnt] = u8p_status[u8_start_byte] << (8u - u8_start_bit - u8_bits); //���ƣ�Ŀ�����������ĸ�λ����
             u8p_result[u8_dat_cnt] >>= (8u - u8_bits);  //���ƣ�Ŀ����ֻ����Ҫȡ�����ݴ����λ��ʼ
             
             u8_dat_cnt++;
         }
     }
     
     return u8_dat_cnt;
 }

  /*************************************************************
 *��������
 *��  �ܣ���ϵͳ������Ϣ�ж�ȡָ������������
 *��  �룺
 *��  ����
 *˵  ����
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
         //ͨ���Ų���ȷ��ֱ�ӷ���
         return;
     }

     //�ж�������Ƿ����
     if(6 != u8_len)
     {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }
     
     //�ж���ʼ��ַ�Ƿ���ȷ
     u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
     if(u16_temp1 < RW_REG_START_ADDR)
     {
         //С��������ʼ��ַ�����ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
         
         return;
     }
     
     //�ж������Ƿ�ᳬ�����ݽ�����ַ����һ֡�������ɲ���
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ����ֵ
     if((u16_temp2 > RW_REG_NUM_MAX) || ((u16_temp1 + u16_temp2) > (RW_REG_END_ADDR + 1)))
     {
         //Ҫ��ȡ���������ڿ�һ���Զ�ȡ���������ߵ��½�����ַ���������ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }
     
     //��ʼ��ַ��ȷ����ȡ����Ҳ��������ʼ��ȡ����������
     u8a_work_buf[0] = u8_addr_temp;
     u8a_work_buf[1] = READ_HOLDING_REG;
     u8a_work_buf[2] = (uint8_t)u16_temp2 * 2u;  //�ֽ���=�Ĵ�������x2
     
     i = (uint8_t)u16_temp2;
     u8p_dat = (uint8_t *)&gu8a_sys_config_info[u16_temp1 - RW_REG_START_ADDR];  //��ȡָ����������Ӧ��ʼ���ݵ�ָ��
     u8p_temp = &u8a_work_buf[3];
     for(i = 0;i < (uint8_t)u16_temp2;i++)
     {
         *u8p_temp++ = 0;           //���ֽں�Ϊ0
         *u8p_temp++ = *u8p_dat++;
     }
     
     u16_temp1 = u16Modbus_Crc16(u8a_work_buf , u8a_work_buf[2] + 3u);  //����CRC16У��ֵ
     u8a_work_buf[u8a_work_buf[2] + 3u] = (uint8_t)(u16_temp1 & 0xFF);
     u8a_work_buf[u8a_work_buf[2] + 4u] = (uint8_t)(u16_temp1 >> 8);
     
     vRS485_DataToBuf(u8_485_id , u8a_work_buf , u8a_work_buf[2] + 5u);  //���Ͷ�ȡ��������
 }
 
   /*************************************************************
 *��������
 *��  �ܣ�д�����Ĵ���
 *��  �룺
 *��  ����
 *˵  ����
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
         //ͨ���Ų���ȷ��ֱ�ӷ���
         return;
     }

     //�ж�������Ƿ����
     if(6 != u8_len)
     {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }

    //�ж���ʼ��ַ�Ƿ���ȷ
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
    if((u16_temp1 < RW_REG_START_ADDR) || (u16_temp1 > RW_REG_END_ADDR))
    {
        //С��������ʼ��ַ�����ش�����Ϣ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);

        return;
    }

    //�ж��Ƿ���д�����ĵ�ַ
    if((u16_temp1 >= WP_RW_REG_START_ADDR) && (u16_temp1 <= WP_RW_REG_END_ADDR))
    {
        //����д������ַ����Ҫ�ж��Ƿ��д
        if(1 == u8_modbus_wp_en)
        {
            //д����������д������ʾ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
            return;
        }
    }
    
    u8_temp = u16_temp1 - RW_REG_START_ADDR;  //����õ�ƫ����

    //�ж�Ҫд���ֵ�Ƿ��ں���Χ
    u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ����ֵ

    //��ӦҪд��ĵ�ַ����Ҫ����ֵ��Χ��飬����Ҫ�ж��Ƿ�С��255
    if(u16_temp1 > RW_REG_VAL_CHECK_END_ADDR)
    {
        if(u16_temp2 > 255u)
        {
            //����Ϊ��Чֵ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

            return;
        }
    }
    //��Ҫ���й涨��ֵ��Χ���
    else
    {
        if((u16_temp2 > gu8a_sys_config_val_max[u8_temp]) || (u16_temp2 < gu8a_sys_config_val_min[u8_temp]))
        {
            //����Ϊ��Чֵ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

            return;
        }
    }

    //����޸Ĺ���ģʽ������Ҫ�ж��Ƿ����
    if(WORK_MODE_POS == u8_temp)
    {
        if(u16_temp2 >= WORK_MODE_MAX)
        {
            //����Ϊ��Чֵ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

            return;
        }
    }
    //�������ʪ��ֵ�Ļ�����Ҫ����Ƿ�����������߼�
    else if(0 == u8SYS_CheckTempRhVal((uint8_t)u16_temp2 , u8_temp))
    {
       vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);

       return;
    }
    else
    {
        //�ж��ǲ���Ҫ��Ƶ
        u8_temp1 = u8RF_FreqSetCheck(u8_temp , (uint8_t)u16_temp2);
        if(1 == u8_temp1)
        {
            //��ʾģ������ͣ��޷���Ƶ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_MODULE_SLEEP);
            return;
        }
        else if(2 == u8_temp1)
        {
            //��ʾ����ɨ��Ƶ���޷���Ƶ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_FREQ_SWITCHING);
            return;
        }
        else
        {
            //�ж��ǲ��Ǹ�ģ������
            if(0 == u8RF_SetDateCheck(u8_temp , (uint8_t)(u16_temp2)))
            {
                //�ж��ǲ����޸�ID
                if(0 == u8RF_SetIdCheck(u8_temp , (uint8_t)(u16_temp2)))
                {
                    //���ֵ�;�ֵ����ȣ������
                   if(gu8a_sys_config_info[u8_temp] != (uint8_t)u16_temp2)
                   {
                       gu8a_sys_config_info[u8_temp] = (uint8_t)u16_temp2;
                        gu8_sys_config_info_update = 1;  //��ϵͳ���ø��±�־λ
                   }
                }
            }
        }
    }

   //�ظ�����һ��������
   vRS485_DataToBuf(u8_485_id , u8p_dat , 8u); 
}
 
   /*************************************************************
 *��������
 *��  �ܣ�д�����Ĵ���
 *��  �룺
 *��  ����
 *˵  ����
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
         //ͨ���Ų���ȷ��ֱ�ӷ���
         return;
     }

    //�ж���ʼ��ַ�Ƿ���ȷ
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
    if(u16_temp1 < RW_REG_START_ADDR)
    {
        //С��������ʼ��ַ�����ش�����Ϣ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);

        return;
    }
    u8_temp = u16_temp1 - RW_REG_START_ADDR;  //����õ�ƫ����
    
    //�ж������Ƿ�ᳬ�����ݽ�����ַ
     u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ�Ĵ�������ֵ
     u16_temp2 += u16_temp1;
     u16_temp2 -= 1u; 
     if(u16_temp2 > RW_REG_END_ADDR)
     {
         //Ҫ��ȡ���������ڿ�һ���Զ�ȡ���������ߵ��½�����ַ���������ش�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         
         return;
     }

     //�ж��Ƿ���д�����ĵ�ַ
    if(((u16_temp1 >= WP_RW_REG_START_ADDR) && (u16_temp1 <= WP_RW_REG_END_ADDR))
        || ((u16_temp2 >= WP_RW_REG_START_ADDR) && (u16_temp2 <= WP_RW_REG_END_ADDR))
        || ((u16_temp1 < WP_RW_REG_START_ADDR) && (u16_temp2 > WP_RW_REG_END_ADDR)))
    {
        //����д������ַ����Ҫ�ж��Ƿ��д
        if(1 == u8_modbus_wp_en)
        {
            //д����������д������ʾ
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , WRITE_PROTECT);
            return;
        }
    }

    u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ�Ĵ�������ֵ

     //�ж�������Ƿ����
     if(((u16_temp2 * 2u) + 7u) != u8_len)
     {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }

     //�ж��ֽ����Ƿ���ڼĴ���������2
     if((u16_temp2 * 2) != u8p_dat[MOD_DATA_OFFSET + 4])
     {
         //������������������Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
         return;
     }

    //�ж�Ҫд���ֵ�Ƿ��ں���Χ
    //��ӦҪд��ĵ�ַ����Ҫ����ֵ��Χ��飬����Ҫ�ж��Ƿ�С��255
    u8p_temp = u8p_dat + 7;  //ָ�����ݿ�ʼ��λ��
    for(i = 0 ; i < u16_temp2 ; i++)
    {
        u16_temp3 = CHARS_TO_INT16(u8p_temp[0] , u8p_temp[1]); //��ȡ����ֵ
        u8p_temp += 2;
        
        if(u16_temp1 > RW_REG_VAL_CHECK_END_ADDR)
        {
            if(u16_temp3 > 255u)
            {
                //����Ϊ��Чֵ
                break;
            }

            //����޸Ĺ���ģʽ������Ҫ�ж��Ƿ����
            if(WORK_MODE_POS == (u8_temp + i))
            {
                if(u16_temp3 >= WORK_MODE_MAX)
                {
                    //����Ϊ��Чֵ
                    break;
                }
            }
        }
        else
        {
            if((u16_temp3 > gu8a_sys_config_val_max[u8_temp + i]) || (u16_temp3 < gu8a_sys_config_val_min[u8_temp + i]))
            {
                //����Ϊ��Чֵ
                break;
            }
        }
    }
    //�������е�ֵ��Χ��鲻ͨ������Ϊ��Ч����
    if(i < u16_temp2)
    {
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
         
         return;
    }
    //���ݶ����ͨ�������¶�Ӧλ��
    else
    {
        u8p_temp = u8p_dat + 7;  //ָ�����ݿ�ʼ��λ��

        //�ȼ�����е���ʪ�ȼ�����º��Ƿ�����߼�
        if(0 == u8SYS_CheckTempRhValAll(u8p_temp , u8_temp , (uint8_t)u16_temp2))
        {
            //��ʾ��ʪ���߼���鲻ͨ��
            vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_VALUE);
         
            return;
        }

        //�ټ�������Ҫ��Ƶʱ���ܲ��ܸ�Ƶ
        for(i = 0 ; i < u16_temp2 ; i++)
        {
            u16_temp3 = CHARS_TO_INT16(u8p_temp[0] , u8p_temp[1]); //��ȡ����ֵ
            u8p_temp += 2;

            //�ж��ǲ���Ҫ��Ƶ
            u8_temp1 = u8RF_FreqSetCheck(u8_temp + i , (uint8_t)u16_temp3);
            if(1 == u8_temp1)
            {
                //��ʾģ������ͣ��޷���Ƶ
                vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_MODULE_SLEEP);
                return;
            }
            else if(2 == u8_temp1)
            {
                //��ʾ����ɨ��Ƶ���޷���Ƶ
                vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , RF_FREQ_SWITCHING);
                return;
            }
        }

        //��ִ�е��˴�����ʾ�����Ҫ������ʪ��������ֵ�Ļ��Ƿ����������߼��ģ����������Ҫ��Ƶ�Ļ�Ҳ����ˣ�
        //Ҳ����˵���д����µ����ݶ��ǺϷ��ģ�����ֱ�Ӹ���
        u8p_temp = u8p_dat + 7;  //ָ�����ݿ�ʼ��λ��
        for(i = 0 ; i < u16_temp2 ; i++)
        {
            u16_temp3 = CHARS_TO_INT16(u8p_temp[0] , u8p_temp[1]); //��ȡ����ֵ
            u8p_temp += 2;

            //�ж��ǲ��Ǹ�ģ������
            if(0 == u8RF_SetDateCheck(u8_temp + i , (uint8_t)u16_temp3))
            {
                //�ж��ǲ����޸�ID
                if(0 == u8RF_SetIdCheck(u8_temp + i , (uint8_t)u16_temp3))
                {
                    gu8a_sys_config_info[u8_temp + i] = (uint8_t)u16_temp3;
                }
            }
        }
        
        gu8_sys_config_info_update = 1;  //��ϵͳ���ô����±�־λ
    }
    //�ظ���������
    u8a_work_buf[0] = u8_addr_temp;
    u8a_work_buf[1] = WRITE_MULTIPLE_REG;
    u8a_work_buf[2] = u8p_dat[2];
    u8a_work_buf[3] = u8p_dat[3];  //��ʼ��ַ
    u8a_work_buf[4] = u8p_dat[4];
    u8a_work_buf[5] = u8p_dat[5];  //�Ĵ�������
    
    u16_temp1 = u16Modbus_Crc16(u8a_work_buf , 6);
    u8a_work_buf[6] = (uint8_t)(u16_temp1 & 0xFF);
    u8a_work_buf[7] = (uint8_t)(u16_temp1 >> 8);
    
    vRS485_DataToBuf(u8_485_id , u8a_work_buf , 8u); 
} 

 /*************************************************************
*��������
*��  �ܣ���ȡ�������ݺ���Ӳ���汾����
*��  �룺u8p_dat��ָ������֡���ݵ�ָ��
*��  ����
*˵  ����
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
         //ͨ���Ų���ȷ��ֱ�ӷ���
         return;
     }

     //�ж�������Ƿ����
     if(6 != u8_len)
     {
        //�����ϱ�����Ĺ̶����ȣ����ع��ܴ�����Ϣ
         vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_FUNCTION);
        return;
     }
    
    //�ж���ʼ��ַ�Ƿ���ȷ
    u16_temp1 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET] , u8p_dat[MOD_DATA_OFFSET + 1]); //��ȡ��ʼ��ֵַ
    if(u16_temp1 < RO_REG_START_ADDR)
    {
        //С��������ʼ��ַ�����ش�����Ϣ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_ADDRESS);
        
        return;
    }
    
    //�ж������Ƿ�ᳬ�����ݽ�����ַ����һ֡�������ɲ���
    u16_temp2 = CHARS_TO_INT16(u8p_dat[MOD_DATA_OFFSET + 2] , u8p_dat[MOD_DATA_OFFSET + 3]); //��ȡ����ֵ
    if((u16_temp2 > RW_REG_NUM_MAX) || ((u16_temp1 + u16_temp2) > (RO_REG_END_ADDR + 1)))
    {
        //Ҫ��ȡ���������ڿ�һ���Զ�ȡ���������ߵ��½�����ַ���������ش�����Ϣ
        vModbus_AckErr(u8_485_id , u8_addr_temp , u8p_dat[MOD_CMD_OFFSET] , ILLEGAL_DATA_NUM);
        
        return;
    }
    
    //��ʼ��ַ��ȷ����ȡ����Ҳ��������ʼ��ȡ����������
    u8a_work_buf[0] = u8_addr_temp;
    u8a_work_buf[1] = READ_INPUT_REG;
    u8a_work_buf[2] = (uint8_t)u16_temp2 * 2u;  //�ֽ���=�Ĵ�������x2
    
    i = (uint8_t)u16_temp2;
    u8p_dat = (uint8_t *)&gu8a_sys_measure_info[u16_temp1 - RO_REG_START_ADDR];  //��ȡָ����������Ӧ��ʼ���ݵ�ָ��
    u8p_temp = &u8a_work_buf[3];
    for(i = 0;i < (uint8_t)u16_temp2;i++)
    {
        if(((u16_temp1 >= RO_REG_SIGNED_START_ADDR1) && (u16_temp1 <= RO_REG_SIGNED_END_ADDR1))
            || ((u16_temp1 >= RO_REG_SIGNED_START_ADDR2) && (u16_temp1 <= RO_REG_SIGNED_END_ADDR2)))
        {
            if(*u8p_dat > 128)
            {
                //���¶Ȼ򶼸�ʪ�ȣ�������ֽ�Ϊ0xFF
                *u8p_temp++ = 0xFF;
            }
            else
            {
                *u8p_temp++ = 0;           //���ֽں�Ϊ0
            }

            *u8p_temp++ = *u8p_dat++;
        }
        else
        {
            *u8p_temp++ = 0;           //���ֽں�Ϊ0

            //Ҫ��ȡ����ģ��ͨѶ�������
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
    
    u16_temp1 = u16Modbus_Crc16(u8a_work_buf , u8a_work_buf[2] + 3u);  //����CRC16У��ֵ
    u8a_work_buf[u8a_work_buf[2] + 3u] = (uint8_t)(u16_temp1 & 0xFF);
    u8a_work_buf[u8a_work_buf[2] + 4u] = (uint8_t)(u16_temp1 >> 8);
    
    vRS485_DataToBuf(u8_485_id , u8a_work_buf , u8a_work_buf[2] + 5u);  //���Ͷ�ȡ��������
}

/*************************************************************
 *��������
 *��  �ܣ���RS485�������е��ã����ж��Ƿ�ִ�����������
 *��  �룺
 *��  ����0�������������1����������
 *˵  ����
 *************************************************************/
uint8_t u8RS485_CheckMakeSetting(uint8_t u8_485_id , uint8_t *u8p_dat , uint8_t u8_len)
{
    uint8_t u8_temp;
    
    if(0 == u8CompareBytes((uint8_t *)"*SETID" , u8p_dat , 6))
    {
        //��λ������ID��
        if(u8_len >= 30)
        {
            //����Ҫ��30�ֽڲ�������������֡
            SetWLTP_ID_Beg = 1;
            Memory_Copy(u8p_dat , gu8a_rf_cmd_buf , 30);

            Conect_STA = 1;//��������ͨѶ
            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*REDID" , u8p_dat , 6))
    {
        //��λ������ID��
        if(u8_len >= 18)
        {
            //����Ҫ��18�ֽڲ�������������֡
            ReadWLTP_ID_Beg = 1;
            Memory_Copy(u8p_dat , gu8a_rf_cmd_buf , 18);

            Conect_STA = 1;//��������ͨѶ
            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*RDLCDSTA" , u8p_dat , 9))
    {
        //��ȡҺ����״ֵ̬�����ڵ���LCD��������
        if(9 <= u8_len)
        {
            //����Ҫ��9�ֽڲ�������������֡
            Memory_Copy("*LCDSTA:" , gu8a_rf_cmd_buf , 8);

            u8_temp = u8LCD_ReadStatus();  //��ȡLCD״ֵ̬
            u8Conv_ByteToBitString(&u8_temp , &gu8a_rf_cmd_buf[8] , 1); //ת���ɶ������ַ���
            vRS485_DataToBuf(u8_485_id , gu8a_rf_cmd_buf , 16);  //���͸���λ��

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*RSTLCD" , u8p_dat , 7))
    {
        //��λLCD�����ڵ���LCD��������
        if(7 <= u8_len)
        {
            //����Ҫ��7�ֽڲ�������������֡
            vGUI_Init();
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD��������򿪣����ް�����������60���ر�
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*RST_LCD_OK" , 11);  //���͸���λ��

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETLCDDISPALL" , u8p_dat , 14))
    {
        //��ʾ�������ص㣬���ڲ���LCD
        if(14 <= u8_len)
        {
            //����Ҫ��14�ֽڲ�������������֡
            gu8_gui_test_lcd_flag = 1;   //����״̬
            vLCD_DispAllWithRam();
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD��������򿪣����ް�����������60���ر�
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*DISP_LCD_OK" , 12);  //���͸���λ��

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETLCDCLRALL" , u8p_dat , 13))
    {
        //����ʾ�������ص㣬���ڲ���LCD
        if(13 <= u8_len)
        {
            //����Ҫ��13�ֽڲ�������������֡
            gu8_gui_test_lcd_flag = 1;   //����״̬
            vLCD_ClearAllWithRam();
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD��������򿪣����ް�����������60���ر�
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLR_LCD_OK" , 11);  //���͸���λ��

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*RESTORELCDALL" , u8p_dat , 14))
    {
        //�ָ�������ʾ�����ڲ���LCD
        if(14 <= u8_len)
        {
            //����Ҫ��13�ֽڲ�������������֡
            gu8_gui_test_lcd_flag = 0;   //�����ָ�����״̬
            vGUI_RestorDisp();
            RESTART_LCD_FRESH();         //���淢���л�������ˢ��ʱ��
            vLED_Flash(LED_POW , LCD_BL_OFF_TIME , LCD_BL_OFF_TIME , 1 , LED_OFF);  //LCD��������򿪣����ް�����������60���ر�
            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*RESTORE_LCD_OK" , 15);  //���͸���λ��

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*REQWR" , u8p_dat , 6))
    {
        //����д�����Ĵ���
        if(6 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            u8_modbus_wp_en = 0;
            u16_modbus_wp_timeout = 500;  //���ü�ʱ��

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*REQOK" , 6);  //���͸���λ��ͬ������д�����Ĵ���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETLAOHUA" , u8p_dat , 10))
    {
        //�����ϻ���־
        if(10 <= u8_len)
        {
            //����Ҫ��10�ֽڲ�������������֡
            LaoHua_sta = 1;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRLAOHUA" , u8p_dat , 10))
    {
        //����ϻ���־
        if(10 <= u8_len)
        {
            //����Ҫ��10�ֽڲ�������������֡
            LaoHua_sta = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*REDLAOHUA" , u8p_dat , 10))
    {
        //��ȡ�ϻ���־
        if(10 <= u8_len)
        {
            //����Ҫ��10�ֽڲ�������������֡
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
        //��ͨ��ʧ�ܱ�����־
        if(13 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            AlarmRF_CommFailFlag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRABSALARM" , u8p_dat , 12))
    {
        //�峬�±�����־
        if(12 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            AlarmAbsFlag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRRELALARM" , u8p_dat , 12))
    {
        //������������־
        if(12 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            AlarmRelFlag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLRBALANCEALARM" , u8p_dat , 16))
    {
        //������²����־
        if(16 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            Alarm_Blance_Flag = 0;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLROK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETCOMMALARM" , u8p_dat , 13))
    {
        //����ͨ��ʧ�ܱ�����־
        if(13 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            AlarmRF_CommFailFlag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETABSALARM" , u8p_dat , 12))
    {
        //���ó��±�����־
        if(12 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            AlarmAbsFlag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETRELALARM" , u8p_dat , 12))
    {
        //��������������־
        if(12 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            AlarmRelFlag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*SETBALANCEALARM" , u8p_dat , 16))
    {
        //��������²����־
        if(16 <= u8_len)
        {
            //����Ҫ��6�ֽڲ�������������֡
            Alarm_Blance_Flag = 0xFFFF;

            vRS485_DataToBuf(u8_485_id , (uint8_t *)"*SETOK" , 6);

            gu8_rf_test_alarm_timercnt = 30;  //����30���ڲ�ִ�����߱�����־���²���

            return 1;
        }
    }
    else if(0 == u8CompareBytes((uint8_t *)"*OPENFAN" , u8p_dat , 8))
    {
        //ǿ�ƴ򿪷���
        gu8_test_fan_flag = 1;
        FAN = 0;                //�򿪷���

        vRS485_DataToBuf(u8_485_id , (uint8_t *)"*OPENOK" , 7);
        
        return 1;
    }
    else if(0 == u8CompareBytes((uint8_t *)"*CLOSEFAN" , u8p_dat , 9))
    {
        //ǿ�ƹرշ���
        gu8_test_fan_flag = 0;
        FAN = 1;                //�رշ���
        vRS485_DataToBuf(u8_485_id , (uint8_t *)"*CLOSEOK" , 8);

        return 1;
    }

    return 0;
}


