/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :modbus_module.c 
* Desc     :modbus通信模块
* 
* 
* Author   :CenLinbo
* Date     :2020/08/27
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/27, CenLinbo create this file
*         
******************************************************************************/


/*------------------------------- Includes ----------------------------------*/
#include "modbus_module.h"
#include "uart_485_drv.h"

/*------------------- Global Definitions and Declarations -------------------*/
#define CHARS_TO_INT16(CH,CL)     ((uint16_t)(CH) << 8) + (CL)   //将两个字节拼成一个16位数


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/
static uint8_t mb_addr;   //内存中保存的总线地址变量，在上电时从掉电保存的参数中初始化

/*----------------------- Function Implement --------------------------------*/
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

void MB_Init(void)
{
    
}

uint8_t MB_Read_Reg_Handler(uint16_t reg_addr , uint8_t *datptr)
{
    uint8_t err_code = MB_EX_NONE;
    uint8_t index;  //在处理连续过长的信息时用作寻址数据位置

    //公共信息0000-0FFF为只读
    //采集信息1000-1FFF为只读
    //
    //对于过长的信息，使用if语句来处理，以节省代码空间
    if((reg_addr >= 0x1004) && (reg_addr <= 0x1033))
    {
        //读取用户ID号，最长可达96字节，待完善
        index = reg_addr - 0x1004;
        index <<= 1;
        datptr[0] = 0;
        datptr[1] = 1;
    }
    else if((reg_addr >= 0x000E) && (reg_addr <= 0x0013))
    {
        //单板或模块名，12字节
        index = reg_addr - 0x000E;
        index <<= 1;
        datptr[0] = sys_device_name[index];
        datptr[1] = sys_device_name[index + 1];
    }
    else if((reg_addr >= 0x0014) && (reg_addr <= 0x001A))
    {
        //系统描述，14字节
        index = reg_addr - 0x0014;
        index <<= 1;
        datptr[0] = sys_device_discribe[index];
        datptr[1] = sys_device_discribe[index + 1];
    }
    else if((reg_addr >= 0x001B) && (reg_addr <= 0x001E))
    {
        //机型(设备小类)，填充空格
        datptr[0] = ' ';
        datptr[1] = ' ';
    }
    else
    {
        switch(reg_addr)
        {
            //协议版本，使用1
            case 0x0000:
                datptr[0] = 0;
                datptr[1] = 1;
                break;

           //软件版本V1.0
            case 0x0001:
                datptr[0] = 1;
                datptr[1] = 0;
                break;

            //硬件PCB版本
            case 0x0002:
                datptr[0] = 0;
                datptr[1] = 'A';
                break;

            //设备类型
            case 0x0003:
                datptr[0] = 0;
                datptr[1] = 0x03;//ID刷卡锁
                break;

            //生产厂家
            case 0x0004:
                datptr[0] = 0;
                datptr[1] = 0x02;//华为
                break;

            //是否支持加载
            case 0x0005:
                datptr[0] = 0;
                datptr[1] = 0xFF;//不支持加载
                break;

            //硬件可编程器版本
            case 0x0006:
                datptr[0] = 0;
                datptr[1] = 0;//没有则填0
                break;

            //编译时间年
            case 0x0007:
                datptr[0] = 0;
                datptr[1] = sys_complied_date[0];//16进制格式
                break;

            //编译时间月
            case 0x0008:
                datptr[0] = 0;
                datptr[1] = sys_complied_date[1];//16进制格式
                break;

            //编译时间日
            case 0x0009:
                datptr[0] = 0;
                datptr[1] = sys_complied_date[2];//16进制格式
                break;

            //设备属性
            case 0x000A:
                datptr[0] = 0;
                datptr[1] = 0x01;//固定为0x01
                break;

            //子软件ID，预留
            case 0x000B:
                datptr[0] = 0;
                datptr[1] = 0;
                break;

            //单板ID,0x00000003
            case 0x000C:
                datptr[0] = 0;
                datptr[1] = 0;
                break;
            case 0x000D:
                datptr[0] = 0;
                datptr[1] = 0x03;
                break;

            //供电类型
            case 0x001F:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;//12V供电
                break;
            }


            //以下为采集信息地址段
            //门锁状态
            case 0x1000:
            {
                datptr[0] = 0;
                datptr[1] = unlock_get_lock_status();
                break;
            }

            //开锁原因，待完善
            case 0x1001:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;
                break;
            }

            //重复上报次数，待完善
            case 0x1002:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;//12V供电
                break;
            }

            //用户ID长度，待完善
            case 0x1003:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;
                break;
            }

            //是否支持自动闭合，待完善
            case 0x1200:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //不支持
                break;
            }

            //是否支持电平开锁，待完善
            case 0x1201:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //不支持
                break;
            }

            //是否支持开锁监控，待完善
            case 0x1202:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //支持
                break;
            }

            //是否支持指示灯，待完善
            case 0x1203:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //支持
                break;
            }

            //是否支持蜂鸣器指示，待完善
            case 0x1204:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //不支持
                break;
            }

            //是否支持RFID开锁，待完善
            case 0x1205:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //支持
                break;
            }

            //是否支持蓝牙开锁，待完善
            case 0x1206:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //不支持
                break;
            }

            //是否支持钥匙开锁，待完善
            case 0x1207:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //支持
                break;
            }

            //手柄状态
            case 0x1208:
            {
                datptr[0] = 0;
                datptr[1] = unlock_get_hall_status();
                break;
            }

            //锁芯状态，待完善
            case 0x1208:
            {
                datptr[0] = 0;
                datptr[1] = unlock_get_hall_status();
                break;
            }
        }
    }
}

