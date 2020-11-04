/******************************************************************************
* Copyright 2020-2025 cenlinbo
* FileName :main.h 
* Desc     :���仪Ϊ���Ӱ���������Ŀ
* 
* 
* Author   :CenLinbo
* Date     :2020/08/17
* Notes    :
* 
* -----------------------------------------------------------------
* Histroy:v1.0   2020/08/17, CenLinbo create this file
*         
******************************************************************************/
#ifndef _MAIN_H_     
#define _MAIN_H_    


/*------------------------------- Includes ----------------------------------*/
#include "stm8l15x.h"
#include <stddef.h>

/*----------------------------- Global Defines ------------------------------*/
//ģ�⴮�ڴ�ӡ����
#define SIM_UART_PRINTF_EN     0

//ϵͳʱ��ֵ�궨��
#define SYS_CLK_FREQ_16M       16000000
#define SYS_CLK_FREQ_8M        8000000
#define SYS_CLK_FREQ_4M        4000000
#define SYS_CLK_FREQ_2M        2000000
#define SYS_CLK_FREQ_1M        1000000
#define SYS_CLK_FREQ_500K      500000
#define SYS_CLK_FREQ_250K      250000
#define SYS_CLK_FREQ_125K      125000


//����ϵͳʱ�ӣ��ڳ�ʼ��ʱ��ʱ���ݴ˶����ʼ����ע����ϵͳʹ���ڲ�16MHz HSIʱ��Դ
#define SYS_CLK_FREQ          SYS_CLK_FREQ_4M    //�ϵ�Ĭ��Ϊ2M���ɸ�������޸�

//����������
#define KEY_GPIO_Port         GPIOA
#define KEY_Pin               GPIO_Pin_2
#define KEY_EXIT_Pin          EXTI_Pin_2
#define KEY_READ()            (KEY_GPIO_Port->IDR & KEY_Pin)
#define KEY_ENABLE()		  GPIO_Init(KEY_GPIO_Port , KEY_Pin , GPIO_Mode_In_FL_IT)
#define KEY_DISABLE()		  GPIO_Init(KEY_GPIO_Port , KEY_Pin , GPIO_Mode_In_PU_IT)


//�񶯴�����
#define VIB_SENSOR_Port     GPIOA
#define VIB_SENSOR_Pin      GPIO_Pin_3
#define VIB_SENSOR_EXIT_Pin  EXTI_Pin_3

#define VIB_EN_Port     GPIOC
#define VIB_EN_Pin      GPIO_Pin_4


#define VIB_IT_ENABLE()        GPIO_Init(VIB_SENSOR_Port , VIB_SENSOR_Pin , GPIO_Mode_In_FL_IT)
#define VIB_IT_DISABLE()       GPIO_Init(VIB_SENSOR_Port , VIB_SENSOR_Pin , GPIO_Mode_In_FL_No_IT)

#define VIB_ENABLE()        (VIB_EN_Port -> ODR |= VIB_EN_Pin)
#define VIB_DISABLE()       (VIB_EN_Port -> ODR &= ~VIB_EN_Pin)

//��ص�ѹ���
#define LOWV_ADC_Port        GPIOD
#define LOWV_ADC_Pin         GPIO_Pin_0
#define LOWV_EN_Port        GPIOB
//#define LOWV_EN_Pin         GPIO_Pin_5// v1.0
#define LOWV_EN_Pin         GPIO_Pin_0// v1.1
#define LOWV_ENABLE()       {LOWV_ADC_Port->DDR&= ~LOWV_ADC_Pin;LOWV_EN_Port->ODR &= ~LOWV_EN_Pin;}
#define LOWV_DISABLE()      {LOWV_EN_Port->ODR |= LOWV_EN_Pin;LOWV_ADC_Port->DDR|= LOWV_ADC_Pin;}

#define LOWV_ADC_CH         ADC_Channel_22
#define LOWV_ADC_CH_GROUP   ADC_Group_SlowChannels


//DCDCʹ������
#define DC_EN_Port        GPIOB
#define DC_EN_Pin         GPIO_Pin_4
#define DC_ENABLE()        DC_EN_Port -> ODR |= DC_EN_Pin
#define DC_DISABLE()       DC_EN_Port -> ODR &= ~DC_EN_Pin


//����������
#define BUZZER_Port          GPIOB
#define BUZZER_Pin           GPIO_Pin_6
#define BUZZER_TOGGLE()        BUZZER_Port->ODR ^= BUZZER_Pin
#define BUZZER_CLOSE()        BUZZER_Port->ODR &= ~BUZZER_Pin
#define BUZZER_OPEN()        BUZZER_Port->ODR |= BUZZER_Pin


//����ģ�⴮�ڴ�ӡ��
#define SIM_DEBUG_Port       GPIOB
#define SIM_DEBUG_Pin        GPIO_Pin_5

//PX��δ������
#define PB_UNUSED_Pin     (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7)
//v1.0
//#define PC_UNUSED_Pin     (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6)
//v1.1
#define PC_UNUSED_Pin     (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6)


#define BSP_ENABLE_INTERRUPT()     {rim();global_interrupt_flag = 1;}
#define BSP_DISABLE_INTERRUPT()     {sim();global_interrupt_flag = 0;}
#define BSP_SET_INTERRUPT(flag)     {if(flag) rim();else sim();global_interrupt_flag = flag;}
#define BSP_GET_INTERRUPT()        global_interrupt_flag

enum
{
    E_SYS_STA_NONE = 0, //��ʼ״̬
    E_SYS_STA_PU,  //�ϵ�״̬
    E_SYS_STA_HALT, //ͣ��״̬
    E_SYS_STA_NO_DEF, //δ����
    E_SYS_STA_WAIT_DEF,//�ȴ�ȷ�ϲ���
    E_SYS_STA_DEF,  //�Ѳ���
    E_SYS_STA_ALARM, //����״̬
};

