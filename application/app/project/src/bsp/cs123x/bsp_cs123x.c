/**
 * @file cs1238.c
 * @author zgg 
 * @brief  ��ģת��оƬcs1238 ���� 
 *         �Ǳ�׼SPI���� Chipsea �Զ����˫��ͨ�Žӿڡ�
 *         ��Ҫ����ʹ�� GPIO ģ��ʱ�� 
 * @version 1.0.0
 * @date 2021-06-17
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "includes.h"
#include "systick.h"

#define CS123X_CLK1                 RCU_GPIOE
#define CS123X_DOUT_PORT1           GPIOE
#define CS123X_DOUT_PIN1            GPIO_PIN_0
#define CS123X_DOUT_VALUE1          BSP_GPO_HI
#define CS123X_DOUT_OUT_TYPE1       GPIO_MODE_OUT_PP
#define CS123X_DOUT_IN_TYPE1        GPIO_MODE_IPU

#define CS123X_SCLK_PORT1           GPIOE
#define CS123X_SCLK_PIN1            GPIO_PIN_1
#define CS123X_SCLK_VALUE1          BSP_GPO_HI
#define CS123X_SCLK_OUT_TYPE1       GPIO_MODE_OUT_PP
#define CS123X_SCLK_IN_TYPE1        GPIO_MODE_IPU

//#define CS1238_DEBUG 1 // �ͷ�ע�Ϳ�����ӡ

#define DOUT_GPIO_PIN 1 ///< cs1238 ���ݽ�
#define SCLK_GPIO_PIN 2 ///< ģ��ʱ�ӽ�

/* ʱ�ӽ� ���ģʽ�� �ߵͿ��� */
#define IO_CLK_AD_H() gpio_bit_write(CS123X_SCLK_PORT1, CS123X_SCLK_PIN1, SET)
#define IO_CLK_AD_L() gpio_bit_write(CS123X_SCLK_PORT1, CS123X_SCLK_PIN1, RESET)

/* ���ݽ� ���ģʽ�� �ߵͿ��� */
#define IO_DATA_AD_H() gpio_bit_write(CS123X_DOUT_PORT1, CS123X_DOUT_PIN1, SET)
#define IO_DATA_AD_L() gpio_bit_write(CS123X_DOUT_PORT1, CS123X_DOUT_PIN1, RESET)

/* ���ݽ� ����ģʽ�� ��ȡ����ֵ */
#define GET_IO_AD_DATA gpio_input_bit_get(CS123X_DOUT_PORT1, CS123X_DOUT_PIN1)

/* ȫ�ֳ������� */
cs1238_load_info_t g_load_info;

/**
 * @brief �������ݹ�������ģʽΪ ��������
 * 
 */
void set_dout_mode_in(void)
{
    gpio_init(CS123X_DOUT_PORT1, CS123X_DOUT_IN_TYPE1, GPIO_OSPEED_50MHZ, CS123X_DOUT_PIN1);
}

/**
 * @brief �������ݹ�������ģʽΪ ���ģʽ
 * 
 */
void set_dout_mode_out(void)
{
    gpio_init(CS123X_DOUT_PORT1, CS123X_DOUT_OUT_TYPE1, GPIO_OSPEED_50MHZ, CS123X_DOUT_PIN1);
}

/**
 * @brief ����ʱ������Ϊ���ģʽ ����
 * 
 */
void set_sclk_mode_out(void)
{
    gpio_init(CS123X_SCLK_PORT1, CS123X_SCLK_OUT_TYPE1, GPIO_OSPEED_50MHZ, CS123X_SCLK_PIN1);
    IO_CLK_AD_L();
}

/**
 * @brief ͨ��ʱ�� SCL ��ƽ��ʱʱ��
 *        ��Ҫ<100��s�� ������󴥷���������ģʽ�� һ�㽨�� SCL=2��s~15��s
 */
void sclk_delay(void)
{
    delay_1us(3);
}

/**
 * @brief cs1238 ʱ��ʱ��
 */
void cs1238_clock(void)
{
    IO_CLK_AD_H();
    sclk_delay();
    IO_CLK_AD_L();
    sclk_delay();
}

/**
 * @brief ��ȡCS1238��AD����
 * 
 * @return int32_t �ɹ����� ��ֵ ��ʱ���� 1
 */