uint8_t MB_Write_Reg_Handler(uint16_t reg_addr , uint8_t *datptr)
{
    uint8_t err_code = MB_EX_NONE;
    uint8_t index;  //在处理连续过长的信息时用作寻址数据位置

    switch(reg_addr)
    {
        //门锁告警使能设置，待完善
        case 0x2100:
            if((0 != datptr[0]) || ((0x55 != datptr[1]) && (0xAA != datptr[1])))
            {
                //数据错误
                err_code = MB_EX_ILLEGAL_DATA_VALUE;
            }
            else
            {
            }
            break;

        //用户操作告警使能设置，待完善
        case 0x2101:
            if((0 != datptr[0]) || ((0x55 != datptr[1]) && (0xAA != datptr[1])))
            {
                //数据错误
                err_code = MB_EX_ILLEGAL_DATA_VALUE;
            }
            else
            {
            }
            break;

        //控制信息地址段
        //单板复位，待完善
        case 0x4000:
            if((0 != datptr[0]) || (0x03 != datptr[1]))
            {
                //数据错误
                err_code = MB_EX_ILLEGAL_DATA_VALUE;
            }
            else
            {
            }
            break;
    }

    return err_code;
}




void MB_Cmd_Parse(uint8_t *datptr , uint8_t size)
{
    uint8_t cmd = datptr[1];
    uint16_t start_reg_addr = CHARS_TO_INT16(datptr[2] , datptr[3]);
    uint16_t reg_num = CHARS_TO_INT16(datptr[4] , datptr[5]);
    uint16_t crc_val;
    uint16_t i;
    uint8_t dat_cnt = 0;  //用于记录应答数据的字节数

    uint8_t err_code = MB_EX_NONE;

    switch(cmd)
    {
        //读单个或者多个寄存器
        case MB_FUNC_READ_HOLDING_REGISTER:
            if(reg_num > 125)
                err_code = MB_EX_ILLEGAL_DATA_ADDRESS;
            else
            {
                for(i = 0;i < reg_num;i++)
                {
                    //使用原接收帧收装数据
                    err_code = MB_Read_Reg_Handler(start_reg_addr , (datptr + 3) + (i << 1));
                    if(MB_EX_NONE != err_code)
                    {
                        //响应错误帧
                        break;
                    }

                    start_reg_addr++;
                }

                //读取无错误，应答数据
                if(i == reg_num)
                {
                    datptr[2] = (uint8_t)(reg_num << 1);
                    
                    crc_val = u16Modbus_Crc16(datptr , datptr[2] + 3);
                    datptr[3] = (uint8_t)(crc_val & 0x00FF);
                    datptr[4] = (uint8_t)(crc_val >> 8);
                    
                    RS485_DataToBuf(datptr , datptr[2] + 5);
                }
            }
            break;

        //写单个寄存器
        case MB_FUNC_WRITE_REGISTER:
            break;

        //写多个寄存器
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            //验证数据数量是否匹配
            if(datptr[6] != (size - 9)) || (reg_num != (datptr[6] >> 1))
            {
                err_code = MB_EX_ILLEGAL_DATA_VALUE;  //非法数据值
            }
            else
            {
                for(i = 0;i < reg_num;i++)
                {
                    //使用原接收帧收装数据
                    err_code = MB_Write_Reg_Handler(start_reg_addr , (datptr + 7) + (i << 1));
                    if(MB_EX_NONE != err_code)
                    {
                        //响应错误帧
                        break;
                    }

                    start_reg_addr++;
                }

                //写入无错误，应答数据
                if(i == reg_num)
                {
                    datptr[2] = (uint8_t)(reg_num << 1);
                    
                    crc_val = u16Modbus_Crc16(datptr , datptr[2] + 3);
                    datptr[3] = (uint8_t)(crc_val & 0x00FF);
                    datptr[4] = (uint8_t)(crc_val >> 8);
                    
                    RS485_DataToBuf(datptr , datptr[2] + 5);
                }
            }
            break;

        default:
            err_code = MB_EX_ILLEGAL_FUNCTION;
            break;
    }

    if(MB_EX_NONE != err_code)
    {
        //响应错误帧
        datptr[1] |= MB_FUNC_ERROR;
        datptr[2] = err_code;
        crc_val = u16Modbus_Crc16(datptr , 3);
        datptr[3] = (uint8_t)(crc_val & 0x00FF);
        datptr[4] = (uint8_t)(crc_val >> 8);

        RS485_DataToBuf(datptr , 5);
    }
}

void MB_Rx_Handler(uint8_t *datptr , uint8_t size)
{
    uint16_t crc_val , crc_val_temp;
    uint8_t addr;
    
    if((NULL == datptr) || (size < MB_SER_PDU_SIZE_MIN))
        return;

    addr = datptr[MB_SER_PDU_ADDR_OFF];
    if((MB_ADDRESS_BROADCAST == addr) || (mb_addr == addr))
    {
        //地址符合，进行下一步检验
        crc_val = u16Modbus_Crc16(datptr , size - 2);
        crc_val_temp = datptr[size - 1];
        crc_val_temp <<= 8;
        crc_val_temp += datptr[size - 2];

        if(crc_val != crc_val_temp)
            return;  //CRC16检验不通过，直接返回
        
    }
    else
        return;  //地址不匹配，直接返回

    //能执行到此表示地址和CRC检验都通过了
    
}

void MB_Run(void)
{
    

    if()
}

/*---------------------------------------------------------------------------*/

