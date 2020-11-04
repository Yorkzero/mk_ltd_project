/*****************************************************************
*�ļ����ƣ�modbus.h 
*����������ʹ��Ӳ������ʵ��485ͨ�ź�modbusЭ��ͨ��
*���û��ͣ�
*��    �ߣ�bobde163
*����ʱ�䣺2018��8��22�� V1 ����
*�޸�ʱ�䣺
*�޸�˵����
*****************************************************************/

#ifndef __UART_485_H__
#define __UART_485_H__

#include "xc.h"
#include "pin_map_config.h"
#include "mcc_generated_files/mcc.h"


#define MODBUS_DEV_ADDR        0x02      //�������豸��ַ
#define MODBUS_CMD_RD          0x03      //������������
#define MODBUS_CMD_RD_ERR      0x83      //�����ݴ���������
#define MODBUS_CMD_WR          0x10      //д����������
#define MODBUS_CMD_WR_ERR      0x90      //д���ݴ���������

#define MODBUS_CMD_ERR         0x01      //������Ƿ��������
#define MODBUS_ADDR_ERR        0x02      //��ַ�Ƿ��������
#define MODBUS_DAT_ERR         0x03      //���ݷǷ��������

#define RS485_BUS_FREE         0x00
#define RS485_BUS_BUSY         0x01
#define RS485_WAIT_RSP         0x02
#define RS485_RETURN_RSP       0x03

#define MODBUS_READING         0x00
#define MODBUS_READ_OK         0x01

#define RS485_1_TXBUF_LEN        38
#define RS485_1_TXBUF_SIZE       2
#define RS485_1_RXBUF_LEN        38
#define RS485_1_RXBUF_SIZE       2

#define RS485_2_TXBUF_LEN        38
#define RS485_2_TXBUF_SIZE       2
#define RS485_2_RXBUF_LEN        38
#define RS485_2_RXBUF_SIZE       2

#define RS485_1_BAUDRATE       9600
#define RS485_2_BAUDRATE       9600

#define RS485_BAUDRATE_1200     1
#define RS485_BAUDRATE_2400     2
#define RS485_BAUDRATE_4800     3
#define RS485_BAUDRATE_9600     4
#define RS485_BAUDRATE_19200    5
#define RS485_BAUDRATE_38400    6

#define RS485_1                1
#define RS485_2                2

//����485оƬ�Ƿ��ͻ��ǽ���״̬�������Ϊ����ģʽ�������Ϊ����ģʽ
#define RS485_1_TXMODE()     RS485_DIR_1_SetLow()  //����485Ϊ����ģʽ
#define RS485_1_RXMODE()     RS485_DIR_1_SetHigh()  //����485Ϊ����ģʽ

#define RS485_2_TXMODE()     RS485_DIR_2_SetLow()  //����485Ϊ����ģʽ
#define RS485_2_RXMODE()     RS485_DIR_2_SetHigh()  //����485Ϊ����ģʽ


//                         MODBUS��غ궨��
#define	MOD_ADDR_OFFSET				0
#define	MOD_CMD_OFFSET				1
#define	MOD_DATA_OFFSET				2

/*********************MODBUS������**********************/
#define	READ_COILS					0x01										//��ȡ��Ȧ״̬��ȡ��һ���߼���Ȧ�ĵ�ǰ״̬
#define READ_DISCRETE_INPUTS		0x02
#define	READ_HOLDING_REG			0x03										//��ȡ���ּĴ�������һ���������ּĴ�����ȡ�õ�ǰ�Ķ�����ֵ
#define READ_INPUT_REG				0x04										//������Ĵ���
#define	WRITE_SINGLE_COIL			0x05										//ǿ�õ���Ȧ��ǿ��һ���߼���Ȧ��ͨ��״̬
#define	WRITE_SINGLE_REG			0x06										//Ԥ�õ��Ĵ������Ѿ������ֵװ��һ�����ּĴ���
#define WRITE_MULTIPLE_COILS		0x0F										//д�����Ȧ
#define WRITE_MULTIPLE_REG			0x10										//д����Ĵ���

/********************MODBUS������*********************/
#define	ILLEGAL_FUNCTION			0x01										//���ܴ�����Ч
#define	ILLEGAL_DATA_ADDRESS		0x02										//ָ�������ݵ�ַ��Ч
#define	ILLEGAL_DATA_NUM			0x03										//������Ч
#define	ILLEGAL_DATA_VALUE			0x04										//������Ч

#define WRITE_PROTECT               0x05           //д������˽�д������
#define RF_FREQ_SWITCHING           0x06           //����ģ������ɨ��Ƶ��˽�д������
#define RF_MODULE_SLEEP             0x07           //����ģ��͵�����˽�д������




