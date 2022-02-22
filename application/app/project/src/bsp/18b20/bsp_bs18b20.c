/**
 * @file bsp_bs18b20.c
 * @author zgg
 * @brief
 * @version 1.0.0
 * @date 2022-01-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "gd32f30x.h"
#include "systick.h"
#include "bsp_bs18b20.h"
#include  <os.h>

#define DS18B20_CLK RCU_GPIOD
#define DS18B20_PORT GPIOD
#define DS18B20_PIN GPIO_PIN_7

#define DS18B20_DQ_1 gpio_bit_write(DS18B20_PORT, DS18B20_PIN, SET)
#define DS18B20_DQ_0 gpio_bit_write(DS18B20_PORT, DS18B20_PIN, RESET)
#define DS18B20_DQ_IN() gpio_input_bit_get(DS18B20_PORT, DS18B20_PIN)

static void delay_us(unsigned int data)
{
    unsigned int i, j;

    for (i = 28; i > 0; i--)
    {
        for (j = data; j > 0; j--)
        {
            ;
        }
    }
}

/**
 * @brief 配置DS18B20用到的I/O口
 *
 */
static void DS18B20_GPIO_Config(void)
{
    /* enable GPO clock */
    rcu_periph_clock_enable(DS18B20_CLK);

    gpio_init(DS18B20_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS18B20_PIN);
}

/**
 * @brief 使DS18B20-DATA引脚变为输出模式
 *
 */
static void DS18B20_Mode_Out_PP(void)
{
    gpio_init(DS18B20_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS18B20_PIN);
}

/**
 * @brief 使DS18B20-DATA引脚变为输入模式
 *
 */
static void DS18B20_Mode_IPU(void)
{
    gpio_init(DS18B20_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, DS18B20_PIN);
}

/*
 *主机给从机发送复位脉冲
 */
static void DS18B20_Rst(void)
{
    OS_ERR err;
    __disable_irq();
    OSSchedLock(&err);
    /* 主机设置为推挽输出 */
    DS18B20_Mode_Out_PP();

    DS18B20_DQ_0;
    /* 主机至少产生480us的低电平复位信号 */
    delay_us(750);

    /* 主机在产生复位信号后，需将总线拉高 */
    DS18B20_DQ_1;

    /*从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲*/
    delay_us(15);
 
    OSSchedUnlock(&err);
    __enable_irq(); 
}

/**
 * @brief 检测从机给主机返回的存在脉冲
 *
 * @return uint8_t  0 success 1 fail
 */
static uint8_t DS18B20_Presence(void)
{
    uint8_t pulse_time = 0;

    /* 主机设置为上拉输入 */
    DS18B20_Mode_IPU();

    /* 等待存在脉冲的到来，存在脉冲为一个60~240us的低电平信号
     * 如果存在脉冲没有来则做超时处理，从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
     */
    while (DS18B20_DQ_IN() && pulse_time < 100)
    {
        pulse_time++;
        delay_us(1);
    }
    /* 经过100us后，存在脉冲都还没有到来*/
    if (pulse_time >= 100)
        return 1;
    else
        pulse_time = 0;

    /* 存在脉冲到来，且存在的时间不能超过240us */
    while (!DS18B20_DQ_IN() && pulse_time < 240)
    {
        pulse_time++;
        delay_us(1);
    }
    if (pulse_time >= 240)
        return 1;
    else
        return 0;
}

/**
 * @brief  DS18B20 初始化函数
 * @param  无
 * @retval 无
 */
uint8_t DS18B20_Init(void)
{
    DS18B20_GPIO_Config();

    DS18B20_DQ_1;

    DS18B20_Rst();

    return DS18B20_Presence();
}

/**
 * @brief 从DS18B20读取一个bit
 *
 * @return uint8_t
 */
