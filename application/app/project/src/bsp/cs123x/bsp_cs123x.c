/**
 * @file cs1238.c
 * @author zgg 
 * @brief  数模转化芯片cs1238 程序 
 *         非标准SPI，是 Chipsea 自定义的双向通信接口。
 *         需要主控使用 GPIO 模拟时序 
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

//#define CS1238_DEBUG 1 // 释放注释开启打印

#define DOUT_GPIO_PIN 1 ///< cs1238 数据脚
#define SCLK_GPIO_PIN 2 ///< 模拟时钟脚

/* 时钟脚 输出模式下 高低控制 */
#define IO_CLK_AD_H() gpio_bit_write(CS123X_SCLK_PORT1, CS123X_SCLK_PIN1, SET)
#define IO_CLK_AD_L() gpio_bit_write(CS123X_SCLK_PORT1, CS123X_SCLK_PIN1, RESET)

/* 数据脚 输出模式下 高低控制 */
#define IO_DATA_AD_H() gpio_bit_write(CS123X_DOUT_PORT1, CS123X_DOUT_PIN1, SET)
#define IO_DATA_AD_L() gpio_bit_write(CS123X_DOUT_PORT1, CS123X_DOUT_PIN1, RESET)

/* 数据脚 输入模式下 读取的数值 */
#define GET_IO_AD_DATA gpio_input_bit_get(CS123X_DOUT_PORT1, CS123X_DOUT_PIN1)

/* 全局称重数据 */
cs1238_load_info_t g_load_info;

/**
 * @brief 设置数据工作引脚模式为 输入上拉
 * 
 */
void set_dout_mode_in(void)
{
    gpio_init(CS123X_DOUT_PORT1, CS123X_DOUT_IN_TYPE1, GPIO_OSPEED_50MHZ, CS123X_DOUT_PIN1);
}

/**
 * @brief 设置数据工作引脚模式为 输出模式
 * 
 */
void set_dout_mode_out(void)
{
    gpio_init(CS123X_DOUT_PORT1, CS123X_DOUT_OUT_TYPE1, GPIO_OSPEED_50MHZ, CS123X_DOUT_PIN1);
}

/**
 * @brief 设置时钟引脚为输出模式 拉低
 * 
 */
void set_sclk_mode_out(void)
{
    gpio_init(CS123X_SCLK_PORT1, CS123X_SCLK_OUT_TYPE1, GPIO_OSPEED_50MHZ, CS123X_SCLK_PIN1);
    IO_CLK_AD_L();
}

/**
 * @brief 通信时序 SCL 电平延时时间
 *        需要<100μs， 否则会误触发进入休眠模式， 一般建议 SCL=2μs~15μs
 */
void sclk_delay(void)
{
    delay_1us(3);
}

/**
 * @brief cs1238 时序时钟
 */
void cs1238_clock(void)
{
    IO_CLK_AD_H();
    sclk_delay();
    IO_CLK_AD_L();
    sclk_delay();
}

/**
 * @brief 读取CS1238的AD数据
 * 
 * @return int32_t 成功返回 数值 超时返回 1
 */
int32_t read_cs1238_ad_data(void)
{
    static uint16_t bit_cout = 0;
    static int32_t data_temp = 0;

  /* 开始进入读取数据状态 */
    set_dout_mode_in();
    IO_CLK_AD_L(); //时钟拉低

    while (GET_IO_AD_DATA != 0) //等待芯片准备好 为低电平
    {
        delay_1ms(1); //定时查询的方法需要缩短查询间隔 例如:1ms~5ms
        bit_cout++;
        if (bit_cout > 300) //这里的300 是最慢时所需要的数据建立时间为300ms
        {
            bit_cout = 0;
            set_dout_mode_out();
            IO_CLK_AD_H();  // CLK 1
            IO_DATA_AD_H(); // OUT 1
            return 1;     // 超时退出 
        }
    }

    /* 1: clk1 ~ clk24 ADC数据*/
    data_temp = 0;
    for (bit_cout = 0; bit_cout < 24; bit_cout++)
    {
        data_temp <<= 1;    //左移1位准备接受数据 初始默认0
        cs1238_clock();     //给一周期时钟
        if (GET_IO_AD_DATA != 0) //有数值则累加
        {
            data_temp++;
        }
    }
 
    /* 2 ~ 7: clk25 ~ clk45 完整时序为 46 个 clocks 后面的为读写寄存器*/
    for (bit_cout = 0; bit_cout < 22; bit_cout++)
    {
        cs1238_clock(); //给一周期时钟
    }

    if (data_temp & 0x00800000) // 判断是负数 最高位24位是符号位
    {
        return -(((~data_temp) & 0x007FFFFF) + 1); // 补码变源码
    }

   return data_temp; // 正数的补码就是源码
}

/**
 * @brief 设置CS1238寄存器
 * 
 * @param[in] ad_reg  通道 0~1bit | PGA 2~3bit | 速率 4~5bit | REF 6bit | 保留位 0 7bit
 * @return int 成功 0 超时 1
 */