#define DISCRETE_INPUT_NUM_MAX		0x55   //bob ���ֻ��85������������һ֡����Я��11���ֽ����ݣ�һ���ֽڿ�Я��8����ɢ��״ֵ̬���㹻   
#define	RW_REG_NUM_MAX				((RS485_1_TXBUF_LEN - 5) / 2)   //һ�οɶ�д�����Ĵ�������,modbus��һ���Ĵ���ֵĬ����16λ����
#define DISCRETE_INPUT_START_ADDR   0x2000 //��ɢ������ʼ��ַ
#define DISCRETE_INPUT_END_ADDR     0x2054 //��ɢ���������ַ

#define WP_DISCRETE_INPUT_START_ADDR 0x2020  //�Զ������Ҫд��������ɢ�������ʼ��ַ�����������߱�����־λ
#define WP_DISCRETE_INPUT_END_ADDR   0x2054  //�Զ������Ҫд��������ɢ�����������ַ�����������߱�����־λ


#define	RW_REG_START_ADDR			0x3000 //�ɶ�д�ļĴ�����ʼ��ַ
#define	RW_REG_END_ADDR			    0x308D //�ɶ�д�ļĴ���������ַ
#define RW_REG_VAL_CHECK_END_ADDR   0x3016 //��д��ʱ��Ҫ���д��ֵ�Ƿ�Ϸ��Ľ�����ַ����������Ҫ���д��ֵ
#define RW_REG_VAL_CHECK_NUM_MAX    (RW_REG_VAL_CHECK_END_ADDR - RW_REG_START_ADDR + 1) //��Ҫ��д��ʱ���ֵ��Χ�ļĴ�������

#define	RO_REG_START_ADDR			0x4000 //ֻ��д�ļĴ�����ʼ��ַ
#define	RO_REG_END_ADDR			    0x402F //ֻ��д�ļĴ���������ַ

#define RO_REG_SIGNED_START_ADDR1   0x4000 //�з�������ʼ��ַ1����Ҫ��Ӧ��ʪ�ȴ�������ʪ��ֵ
#define RO_REG_SIGNED_END_ADDR1     0x4004 

#define RO_REG_SIGNED_START_ADDR2   0x4010 //�з�������ʼ��ַ2����Ҫ��Ӧ�����¶�ֵ
#define RO_REG_SIGNED_END_ADDR2     0x401F 

#define RO_REG_RF_COMM_ERR_START_ADDR 0x4020 //����ģ��ͨ�Ŵ������ӳ�䵽�˵�ַ
#define RO_REG_RF_COMM_ERR_END_ADDR   0x402F


#define WP_RW_REG_START_ADDR        0x3013 //��Ҫд�����Ŀɶ�д�Ĵ�����ʼ��ַ
#define WP_RW_REG_END_ADDR          0x308D //��Ҫд�����Ŀɶ�д�Ĵ���������ַ


#define ControlLoopMin				1
#define ControlLoopMax				2
#define ManualHeat_StatusMin		0
#define ManualHeat_StatusMax		1
#define TemperatureThresholdMax		40
#define TemperatureThresholdMin		3
#define HumidityThresholdMax		90
#define HumidityThresholdMin		40
#define FanThresholdMax				50
#define FanThresholdMin				30

#define RS485_USED_SLAVE            RS485_1  //ָ�������ӻ����ܵ�485�ӿ�

#define CHARS_TO_INT16(CH,CL)     ((uint16_t)(CH) << 8) + (CL)   //�������ֽ�ƴ��һ��16λ��

extern volatile uint8_t gu8_modbus_ch1_addr;        //�����ݴ�ͨ���豸�ĵ�ַ���������ݽ���ʱ���ж�
extern volatile uint8_t gu8_modbus_ch2_addr;        //�����ݴ�ͨ���豸�ĵ�ַ���������ݽ���ʱ���ж�

extern uint8_t gu8_modbus_ch1_baudrate;  //���ڱ��浱ǰͨ��1�����ʣ������ж��Ƿ����仯���仯ʱҪ���³�ʼ��������
extern uint8_t gu8_modbus_ch2_baudrate;  //���ڱ��浱ǰͨ��1�����ʣ������ж��Ƿ����仯���仯ʱҪ���³�ʼ��������


extern void vRS485_Init(uint8_t u8_485_id , uint8_t u8_baudrate);
extern void vRS485_RxHandle(void);
extern void vRS485_DataToBuf(uint8_t u8_485_id , uint8_t *u8p_datptr , uint8_t u8_len);

extern void vRS485_TimerServer(void);
extern void vRS485_SendFram(void);






#endif

