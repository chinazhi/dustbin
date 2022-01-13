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
#define DS18B20_IN_TYPE           GPIO_MODE_IPD

#define DS18B20_OUTPUT_MODE()     gpio_init(DS18B20_PORT, DS18B20_OUT_TYPE, GPIO_OSPEED_50MHZ, DS18B20_PIN)
#define DS18B20_INPUT_MODE()      gpio_init(DS18B20_PORT, DS18B20_IN_TYPE, GPIO_OSPEED_50MHZ, DS18B20_PIN);

#define DS18B20_OUT_HIGH()        gpio_bit_write(DS18B20_PORT, DS18B20_PIN, SET)
#define DS18B20_OUT_LOW()         gpio_bit_write(DS18B20_PORT, DS18B20_PIN, RESET)

#define DS18B20_IN_READ()         gpio_input_bit_get(DS18B20_PORT, DS18B20_PIN)

void delay_us(int data)
{
    int i = 0;
    for (i = 0; i < data; i++)
    {
        //_nop_();
    }
    return;
}

/**
 * @brief DS18B20 BSP init
 * 
 * @return int 0 error 1 success
 */
int ds18b20_init(void)
{
    int i = 0;

    /* enable GPO clock */
    rcu_periph_clock_enable(DS18B20_CLK); 
    DS18B20_OUTPUT_MODE(); // 进入输出模式
    DS18B20_OUT_LOW();     // 输出0 拉低 发送复位脉冲
    delay_us(642);         // 642 延时（>480us <960us)
    DS18B20_OUT_HIGH();    // 输出1 拉高 Mark1
    delay_us(30);          // 等待（15~60us)
    DS18B20_INPUT_MODE();  // 进入输入模式
    while (DS18B20_IN_READ() == 1)
    { //收到低电平则存在，否则循环5ms告诉不存在,应答是在60-240微秒
        delay_1ms(1);
        i++;
        if (i > 5)
        {
            return 0; //不存在返回0 Mark2
        }
    }
    return 1; //存在返回1 时序图看出从Mark1到Mark2之间不能多于300微秒，否则检测出错。
}

/**
 * @brief write byte
 * 
 * @param[in] dat 
 */
void ds18b20_write_byte(unsigned char dat)
{
    int i;
    DS18B20_OUTPUT_MODE();
    for (i = 0; i < 8; i++)
    {
        DS18B20_OUT_LOW();     // 进入写时序拉低
        delay_us(15);          // 写入时先拉低大于15微秒
        if(dat & 0x01)         // 写入0或1
        {
            DS18B20_OUT_HIGH();
        }
        else
        {
            DS18B20_OUT_LOW();
        }
        delay_us(60);          // 写入1或0时都需要至少60微秒的间隙
        DS18B20_OUT_HIGH();    // 再拉高恢复可写状态
        dat >>= 1;             // 一共8位右移一位把下一位数据放在最右边
    }
}

/**
 * @brief read byte
 * 
 * @return unsigned char 
 */
unsigned char ds18b20_read_byte(void)
{
    int j;
    unsigned char dat, byte;
    for (j = 8; j > 0; j--)
    {
        DS18B20_OUTPUT_MODE();
        DS18B20_OUT_LOW();    // 看读时序
        delay_us(1);          // 拉低延迟1微秒
        DS18B20_OUT_HIGH();
        delay_us(10);         // 进入读的准备阶段10微秒
        DS18B20_INPUT_MODE();
        dat = DS18B20_IN_READ();
        byte = (byte >> 1) | (dat << 7);
        delay_us(45);         // 延迟45微秒读完1位
        DS18B20_OUTPUT_MODE();
        DS18B20_OUT_HIGH();   // 继续拉高为读下一位做准备
    }
    return byte;
}

void ds18b20_change_temp()
{
    ds18b20_init();
    delay_1ms(1);
    ds18b20_write_byte(0xcc); //跳过ROM直接发送温度转换命令
    ds18b20_write_byte(0x44); //发送指令RAM设为0x44为温度变换
}

void ds18b20_read_temp_com()
{
    ds18b20_init();
    delay_1ms(1);
    ds18b20_write_byte(0xcc); //跳过ROM直接发送温度转换命令
    ds18b20_write_byte(0xbe); //发送指令RAM设为0xBE为读暂时寄存器
}

/**
 * @brief read temp
 *
 * @return float
 */
float ds18b20_read_temp(void)
{
    int temp = 0;
    float f_temp = 0;
    unsigned char temp_low, temp_high;
    ds18b20_change_temp();
    ds18b20_read_temp_com();
    temp_low = ds18b20_read_byte();  //读低8位数据
    temp_high = ds18b20_read_byte(); //读高8位数据

    temp = temp_high;
    temp <<= 8;
    temp |= temp_low;  // 拼接为16位数据

    if (temp < 0)      // 负温度
        f_temp = (~temp + 1) * 0.0625;
    else
        f_temp = temp * 0.0625;

    return f_temp;
}
