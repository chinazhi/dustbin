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

#define DS18B20_CLK               RCU_GPIOD
#define DS18B20_PORT              GPIOD
#define DS18B20_PIN               GPIO_PIN_7
#define DS18B20_VALUE             BSP_GPO_HI
#define DS18B20_OUT_TYPE          GPIO_MODE_OUT_PP
#define DS18B20_IN_TYPE           GPIO_MODE_IPU

#define DS18B20_OUTPUT_MODE()     gpio_init(DS18B20_PORT, DS18B20_OUT_TYPE, GPIO_OSPEED_50MHZ, DS18B20_PIN)
#define DS18B20_INPUT_MODE()      gpio_init(DS18B20_PORT, DS18B20_IN_TYPE, GPIO_OSPEED_50MHZ, DS18B20_PIN);

#define DS18B20_OUT_HIGH()        gpio_bit_write(DS18B20_PORT, DS18B20_PIN, SET)
#define DS18B20_OUT_LOW()         gpio_bit_write(DS18B20_PORT, DS18B20_PIN, RESET)

#define DS18B20_IN_READ()         gpio_input_bit_get(DS18B20_PORT, DS18B20_PIN)

// /**
//  * @brief DS18B20 BSP init
//  * 
//  * @return int 0 error 1 success
//  */
// int ds18b20_init(void)
// {
//     int i = 0;

//     /* enable GPO clock */
//     rcu_periph_clock_enable(DS18B20_CLK); 
//     DS18B20_OUTPUT_MODE(); // 进入输出模式
//     DS18B20_OUT_LOW();     // 输出0 拉低 发送复位脉冲
//     delay_1us(642);         // 642 延时（>480us <960us)
//     DS18B20_OUT_HIGH();    // 输出1 拉高 Mark1
//     delay_1us(30);          // 等待（15~60us)
//     DS18B20_INPUT_MODE();  // 进入输入模式
//     while (DS18B20_IN_READ() == 1)
//     { //收到低电平则存在，否则循环5ms告诉不存在,应答是在60-240微秒
//         delay_1us(1000);
//         i++;
//         if (i > 5)
//         {
//             return 0; //不存在返回0 Mark2
//         }
//     }
//     return 1; //存在返回1 时序图看出从Mark1到Mark2之间不能多于300微秒，否则检测出错。
// }

// /**
//  * @brief write byte
//  * 
//  * @param[in] dat 
//  */
// void ds18b20_write_byte(unsigned char dat)
// {
//     int i;
//     DS18B20_OUTPUT_MODE();
//     for (i = 0; i < 8; i++)
//     {
//         DS18B20_OUT_LOW();     // 进入写时序拉低
//         delay_1us(15);          // 写入时先拉低大于15微秒
//         if(dat & 0x01)         // 写入0或1
//         {
//             DS18B20_OUT_HIGH();
//         }
//         else
//         {
//             DS18B20_OUT_LOW();
//         }
//         delay_1us(60);          // 写入1或0时都需要至少60微秒的间隙
//         DS18B20_OUT_HIGH();    // 再拉高恢复可写状态
//         dat >>= 1;             // 一共8位右移一位把下一位数据放在最右边
//     }
// }

// /**
//  * @brief read byte
//  * 
//  * @return unsigned char 
//  */
// unsigned char ds18b20_read_byte(void)
// {
//     int j;
//     unsigned char dat, byte;
//     for (j = 8; j > 0; j--)
//     {
//         DS18B20_OUTPUT_MODE();
//         DS18B20_OUT_LOW();    // 看读时序
//         delay_1us(1);          // 拉低延迟1微秒
//         DS18B20_OUT_HIGH();
//         delay_1us(10);         // 进入读的准备阶段10微秒
//         DS18B20_INPUT_MODE();
//         dat = DS18B20_IN_READ();
//         byte = (byte >> 1) | (dat << 7);
//         delay_1us(45);         // 延迟45微秒读完1位
//         DS18B20_OUTPUT_MODE();
//         DS18B20_OUT_HIGH();   // 继续拉高为读下一位做准备
//     }
//     return byte;
// }

// void ds18b20_change_temp()
// {
//     ds18b20_init();
//     delay_1us(1000);
//     ds18b20_write_byte(0xcc); //跳过ROM直接发送温度转换命令
//     ds18b20_write_byte(0x44); //发送指令RAM设为0x44为温度变换
// }

// void ds18b20_read_temp_com()
// {
//     ds18b20_init();
//     delay_1us(1000);
//     ds18b20_write_byte(0xcc); //跳过ROM直接发送温度转换命令
//     ds18b20_write_byte(0xbe); //发送指令RAM设为0xBE为读暂时寄存器
// }