int32_t read_cs1238_ad_data(void)
{
    static uint16_t bit_cout = 0;
    static int32_t data_temp = 0;

  /* ��ʼ�����ȡ����״̬ */
    set_dout_mode_in();
    IO_CLK_AD_L(); //ʱ������

    while (GET_IO_AD_DATA != 0) //�ȴ�оƬ׼���� Ϊ�͵�ƽ
    {
        delay_1ms(1); //��ʱ��ѯ�ķ�����Ҫ���̲�ѯ��� ����:1ms~5ms
        bit_cout++;
        if (bit_cout > 300) //�����300 ������ʱ����Ҫ�����ݽ���ʱ��Ϊ300ms
        {
            bit_cout = 0;
            set_dout_mode_out();
            IO_CLK_AD_H();  // CLK 1
            IO_DATA_AD_H(); // OUT 1
            return 1;     // ��ʱ�˳� 
        }
    }

    /* 1: clk1 ~ clk24 ADC����*/
    data_temp = 0;
    for (bit_cout = 0; bit_cout < 24; bit_cout++)
    {
        data_temp <<= 1;    //����1λ׼���������� ��ʼĬ��0
        cs1238_clock();     //��һ����ʱ��
        if (GET_IO_AD_DATA != 0) //����ֵ���ۼ�
        {
            data_temp++;
        }
    }
 
    /* 2 ~ 7: clk25 ~ clk45 ����ʱ��Ϊ 46 �� clocks �����Ϊ��д�Ĵ���*/
    for (bit_cout = 0; bit_cout < 22; bit_cout++)
    {
        cs1238_clock(); //��һ����ʱ��
    }

    if (data_temp & 0x00800000) // �ж��Ǹ��� ���λ24λ�Ƿ���λ
    {
        return -(((~data_temp) & 0x007FFFFF) + 1); // �����Դ��
    }

   return data_temp; // �����Ĳ������Դ��
}

/**
 * @brief ����CS1238�Ĵ���
 * 
 * @param[in] ad_reg  ͨ�� 0~1bit | PGA 2~3bit | ���� 4~5bit | REF 6bit | ����λ 0 7bit
 * @return int �ɹ� 0 ��ʱ 1
 */
int set_cs1238_config(uint8_t ad_reg)
{
    static uint16_t bit_cout = 0;
    static uint8_t reg_temp = 0x00;

    /* ��ʼ�����ȡ����״̬ */
    set_dout_mode_in();
    IO_CLK_AD_L(); //ʱ������

    while (GET_IO_AD_DATA != 0) //�ȴ�оƬ׼���� Ϊ�͵�ƽ
    {
        delay_1ms(1); //��ʱ��ѯ�ķ�����Ҫ���̲�ѯ��� ����:1ms~5ms
        bit_cout++;
        if (bit_cout > 300)
        {
            bit_cout = 0;
            set_dout_mode_out();
            IO_CLK_AD_H();  // CLK 1
            IO_DATA_AD_H(); // OUT 1
            return 1;       // ��ʱ�˳�
        }
    }

    /* 1 ~ 4: clk1-clk29 */
    for (bit_cout = 0; bit_cout < 29; bit_cout++)
    {
        cs1238_clock(); //��һ����ʱ��
    }

    /* 5: clk30 - clk36 ����д������ */
    set_dout_mode_out();
    reg_temp = 0xCA;                 //�����Ϊ 7bits (д0x65)����1λ
    for (bit_cout = 0; bit_cout < 7; bit_cout++)
    {
        if (reg_temp & 0x80) //MSB 
        {
            IO_DATA_AD_H();
        }
        else
        {
            IO_DATA_AD_L();
        }
        reg_temp = reg_temp << 1;
        cs1238_clock();
    }

    /* 6: clk37  д ��� */
    cs1238_clock();

    /* 7: clk38 ~ clk45 д��Ĵ���ֵ */
    reg_temp = ad_reg;
    for (bit_cout = 0; bit_cout < 8; bit_cout++)
    {
        if (reg_temp & 0x80) //MSB
        {
            IO_DATA_AD_H();
        }
        else
        {
            IO_DATA_AD_L();
        }
        reg_temp = reg_temp << 1;
        cs1238_clock();
    }

    /* 8: clk46 */
    cs1238_clock();

    return 0;
}

/**
 * @brief ��ȡcs1238 ����
 * 
 * @return int �ɹ����� ��ֵ ��ʱ 1
 */
int read_cs1238_config(void)
{
    static uint16_t bit_cout = 0;
    static uint8_t reg_temp = 0x00;
    static uint8_t cfg_temp = 0x00;
    /* ��ʼ�����ȡ����״̬ */
    set_dout_mode_in();
    IO_CLK_AD_L(); //ʱ������

    while (GET_IO_AD_DATA != 0) //�ȴ�оƬ׼���� Ϊ�͵�ƽ
    {
        delay_1ms(1); //��ʱ��ѯ�ķ�����Ҫ���̲�ѯ��� ����:1ms~5ms
        bit_cout++;
        if (bit_cout > 300)
        {
            bit_cout = 0;
            set_dout_mode_out();
            IO_CLK_AD_H();  // CLK 1
            IO_DATA_AD_H(); // OUT 1
            return 1;       // ��ʱ�˳�
        }
    }

    /* 1 : clk1-clk27  ������ ������AD���� */
    for (bit_cout = 0; bit_cout < 27; bit_cout++)
    {
        cs1238_clock(); //��һ����ʱ��
    }

    /* 4: clk28-clk29  ��� ׼��д������ */
    set_dout_mode_out();
    cs1238_clock();
    cs1238_clock();

    /* 5: clk30 - clk36 ����д������ */
    reg_temp = 0xAC; //�����Ϊ 7bits (д0x65)����1λ
    for (bit_cout = 0; bit_cout < 7; bit_cout++)
    {
        if (reg_temp & 0x80) //MSB 
        {
            IO_DATA_AD_H();
        }
        else
        {
            IO_DATA_AD_L();
        }
        reg_temp = reg_temp << 1;
        cs1238_clock();
    }

    /* 6: clk37  ���� ׼������ */
    set_dout_mode_in();
    cs1238_clock();

    /* 7: clk38 ~ clk45 ����Ĵ���ֵ */
    for (bit_cout = 0; bit_cout < 8; bit_cout++)
    {
        cfg_temp <<= 1;    //����1λ׼���������� ��ʼĬ��0
        cs1238_clock();     //��һ����ʱ��
        if (GET_IO_AD_DATA != 0) //����ֵ���ۼ�
        {
            cfg_temp++;
        }
    }
 
    /* 8: clk46 */
    cs1238_clock();

    return cfg_temp;
}