static uint8_t DS18B20_ReadBit(void)
{
    uint8_t dat;

    /* 读0和读1的时间至少要大于60us */
    DS18B20_Mode_Out_PP();
    /* 读时间的起始：必须由主机产生 >1us <15us 的低电平信号 */
    DS18B20_DQ_0;
    delay_us(10);

    /* 设置成输入，释放总线，由外部上拉电阻将总线拉高 */
    DS18B20_Mode_IPU();
    // delay_us(2);

    if (DS18B20_DQ_IN() == SET)
        dat = 1;
    else
        dat = 0;

    /* 这个延时参数请参考时序图 */
    delay_us(45);

    return dat;
}

/**
 * @brief 从DS18B20读一个字节，低位先行
 *
 * @return uint8_t
 */
static uint8_t DS18B20_ReadByte(void)
{
    uint8_t i, j, dat = 0;
    OS_ERR err;
    __disable_irq();
    OSSchedLock(&err);
    for (i = 0; i < 8; i++)
    {
        j = DS18B20_ReadBit();
        dat = (dat) | (j << i);
    }
    OSSchedUnlock(&err);
    __enable_irq();
    return dat;
}

/**
 * @brief
 *
 * @param[in] dat 写一个字节到DS18B20，低位先行
 */
static void DS18B20_WriteByte(uint8_t dat)
{
    uint8_t i, testb;

    OS_ERR err;
    __disable_irq();
    OSSchedLock(&err);

    DS18B20_Mode_Out_PP();

    for (i = 0; i < 8; i++)
    {
        testb = dat & 0x01;
        dat = dat >> 1;
        /* 写0和写1的时间至少要大于60us */
        if (testb)
        {
            DS18B20_DQ_0;
            /* 1us < 这个延时 < 15us */
            delay_us(8);

            DS18B20_DQ_1;
            delay_us(58);
        }
        else
        {
            DS18B20_DQ_0;
            /* 60us < Tx 0 < 120us */
            delay_us(70);

            DS18B20_DQ_1;
            /* 1us < Trec(恢复时间) < 无穷大*/
            delay_us(2);
        }
    }
    OSSchedUnlock (&err);
    __enable_irq();
}

/**
 * @brief  跳过匹配 DS18B20 ROM
 * @param  无
 * @retval 无
 */
static void DS18B20_SkipRom(void)
{
    DS18B20_Rst();

    DS18B20_Presence();

    DS18B20_WriteByte(0XCC); /* 跳过 ROM */
}

/*
 * 存储的温度是16 位的带符号扩展的二进制补码形式
 * 当工作在12位分辨率时，其中5个符号位，7个整数位，4个小数位
 *
 *         |---------整数----------|-----小数 分辨率 1/(2^4)=0.0625----|
 * 低字节  | 2^3 | 2^2 | 2^1 | 2^0 | 2^(-1) | 2^(-2) | 2^(-3) | 2^(-4) |
 *
 *
 *         |-----符号位：0->正  1->负-------|-----------整数-----------|
 * 高字节  |  s  |  s  |  s  |  s  |    s   |   2^6  |   2^5  |   2^4  |
 *
 *
 * 温度 = 符号位 + 整数 + 小数*0.0625
 */
/**
 * @brief  在跳过匹配 ROM 情况下获取 DS18B20 温度值
 * @param  无
 * @retval 温度值
 */
float DS18B20_GetTemp_SkipRom(void)
{
    uint8_t tpmsb, tplsb;
    short s_tem;
    float f_tem = 0.1;

    DS18B20_SkipRom();
    DS18B20_WriteByte(0X44); /* 开始转换 */

    DS18B20_SkipRom();
    DS18B20_WriteByte(0XBE); /* 读温度值 */

    tplsb = DS18B20_ReadByte();
    tpmsb = DS18B20_ReadByte();

    s_tem = tpmsb << 8;
    s_tem = s_tem | tplsb;

    if (s_tem < 0) /* 负温度 */
        f_tem = (~s_tem + 1) * 0.0625;
    else
        f_tem = s_tem * 0.0625;

    return f_tem;
}