// /**
//  * @brief read temp
//  *
//  * @return float
//  */
// float ds18b20_read_temp(void)
// {
//     int temp = 0;
//     float f_temp = 0;
//     unsigned char temp_low, temp_high;
//     ds18b20_change_temp();
//     ds18b20_read_temp_com();
//     temp_low = ds18b20_read_byte();  //读低8位数据
//     temp_high = ds18b20_read_byte(); //读高8位数据

//     temp = temp_high;
//     temp <<= 8;
//     temp |= temp_low;  // 拼接为16位数据

//     if (temp < 0)      // 负温度
//         f_temp = (~temp + 1) * 0.0625;
//     else
//         f_temp = temp * 0.0625;

//     return f_temp;
// }






#define   DS18B20_DQ_1                                gpio_bit_write(DS18B20_PORT, DS18B20_PIN, SET) 
#define   DS18B20_DQ_0                                gpio_bit_write(DS18B20_PORT, DS18B20_PIN, RESET) 
#define   DS18B20_DQ_IN()                            gpio_input_bit_get(DS18B20_PORT, DS18B20_PIN)


void Delay_us(unsigned int data)
{
    unsigned int i, j;
    //CPU_SR_ALLOC();
    //OS_CRITICAL_ENTER();
    for (i = 28; i > 0; i--)
    {
        for (j = data; j > 0; j--)
        {
            ;
        }
    }
    //OS_CRITICAL_EXIT();
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
    /* 主机设置为推挽输出 */
    DS18B20_Mode_Out_PP();
    
    DS18B20_DQ_0;
    /* 主机至少产生480us的低电平复位信号 */
    Delay_us(750);
    
    /* 主机在产生复位信号后，需将总线拉高 */
    DS18B20_DQ_1;
    
    /*从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲*/
    Delay_us(15);
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
    while( DS18B20_DQ_IN() && pulse_time<100 )
    {
        pulse_time++;
        Delay_us(1);
    }    
    /* 经过100us后，存在脉冲都还没有到来*/
    if( pulse_time >=100 )
        return 1;
    else
        pulse_time = 0;
    
    /* 存在脉冲到来，且存在的时间不能超过240us */
    while( !DS18B20_DQ_IN() && pulse_time<240 )
    {
        pulse_time++;
        Delay_us(1);
    }    
    if( pulse_time >= 240 )
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
    DS18B20_GPIO_Config ();
    
    DS18B20_DQ_1;
    
    DS18B20_Rst();
    
    return DS18B20_Presence ();
    
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
    Delay_us(10);
    
    /* 设置成输入，释放总线，由外部上拉电阻将总线拉高 */
    DS18B20_Mode_IPU();
    //Delay_us(2);
    
    if( DS18B20_DQ_IN() == SET )
        dat = 1;
    else
        dat = 0;
    
    /* 这个延时参数请参考时序图 */
    Delay_us(45);
    
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
    
    for(i=0; i<8; i++) 
    {
        j = DS18B20_ReadBit();        
        dat = (dat) | (j<<i);
    }
    
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
    DS18B20_Mode_Out_PP();
    
    for( i=0; i<8; i++ )
    {
        testb = dat&0x01;
        dat = dat>>1;        
        /* 写0和写1的时间至少要大于60us */
        if (testb)
        {            
            DS18B20_DQ_0;
            /* 1us < 这个延时 < 15us */
            Delay_us(8);
            
            DS18B20_DQ_1;
            Delay_us(58);
        }        
        else
        {            
            DS18B20_DQ_0;
            /* 60us < Tx 0 < 120us */
            Delay_us(70);
            
            DS18B20_DQ_1;            
            /* 1us < Trec(恢复时间) < 无穷大*/
            Delay_us(2);
        }
    }
}

/**
 * @brief  跳过匹配 DS18B20 ROM
 * @param  无
 * @retval 无
 */
static void DS18B20_SkipRom ( void )
{
    DS18B20_Rst();       
    
    DS18B20_Presence();     
    
    DS18B20_WriteByte(0XCC);        /* 跳过 ROM */
    
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
float DS18B20_GetTemp_SkipRom ( void )
{
    uint8_t tpmsb, tplsb;
    short s_tem;
    float f_tem;
    
    
    DS18B20_SkipRom ();
    DS18B20_WriteByte(0X44);                /* 开始转换 */
    
    
    DS18B20_SkipRom ();
    DS18B20_WriteByte(0XBE);                /* 读温度值 */
    
    tplsb = DS18B20_ReadByte();         
    tpmsb = DS18B20_ReadByte(); 
    
    
    s_tem = tpmsb<<8;
    s_tem = s_tem | tplsb;
    
    if( s_tem < 0 )        /* 负温度 */
        f_tem = (~s_tem+1) * 0.0625;    
    else
        f_tem = s_tem * 0.0625;
    
    return f_tem;  
}




