enum
{
    E_BEEP_PLAY_NONE = 0,
    E_BEEP_PLAY_PU,  //�ϵ���ʾ���֡�
    E_BEEP_PLAY_DEF, //������ʾ
    E_BEEP_PLAY_DEF_OK,//�����ɹ���ʾ
    E_BEEP_PLAY_TIPS,//������ʾ��Ԥ������ʾ�������֡�
    E_BEEP_PLAY_LOW_PWR, //�͵�����ʾ���������֡�
    E_BEEP_PLAY_ALARM,  //����
    
    E_BEEP_PLAY_END,
};

enum
{
    E_BEEP_PLAY_STA_NONE = 0,//���У��޲���
    E_BEEP_PLAY_STA_PAUSE,    //��������ʱ�����ʱ������ͣ״̬
    E_BEEP_PLAY_STA_REPLAY,    //��������ʱ���ָ��ط�״̬
    E_BEEP_PLAY_STA_FIRST,   //��һ�β���
    E_BEEP_PLAY_STA_REPEAT,  //�ظ�����״̬
    E_BEEP_PLAY_STA_CONTINUE,//��������״̬
    E_BEEP_PLAY_STA_LAST,    //���һ�β���
};

//RTC�Զ�����ʱ������
enum
{
    E_RTC_WAKEUP_PERIOD_1MS = 1,
    E_RTC_WAKEUP_PERIOD_10MS,
    E_RTC_WAKEUP_PERIOD_1000MS,
};

/*----------------------------- Global Typedefs -----------------------------*/

/*----------------------------- External Variables --------------------------*/
//�������ڣ�����汾����ʱ��Ҫ�޸�
extern const uint8_t sys_complied_date[3];//ʹ��BCD�룬���ֻȡ��2λ


extern volatile uint8_t global_interrupt_flag;  //���ڼ�¼��ǰ�����жϱ�־

extern uint8_t sys_sta;  //���ڼ�¼��ǰ�Ĳ�����״̬

extern volatile uint8_t beep_play_type;

extern uint8_t key_val;  //���ڼ�¼����ֵ��0��δ�ͷţ�1������

/*------------------------ Global Function Prototypes -----------------------*/
/******************************************************************************
* Name     :bsp_system_clock_init 
*
* Desc     :�������õ�ϵͳʱ�Ӻ궨�壬��ʼ��CPUʱ�����ã�����BOOT_ROM���ģ���ʱ�������⣬
            ��������ģ��ʱ������Ϊ�ر�
* Param_in :sys_clk:��Ҫʹ�õ�ϵͳʱ�ӣ��μ�ö�����Ͷ���
* Param_out:
* Return   :
* Global   :
* Note     :1������Ŀ�̶�ʹ������HSI 16MHzΪʱ��Դ����ϵͳ��λ��Ĭ�ϼ�ʹ�ô�ʱ��Դ����˲�
            �ٵ���ѡ��ʱ��Դ
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/03/28, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_system_clock_init(uint32_t sys_clk);

/******************************************************************************
* Name     :bsp_sys_clock_inc 
*
* Desc     :ϵͳ�δ�ʱ��ֵ��1���ڶ�ʱ���е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/22, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_sys_clock_inc(void);

/******************************************************************************
* Name     :bsp_get_clock 
*
* Desc     :��ȡϵͳ�δ�ʱ��ֵ
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/08/22, Create this function by CenLinbo
 ******************************************************************************/
extern uint32_t bsp_get_clock(void);

/******************************************************************************
* Name     :bsp_delay_ms_by_rtc 
*
* Desc     :ʹ��RTC����ms����ʱ���ɸ��ݵ�ǰRTC��ģʽѡ���Ӧ�ķ�ʽ���Խ�ʡ����
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/17, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_delay_ms_by_rtc(uint16_t n);

/******************************************************************************
* Name     :bsp_rtc_init 
*
* Desc     :����RTCΪ�Զ�����
* Param_in :֧��1ms��10ms��1000ms
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/16, Create this function by CenLinbo
 ******************************************************************************/
void bsp_rtc_init(uint16_t wakeup_period);


/******************************************************************************
* Name     :bsp_rtc_deinit 
*
* Desc     :����RTC
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/16, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_rtc_deinit(void);


/******************************************************************************
* Name     :bsp_rtc_it_handler 
*
* Desc     :RTC�Զ������ж��е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/17, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_rtc_it_handler(void);

extern void bsp_beep_play(uint8_t type);
extern void bsp_beep_play_it(void);

/******************************************************************************
* Name     :bsp_beep_stop 
*
* Desc     :�������ģʽʱ���رշ�������ص�����ͱ���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_beep_stop(void);

/******************************************************************************
* Name     :bsp_key_it 
*
* Desc     :�����ж��е���
* Param_in :
* Param_out:
* Return   :
* Global   :
* Note     :
* Author   :CenLinbo
* -------------------------------------
* Log     :2020/09/22, Create this function by CenLinbo
 ******************************************************************************/
extern void bsp_key_it(void);

#if (SIM_UART_PRINTF_EN)
extern void sim_uart_printf(uint8_t data);
extern void sim_uart_printf_it(uint8_t data);

extern void sim_printf_string(uint8_t *str);
extern void sim_printf_hex(uint8_t data);

#else
#define sim_uart_printf(N)
#define sim_uart_printf_it(N)
#define sim_printf_string(N)
#define sim_printf_hex(N)
#endif

#endif //_MAIN_H_