int set_cs1238_config(uint8_t ad_reg)
{
    static uint16_t bit_cout = 0;
    static uint8_t reg_temp = 0x00;

    /* 开始进入读取数据状态 */
    set_dout_mode_in();
    IO_CLK_AD_L(); //时钟拉低

    while (GET_IO_AD_DATA != 0) //等待芯片准备好 为低电平
    {
        delay_1ms(1); //定时查询的方法需要缩短查询间隔 例如:1ms~5ms
        bit_cout++;
        if (bit_cout > 300)
        {
            bit_cout = 0;
            set_dout_mode_out();
            IO_CLK_AD_H();  // CLK 1
            IO_DATA_AD_H(); // OUT 1
            return 1;       // 超时退出
        }
    }

    /* 1 ~ 4: clk1-clk29 */
    for (bit_cout = 0; bit_cout < 29; bit_cout++)
    {
        cs1238_clock(); //给一周期时钟
    }

    /* 5: clk30 - clk36 发送写命令字 */
    set_dout_mode_out();
    reg_temp = 0xCA;                 //命令长度为 7bits (写0x65)左移1位
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

    /* 6: clk37  写 输出 */
    cs1238_clock();

    /* 7: clk38 ~ clk45 写入寄存器值 */
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
 * @brief 读取cs1238 配置
 * 
 * @return int 成功返回 数值 超时 1
 */
int read_cs1238_config(void)
{
    static uint16_t bit_cout = 0;
    static uint8_t reg_temp = 0x00;
    static uint8_t cfg_temp = 0x00;
    /* 开始进入读取数据状态 */
    set_dout_mode_in();
    IO_CLK_AD_L(); //时钟拉低

    while (GET_IO_AD_DATA != 0) //等待芯片准备好 为低电平
    {
        delay_1ms(1); //定时查询的方法需要缩短查询间隔 例如:1ms~5ms
        bit_cout++;
        if (bit_cout > 300)
        {
            bit_cout = 0;
            set_dout_mode_out();
            IO_CLK_AD_H();  // CLK 1
            IO_DATA_AD_H(); // OUT 1
            return 1;       // 超时退出
        }
    }

    /* 1 : clk1-clk27  读配置 不关心AD数据 */
    for (bit_cout = 0; bit_cout < 27; bit_cout++)
    {
        cs1238_clock(); //给一周期时钟
    }

    /* 4: clk28-clk29  输出 准备写命令字 */
    set_dout_mode_out();
    cs1238_clock();
    cs1238_clock();

    /* 5: clk30 - clk36 发送写命令字 */
    reg_temp = 0xAC; //命令长度为 7bits (写0x65)左移1位
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

    /* 6: clk37  输入 准备接收 */
    set_dout_mode_in();
    cs1238_clock();

    /* 7: clk38 ~ clk45 读入寄存器值 */
    for (bit_cout = 0; bit_cout < 8; bit_cout++)
    {
        cfg_temp <<= 1;    //左移1位准备接受数据 初始默认0
        cs1238_clock();     //给一周期时钟
        if (GET_IO_AD_DATA != 0) //有数值则累加
        {
            cfg_temp++;
        }
    }
 
    /* 8: clk46 */
    cs1238_clock();

    return cfg_temp;
}


// 限幅平均滤波法
// #define FILTER_MAX 10000    // 限幅滤波最大值
// #define FILTER_N 11         // 平均滤波容量 

// int32_t filter_adc1_init = 0;         // 开始阶段数据填充
// int32_t filter_adc1_buf[FILTER_N];    // ADC1的递推平滑滤波数组

// int32_t filter_adc2_init = 0;
// int32_t filter_adc2_buf[FILTER_N];

// /**
//  * @brief 限幅平均滤波 ADC1
//  * 
//  * @param[in] read_data 
//  * @return int 返回 滤波后的数值
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
//     filter_adc1_buf[i] = filter_adc1_buf[i + 1]; // 所有数据左移，低位仍掉
//     filter_sum += filter_adc1_buf[i];
//   }
//   return (int)filter_sum / (FILTER_N - 1);
// }

// /**
//  * @brief 限幅平均滤波 ADC2
//  * 
//  * @param[in] read_data 
//  * @return int 返回 滤波后的数值
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
//     filter_adc2_buf[i] = filter_adc2_buf[i + 1]; // 所有数据左移，低位仍掉
//     filter_sum += filter_adc2_buf[i];
//   }
//   return (int)filter_sum / (FILTER_N - 1);
// }

/**
 * @brief 初始化cs1238 
 * 
 */
void new_cs1238_init(void)
{
    rcu_periph_clock_enable(CS123X_CLK1);

    // 设置 cs1238时钟引脚 
    set_sclk_mode_out();

    // 设置 cs1238数据脚  
    set_dout_mode_in();

    // 上电所需最长建立时间
    delay_1ms(100);

    // 传感器状态位 初始化正常
    g_load_info.sensor_state = 0;

    // 初始设置 通道A 读A操作
    set_cs1238_config(CHANNEL_A_CONFIG);
 
    #ifdef CS1238_DEBUG
    printf("cs1238 init ok\n");
    #endif
}

/**
 * @brief 顺序读取CS1238 
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
