/*****************************************************************
*文件名称：modbus.h 
*功能描述：使用硬件串口实现485通信和modbus协议通信
*适用机型：
*作    者：bobde163
*创建时间：2018年8月22日 V1 创建
*修改时间：
*修改说明：
*****************************************************************/

#ifndef __UART_485_H__
#define __UART_485_H__

#include "xc.h"
#include "pin_map_config.h"
#include "mcc_generated_files/mcc.h"


#define MODBUS_DEV_ADDR        0x02      //电流表设备地址
#define MODBUS_CMD_RD          0x03      //读数据命令码
#define MODBUS_CMD_RD_ERR      0x83      //读数据错误命令码
#define MODBUS_CMD_WR          0x10      //写数据命令码
#define MODBUS_CMD_WR_ERR      0x90      //写数据错误命令码

#define MODBUS_CMD_ERR         0x01      //命令码非法错误代码
#define MODBUS_ADDR_ERR        0x02      //地址非法错误代码
#define MODBUS_DAT_ERR         0x03      //数据非法错误代码

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

//控制485芯片是发送还是接收状态，输出高为接收模式，输出低为发送模式
#define RS485_1_TXMODE()     RS485_DIR_1_SetLow()  //设置485为发送模式
#define RS485_1_RXMODE()     RS485_DIR_1_SetHigh()  //设置485为接收模式

#define RS485_2_TXMODE()     RS485_DIR_2_SetLow()  //设置485为发送模式
#define RS485_2_RXMODE()     RS485_DIR_2_SetHigh()  //设置485为接收模式


//                         MODBUS相关宏定义
#define	MOD_ADDR_OFFSET				0
#define	MOD_CMD_OFFSET				1
#define	MOD_DATA_OFFSET				2

/*********************MODBUS功能码**********************/
#define	READ_COILS					0x01										//读取线圈状态，取得一组逻辑线圈的当前状态
#define READ_DISCRETE_INPUTS		0x02
#define	READ_HOLDING_REG			0x03										//读取保持寄存器，在一个或多个保持寄存器中取得当前的二进制值
#define READ_INPUT_REG				0x04										//读输入寄存器
#define	WRITE_SINGLE_COIL			0x05										//强置单线圈，强置一个逻辑线圈的通断状态
#define	WRITE_SINGLE_REG			0x06										//预置单寄存器，把具体二进值装入一个保持寄存器
#define WRITE_MULTIPLE_COILS		0x0F										//写多个线圈
#define WRITE_MULTIPLE_REG			0x10										//写多个寄存器

/********************MODBUS错误码*********************/
#define	ILLEGAL_FUNCTION			0x01										//功能代码无效
#define	ILLEGAL_DATA_ADDRESS		0x02										//指定的数据地址无效
#define	ILLEGAL_DATA_NUM			0x03										//数量无效
#define	ILLEGAL_DATA_VALUE			0x04										//数据无效

#define WRITE_PROTECT               0x05           //写保护，私有错误代码
#define RF_FREQ_SWITCHING           0x06           //无线模块正在扫改频，私有错误代码
#define RF_MODULE_SLEEP             0x07           //无线模块低电流，私有错误代码




#define DISCRETE_INPUT_NUM_MAX		0x55   //bob 最多只有85个输入量，而一帧最多可携带11个字节数据，一个字节可携带8个离散入状态值，足够   
#define	RW_REG_NUM_MAX				((RS485_1_TXBUF_LEN - 5) / 2)   //一次可读写的最大寄存器数量,modbus中一个寄存器值默认是16位长度
#define DISCRETE_INPUT_START_ADDR   0x2000 //离散输入起始地址
#define DISCRETE_INPUT_END_ADDR     0x2054 //离散输入结束地址

#define WP_DISCRETE_INPUT_START_ADDR 0x2020  //自定义的需要写保护的离散量输入地始地址，关联到无线报警标志位
#define WP_DISCRETE_INPUT_END_ADDR   0x2054  //自定义的需要写保护的离散量输入结束地址，关联到无线报警标志位


#define	RW_REG_START_ADDR			0x3000 //可读写的寄存器起始地址
#define	RW_REG_END_ADDR			    0x308D //可读写的寄存器结束地址
#define RW_REG_VAL_CHECK_END_ADDR   0x3016 //在写入时需要检查写入值是否合法的结束地址，超出则不需要检查写入值
#define RW_REG_VAL_CHECK_NUM_MAX    (RW_REG_VAL_CHECK_END_ADDR - RW_REG_START_ADDR + 1) //需要在写入时检查值范围的寄存器总数

#define	RO_REG_START_ADDR			0x4000 //只读写的寄存器起始地址
#define	RO_REG_END_ADDR			    0x402F //只读写的寄存器结束地址

#define RO_REG_SIGNED_START_ADDR1   0x4000 //有符号数起始地址1，主要对应温湿度传感器温湿度值
#define RO_REG_SIGNED_END_ADDR1     0x4004 

#define RO_REG_SIGNED_START_ADDR2   0x4010 //有符号数起始地址2，主要对应无线温度值
#define RO_REG_SIGNED_END_ADDR2     0x401F 

#define RO_REG_RF_COMM_ERR_START_ADDR 0x4020 //无线模块通信错误次数映射到此地址
#define RO_REG_RF_COMM_ERR_END_ADDR   0x402F


#define WP_RW_REG_START_ADDR        0x3013 //需要写保护的可读写寄存器起始地址
#define WP_RW_REG_END_ADDR          0x308D //需要写保护的可读写寄存器结束地址


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

#define RS485_USED_SLAVE            RS485_1  //指定用作从机功能的485接口

#define CHARS_TO_INT16(CH,CL)     ((uint16_t)(CH) << 8) + (CL)   //将两个字节拼成一个16位数

extern volatile uint8_t gu8_modbus_ch1_addr;        //用于暂存通信设备的地址，用于数据接收时的判断
extern volatile uint8_t gu8_modbus_ch2_addr;        //用于暂存通信设备的地址，用于数据接收时的判断

extern uint8_t gu8_modbus_ch1_baudrate;  //用于保存当前通道1波特率，定期判断是否发生变化，变化时要重新初始化波特率
extern uint8_t gu8_modbus_ch2_baudrate;  //用于保存当前通道1波特率，定期判断是否发生变化，变化时要重新初始化波特率


extern void vRS485_Init(uint8_t u8_485_id , uint8_t u8_baudrate);
extern void vRS485_RxHandle(void);
extern void vRS485_DataToBuf(uint8_t u8_485_id , uint8_t *u8p_datptr , uint8_t u8_len);

extern void vRS485_TimerServer(void);
extern void vRS485_SendFram(void);






#endif

