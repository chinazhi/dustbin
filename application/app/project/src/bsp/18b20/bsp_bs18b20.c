

// #include <reg51.h>

// #define uchar unsigned char

// sbit DQ = P3 ^ 6;        //数据传输线接单片机的相应的引脚
// unsigned char tempL = 0; //设全局变量
// unsigned char tempH = 0;
// unsigned int sdata;     //测量到的温度的整数部分
// unsigned char xiaoshu1; //小数第一位
// unsigned char xiaoshu2; //小数第二位
// unsigned char xiaoshu;  //两位小数
// bit fg = 1;             //温度正负标志

// void delay(unsigned char i)
// {
//     for (i; i > 0; i--)
//         ;
// }

// void delay1(uchar i)
// {
//     uchar j, k;
//     for (j = i; j > 0; j--)
//         for (k = 125; k > 0; k--)
//             ;
// }

// void Init_DS18B20(void)
// {
//     unsigned char x = 0;
//     DQ = 1;    //DQ先置高
//     delay(8);  //稍延时
//     DQ = 0;    //发送复位脉冲
//     delay(80); //延时（>480us)
//     DQ = 1;    //拉高数据线
//     delay(5);  //等待（15~60us)
//     x = DQ;    //用X的值来判断初始化有没有成功，18B20存在的话X=0，否则X=1
//     delay(20);
// }

// //读一个字节
// ReadOneChar(void) //主机数据线先从高拉至低电平1us以上，再使数据线升为高电平，从而产生读信号
// {
//     unsigned char i = 0; //每个读周期最短的持续时间为60us，各个读周期之间必须有1us以上的高电平恢复期
//     unsigned char dat = 0;
//     for (i = 8; i > 0; i--) //一个字节有8位
//     {
//         DQ = 1;
//         delay(1);
//         DQ = 0;
//         dat >>= 1;
//         DQ = 1;
//         if (DQ)
//             dat |= 0x80;
//         delay(4);
//     }
//     return (dat);
// }

// //写一个字节
// void WriteOneChar(unsigned char dat)
// {
//     unsigned char i = 0;    //数据线从高电平拉至低电平，产生写起始信号。15us之内将所需写的位送到数据线上，
//     for (i = 8; i > 0; i--) //在15~60us之间对数据线进行采样，如果是高电平就写1，低写0发生。
//     {
//         DQ = 0; //在开始另一个写周期前必须有1us以上的高电平恢复期。
//         DQ = dat & 0x01;
//         delay(5);
//         DQ = 1;
//         dat >>= 1;
//     }
//     delay(4);
// }

// //读温度值（低位放tempL;高位放tempH;）
// void ReadTemperature(void)
// {
//     Init_DS18B20();        //初始化
//     WriteOneChar(0xcc);    //跳过读序列号的操作
//     WriteOneChar(0x44);    //启动温度转换
//     delay(125);            //转换需要一点时间，延时
//     Init_DS18B20();        //初始化
//     WriteOneChar(0xcc);    //跳过读序列号的操作
//     WriteOneChar(0xbe);    //读温度寄存器（头两个值分别为温度的低位和高位）
//     tempL = ReadOneChar(); //读出温度的低位LSB
//     tempH = ReadOneChar(); //读出温度的高位MSB
//     if (tempH > 0x7f)      //最高位为1时温度是负
//     {
//         tempL = ~tempL; //补码转换，取反加一
//         tempH = ~tempH + 1;
//         fg = 0; //读取温度为负时fg=0
//     }
//     sdata = tempL / 16 + tempH * 16;           //整数部分
//     xiaoshu1 = (tempL & 0x0f) * 10 / 16;       //小数第一位
//     xiaoshu2 = (tempL & 0x0f) * 100 / 16 % 10; //小数第二位
//     xiaoshu = xiaoshu1 * 10 + xiaoshu2;        //小数两位
// }

#include "gd32f30x.h"
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


void bsp_18b20_gpo_init(void)
{
    /* enable GPO clock */
    rcu_periph_clock_enable(DS18B20_CLK);
    /* configure GPO port */
    gpio_init(DS18B20_PORT, DS18B20_OUT_TYPE, GPIO_OSPEED_50MHZ, DS18B20_PIN);
    
    gpio_bit_write(DS18B20_PORT, DS18B20_PIN, SET);

    return ;
}



int Ds18b20_Init(){
		int i=0;

        bsp_18b20_gpo_init();
        
		DS18B20_OUTPUT_MODE();//让PB7进入输出模式
		DS18B20_OUT_LOW();//输出0 拉低
		delay_us(642);//642 查看时序图得出 大于480小于960微秒
		DS18B20_OUT_HIGH();//输出1 拉高 Mark1
		delay_us(30);//查时序图得出 大于15小于60微秒
		
		DS18B20_INPUT_MODE();//让PB7进入输入模式
		while(DS18B20_IN_READ() ==1){//收到低电平则存在，否则循环5ms告诉不存在 看时序图应答是在60-240微秒 
					delay_ms(1);
					i++;
					if(i>5){
							return 0;//不存在返回0 Mark2
					}
		}
		return 1;//存在返回1
		//时序图可以看出从Mark1到Mark2之间不能多于300微秒，否则检测出错。
}


void Ds18b20WriteByte(uchar dat){
	int i;
	DS18B20_OUTPUT_MODE();
	for(i=0;i<8;i++){
		DS18B20_OUT_LOW();	//进入写时序拉低
		delay_us(15);//写入时先拉低大于15微秒 
		PBout(7)=dat&0x01;//写入0或1
		delay_us(60);//写入1或0时都需要至少60微秒的间隙 
		DS18B20_OUT_HIGH();//再拉高恢复可写状态
		dat>>=1;//一共8位右移一位把下一位数据放在最右边
	}
}
uchar Ds18b20ReadByte(){
	int j;
	uchar dat,byte;
	for(j=8;j>0;j--){
	DS18B20_OUTPUT_MODE();
	DS18B20_OUT_LOW();//看读时序
	delay_us(1);//拉低延迟1微秒
	DS18B20_OUT_HIGH();
	delay_us(10);//进入读的准备阶段10微秒
	DS18B20_INPUT_MODE();
	dat=DS18B20_IN_READ();
	byte=(byte>>1)|(dat<<7);
	delay_us(45);//延迟45微秒读完1位
	DS18B20_OUTPUT_MODE();
	DS18B20_OUT_HIGH();//继续拉高为读下一位做准备
	}
	return byte;
}

void Ds18b20ChangeTemp(){
	Ds18b20_Init();
	delay_ms(1);
	Ds18b20WriteByte(0xcc);//跳过ROM直接发送温度转换命令
	Ds18b20WriteByte(0x44);//发送指令RAM设为0x44为温度变换
}

void Ds18b20ReadTempCom(){
	Ds18b20_Init();
	delay_ms(1);
	Ds18b20WriteByte(0xcc);//跳过ROM直接发送温度转换命令
	Ds18b20WriteByte(0xbe);//发送指令RAM设为0xBE为读暂时寄存器
}

int Ds18b20ReadTemp(void){
	int temp=0;
	uchar tml,tmh;	
	Ds18b20ChangeTemp();
	Ds18b20ReadTempCom();
	tml=Ds18b20ReadByte();//读低8位数据
	tmh=Ds18b20ReadByte();//读高8位数据
	temp=tmh;
	temp<<=8;
	temp|=tml;//拼接为16位数据
	return temp;//返回16位数据
}