// �޷�ƽ���˲���
// #define FILTER_MAX 10000    // �޷��˲����ֵ
// #define FILTER_N 11         // ƽ���˲����� 

// int32_t filter_adc1_init = 0;         // ��ʼ�׶��������
// int32_t filter_adc1_buf[FILTER_N];    // ADC1�ĵ���ƽ���˲�����

// int32_t filter_adc2_init = 0;
// int32_t filter_adc2_buf[FILTER_N];

// /**
//  * @brief �޷�ƽ���˲� ADC1
//  * 
//  * @param[in] read_data 
//  * @return int ���� �˲������ֵ
//  */
// int filter_adc1(int32_t read_data)
// {
//   int i;
//   int filter_sum = 0;
//   filter_adc1_buf[FILTER_N - 1] = read_data;
//   if ((g_load_info.sensor_state == 0) && (right_adc1_num >= CS1238_RIGHT_NUM))
//   {
//     if (((filter_adc1_buf[FILTER_N - 1] - filter_adc1_buf[FILTER_N - 2]) > FILTER_MAX) || ((filter_adc1_buf[FILTER_N - 2] - filter_adc1_buf[FILTER_N - 1]) > FILTER_MAX))
//     {
//       filter_adc1_buf[FILTER_N - 1] = filter_adc1_buf[FILTER_N - 2];
//     }
//   }
//   else
//   {
//     printf("close filter adc1\n");
//   }

//   for (i = 0; i < FILTER_N - 1; i++)
//   {
//     filter_adc1_buf[i] = filter_adc1_buf[i + 1]; // �����������ƣ���λ�Ե�
//     filter_sum += filter_adc1_buf[i];
//   }
//   return (int)filter_sum / (FILTER_N - 1);
// }

// /**
//  * @brief �޷�ƽ���˲� ADC2
//  * 
//  * @param[in] read_data 
//  * @return int ���� �˲������ֵ
//  */
// int filter_adc2(int32_t read_data)
// {
//   int i;
//   int filter_sum = 0;
//   filter_adc2_buf[FILTER_N - 1] = read_data;
//   if ((g_load_info.sensor_state == 0) && (right_adc2_num >= CS1238_RIGHT_NUM))
//   {
//     if (((filter_adc2_buf[FILTER_N - 1] - filter_adc2_buf[FILTER_N - 2]) > FILTER_MAX) || ((filter_adc2_buf[FILTER_N - 2] - filter_adc2_buf[FILTER_N - 1]) > FILTER_MAX))
//     {
//       filter_adc2_buf[FILTER_N - 1] = filter_adc2_buf[FILTER_N - 2];
//     }
//   }
//   else
//   {
//     printf("close filter adc2\n");
//   }

//   for (i = 0; i < FILTER_N - 1; i++)
//   {
//     filter_adc2_buf[i] = filter_adc2_buf[i + 1]; // �����������ƣ���λ�Ե�
//     filter_sum += filter_adc2_buf[i];
//   }
//   return (int)filter_sum / (FILTER_N - 1);
// }

/**
 * @brief ��ʼ��cs1238 
 * 
 */
void new_cs1238_init(void)
{
    rcu_periph_clock_enable(CS123X_CLK1);

    // ���� cs1238ʱ������ 
    set_sclk_mode_out();

    // ���� cs1238���ݽ�  
    set_dout_mode_in();

    // �ϵ����������ʱ��
    delay_1ms(100);

    // ������״̬λ ��ʼ������
    g_load_info.sensor_state = 0;

    // ��ʼ���� ͨ��A ��A����
    set_cs1238_config(CHANNEL_A_CONFIG);
 
    #ifdef CS1238_DEBUG
    printf("cs1238 init ok\n");
    #endif
}

/**
 * @brief ˳���ȡCS1238 
 */
void polling_read_cs1238_data(void)
{
    int data_temp = 0;
    data_temp = read_cs1238_ad_data();
    printf("cs123x adc: %d\n", data_temp);
}

/**
 * @brief make cs1238 sleep
 * 
 */
void sleep_cs1238(void)
{
    IO_CLK_AD_H();
}
