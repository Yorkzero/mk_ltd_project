/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :modbus_module.c 
* Desc     :modbusͨ��ģ��
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
#define CHARS_TO_INT16(CH,CL)     ((uint16_t)(CH) << 8) + (CL)   //�������ֽ�ƴ��һ��16λ��


/*---------------------- Constant / Macro Definitions -----------------------*/


/*----------------------- Type Declarations ---------------------------------*/


/*----------------------- Variable Declarations -----------------------------*/


/*----------------------- Function Prototype --------------------------------*/
static uint8_t mb_addr;   //�ڴ��б�������ߵ�ַ���������ϵ�ʱ�ӵ��籣��Ĳ����г�ʼ��

/*----------------------- Function Implement --------------------------------*/
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

void MB_Init(void)
{
    
}

uint8_t MB_Read_Reg_Handler(uint16_t reg_addr , uint8_t *datptr)
{
    uint8_t err_code = MB_EX_NONE;
    uint8_t index;  //�ڴ���������������Ϣʱ����Ѱַ����λ��

    //������Ϣ0000-0FFFΪֻ��
    //�ɼ���Ϣ1000-1FFFΪֻ��
    //
    //���ڹ�������Ϣ��ʹ��if����������Խ�ʡ����ռ�
    if((reg_addr >= 0x1004) && (reg_addr <= 0x1033))
    {
        //��ȡ�û�ID�ţ���ɴ�96�ֽڣ�������
        index = reg_addr - 0x1004;
        index <<= 1;
        datptr[0] = 0;
        datptr[1] = 1;
    }
    else if((reg_addr >= 0x000E) && (reg_addr <= 0x0013))
    {
        //�����ģ������12�ֽ�
        index = reg_addr - 0x000E;
        index <<= 1;
        datptr[0] = sys_device_name[index];
        datptr[1] = sys_device_name[index + 1];
    }
    else if((reg_addr >= 0x0014) && (reg_addr <= 0x001A))
    {
        //ϵͳ������14�ֽ�
        index = reg_addr - 0x0014;
        index <<= 1;
        datptr[0] = sys_device_discribe[index];
        datptr[1] = sys_device_discribe[index + 1];
    }
    else if((reg_addr >= 0x001B) && (reg_addr <= 0x001E))
    {
        //����(�豸С��)�����ո�
        datptr[0] = ' ';
        datptr[1] = ' ';
    }
    else
    {
        switch(reg_addr)
        {
            //Э��汾��ʹ��1
            case 0x0000:
                datptr[0] = 0;
                datptr[1] = 1;
                break;

           //����汾V1.0
            case 0x0001:
                datptr[0] = 1;
                datptr[1] = 0;
                break;

            //Ӳ��PCB�汾
            case 0x0002:
                datptr[0] = 0;
                datptr[1] = 'A';
                break;

            //�豸����
            case 0x0003:
                datptr[0] = 0;
                datptr[1] = 0x03;//IDˢ����
                break;

            //��������
            case 0x0004:
                datptr[0] = 0;
                datptr[1] = 0x02;//��Ϊ
                break;

            //�Ƿ�֧�ּ���
            case 0x0005:
                datptr[0] = 0;
                datptr[1] = 0xFF;//��֧�ּ���
                break;

            //Ӳ���ɱ�����汾
            case 0x0006:
                datptr[0] = 0;
                datptr[1] = 0;//û������0
                break;

            //����ʱ����
            case 0x0007:
                datptr[0] = 0;
                datptr[1] = sys_complied_date[0];//16���Ƹ�ʽ
                break;

            //����ʱ����
            case 0x0008:
                datptr[0] = 0;
                datptr[1] = sys_complied_date[1];//16���Ƹ�ʽ
                break;

            //����ʱ����
            case 0x0009:
                datptr[0] = 0;
                datptr[1] = sys_complied_date[2];//16���Ƹ�ʽ
                break;

            //�豸����
            case 0x000A:
                datptr[0] = 0;
                datptr[1] = 0x01;//�̶�Ϊ0x01
                break;

            //�����ID��Ԥ��
            case 0x000B:
                datptr[0] = 0;
                datptr[1] = 0;
                break;

            //����ID,0x00000003
            case 0x000C:
                datptr[0] = 0;
                datptr[1] = 0;
                break;
            case 0x000D:
                datptr[0] = 0;
                datptr[1] = 0x03;
                break;

            //��������
            case 0x001F:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;//12V����
                break;
            }


            //����Ϊ�ɼ���Ϣ��ַ��
            //����״̬
            case 0x1000:
            {
                datptr[0] = 0;
                datptr[1] = unlock_get_lock_status();
                break;
            }

            //����ԭ�򣬴�����
            case 0x1001:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;
                break;
            }

            //�ظ��ϱ�������������
            case 0x1002:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;//12V����
                break;
            }

            //�û�ID���ȣ�������
            case 0x1003:
            {
                datptr[0] = 0;
                datptr[1] = 0x01;
                break;
            }

            //�Ƿ�֧���Զ��պϣ�������
            case 0x1200:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //��֧��
                break;
            }

            //�Ƿ�֧�ֵ�ƽ������������
            case 0x1201:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //��֧��
                break;
            }

            //�Ƿ�֧�ֿ�����أ�������
            case 0x1202:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //֧��
                break;
            }

            //�Ƿ�֧��ָʾ�ƣ�������
            case 0x1203:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //֧��
                break;
            }

            //�Ƿ�֧�ַ�����ָʾ��������
            case 0x1204:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //��֧��
                break;
            }

            //�Ƿ�֧��RFID������������
            case 0x1205:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //֧��
                break;
            }

            //�Ƿ�֧������������������
            case 0x1206:
            {
                datptr[0] = 0;
                datptr[1] = 0xAA;  //��֧��
                break;
            }

            //�Ƿ�֧��Կ�׿�����������
            case 0x1207:
            {
                datptr[0] = 0;
                datptr[1] = 0x55;  //֧��
                break;
            }

            //�ֱ�״̬
            case 0x1208:
            {
                datptr[0] = 0;
                datptr[1] = unlock_get_hall_status();
                break;
            }

            //��о״̬��������
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
    uint8_t index;  //�ڴ���������������Ϣʱ����Ѱַ����λ��

    switch(reg_addr)
    {
        //�����澯ʹ�����ã�������
        case 0x2100:
            if((0 != datptr[0]) || ((0x55 != datptr[1]) && (0xAA != datptr[1])))
            {
                //���ݴ���
                err_code = MB_EX_ILLEGAL_DATA_VALUE;
            }
            else
            {
            }
            break;

        //�û������澯ʹ�����ã�������
        case 0x2101:
            if((0 != datptr[0]) || ((0x55 != datptr[1]) && (0xAA != datptr[1])))
            {
                //���ݴ���
                err_code = MB_EX_ILLEGAL_DATA_VALUE;
            }
            else
            {
            }
            break;

        //������Ϣ��ַ��
        //���帴λ��������
        case 0x4000:
            if((0 != datptr[0]) || (0x03 != datptr[1]))
            {
                //���ݴ���
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
    uint8_t dat_cnt = 0;  //���ڼ�¼Ӧ�����ݵ��ֽ���

    uint8_t err_code = MB_EX_NONE;

    switch(cmd)
    {
        //���������߶���Ĵ���
        case MB_FUNC_READ_HOLDING_REGISTER:
            if(reg_num > 125)
                err_code = MB_EX_ILLEGAL_DATA_ADDRESS;
            else
            {
                for(i = 0;i < reg_num;i++)
                {
                    //ʹ��ԭ����֡��װ����
                    err_code = MB_Read_Reg_Handler(start_reg_addr , (datptr + 3) + (i << 1));
                    if(MB_EX_NONE != err_code)
                    {
                        //��Ӧ����֡
                        break;
                    }

                    start_reg_addr++;
                }

                //��ȡ�޴���Ӧ������
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

        //д�����Ĵ���
        case MB_FUNC_WRITE_REGISTER:
            break;

        //д����Ĵ���
        case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
            //��֤���������Ƿ�ƥ��
            if(datptr[6] != (size - 9)) || (reg_num != (datptr[6] >> 1))
            {
                err_code = MB_EX_ILLEGAL_DATA_VALUE;  //�Ƿ�����ֵ
            }
            else
            {
                for(i = 0;i < reg_num;i++)
                {
                    //ʹ��ԭ����֡��װ����
                    err_code = MB_Write_Reg_Handler(start_reg_addr , (datptr + 7) + (i << 1));
                    if(MB_EX_NONE != err_code)
                    {
                        //��Ӧ����֡
                        break;
                    }

                    start_reg_addr++;
                }

                //д���޴���Ӧ������
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
        //��Ӧ����֡
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
        //��ַ���ϣ�������һ������
        crc_val = u16Modbus_Crc16(datptr , size - 2);
        crc_val_temp = datptr[size - 1];
        crc_val_temp <<= 8;
        crc_val_temp += datptr[size - 2];

        if(crc_val != crc_val_temp)
            return;  //CRC16���鲻ͨ����ֱ�ӷ���
        
    }
    else
        return;  //��ַ��ƥ�䣬ֱ�ӷ���

    //��ִ�е��˱�ʾ��ַ��CRC���鶼ͨ����
    
}

void MB_Run(void)
{
    

    if()
}

/*---------------------------------------------------------------------------*/

