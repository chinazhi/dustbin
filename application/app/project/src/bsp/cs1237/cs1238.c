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

//#define CS1238_DEBUG 1 // 释放注释开启打印

#define DOUT_GPIO_PIN 1 ///< cs1238 数据脚
#define SCLK_GPIO_PIN 2 ///< 模拟时钟脚

/* 时钟脚 输出模式下 高低控制 */
#define IO_CLK_AD_H() nrf_gpio_pin_set(SCLK_GPIO_PIN)
#define IO_CLK_AD_L() nrf_gpio_pin_clear(SCLK_GPIO_PIN)

/* 数据脚 输出模式下 高低控制 */
#define IO_DATA_AD_H() nrf_gpio_pin_set(DOUT_GPIO_PIN)
#define IO_DATA_AD_L() nrf_gpio_pin_clear(DOUT_GPIO_PIN)

/* 数据脚 输入模式下 读取的数值 */
#define GET_IO_AD_DATA nrf_gpio_pin_read(DOUT_GPIO_PIN)

// 限幅平均滤波法
#define FILTER_MAX 10000	// 限幅滤波最大值
#define FILTER_N 11			  // 平均滤波容量 

int32_t filter_adc1_init = 0;		    // 开始阶段数据填充
int32_t filter_adc1_buf[FILTER_N];	// ADC1的递推平滑滤波数组

int32_t filter_adc2_init = 0;
int32_t filter_adc2_buf[FILTER_N];

/* 全局称重数据 */
cs1238_load_info_t g_load_info;
int error_adc1;          // 超过此数值认为传感器异常 
int error_adc1_num;      // 超过错误数值的次数
int right_adc1_num;      // 未超过错误值的次数
int error_adc2;
int error_adc2_num;
int right_adc2_num;
/* 称重状态机操作状态 */
int work_cycle_start_flag = 0;       // 工作循环开始标志 0 未开始 1开始
int cs1238_set_error_num = 0;
operation_cs1238_state  cs1238_state;

/**
 * @brief 设置数据工作引脚模式为 输入上拉
 * 
 */
void set_dout_mode_in(void)
{
	nrf_gpio_cfg_input(DOUT_GPIO_PIN, NRF_GPIO_PIN_PULLUP);
}

/**
 * @brief 设置数据工作引脚模式为 输出模式
 * 
 */
void set_dout_mode_out(void)
{
	nrf_gpio_cfg_output(DOUT_GPIO_PIN);
}

/**
 * @brief 设置时钟引脚为输出模式 拉低
 * 
 */
void set_sclk_mode_out(void)
{
	nrf_gpio_cfg_output(SCLK_GPIO_PIN);
	IO_CLK_AD_L();
}

/**
 * @brief 通信时序 SCL 电平延时时间
 *        需要<100μs， 否则会误触发进入休眠模式， 一般建议 SCL=2μs~15μs
 */
void sclk_delay(void)
{
	nrf_delay_us(3);
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
		nrf_delay_ms(1); //定时查询的方法需要缩短查询间隔 例如:1ms~5ms
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
		nrf_delay_ms(1); //定时查询的方法需要缩短查询间隔 例如:1ms~5ms
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
	reg_temp = 0xCA; 				//命令长度为 7bits (写0x65)左移1位
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
		nrf_delay_ms(1); //定时查询的方法需要缩短查询间隔 例如:1ms~5ms
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

/**
 * @brief 限幅平均滤波 ADC1
 * 
 * @param[in] read_data 
 * @return int 返回 滤波后的数值
 */
int filter_adc1(int32_t read_data)
{
  int i;
  int filter_sum = 0;
  filter_adc1_buf[FILTER_N - 1] = read_data;
  if ((g_load_info.sensor_state == 0) && (right_adc1_num >= CS1238_RIGHT_NUM))
  {
    if (((filter_adc1_buf[FILTER_N - 1] - filter_adc1_buf[FILTER_N - 2]) > FILTER_MAX) || ((filter_adc1_buf[FILTER_N - 2] - filter_adc1_buf[FILTER_N - 1]) > FILTER_MAX))
    {
      filter_adc1_buf[FILTER_N - 1] = filter_adc1_buf[FILTER_N - 2];
    }
  }
  else
  {
    #ifdef CS1238_DEBUG
    printf("close filter adc1\n");
    #endif
  }

  for (i = 0; i < FILTER_N - 1; i++)
  {
    filter_adc1_buf[i] = filter_adc1_buf[i + 1]; // 所有数据左移，低位仍掉
    filter_sum += filter_adc1_buf[i];
  }
  return (int)filter_sum / (FILTER_N - 1);
}

/**
 * @brief 限幅平均滤波 ADC2
 * 
 * @param[in] read_data 
 * @return int 返回 滤波后的数值
 */
int filter_adc2(int32_t read_data)
{
  int i;
  int filter_sum = 0;
  filter_adc2_buf[FILTER_N - 1] = read_data;
  if ((g_load_info.sensor_state == 0) && (right_adc2_num >= CS1238_RIGHT_NUM))
  {
    if (((filter_adc2_buf[FILTER_N - 1] - filter_adc2_buf[FILTER_N - 2]) > FILTER_MAX) || ((filter_adc2_buf[FILTER_N - 2] - filter_adc2_buf[FILTER_N - 1]) > FILTER_MAX))
    {
      filter_adc2_buf[FILTER_N - 1] = filter_adc2_buf[FILTER_N - 2];
    }
  }
  else
  {
    #ifdef CS1238_DEBUG
    printf("close filter adc2\n");
    #endif
  }

  for (i = 0; i < FILTER_N - 1; i++)
  {
    filter_adc2_buf[i] = filter_adc2_buf[i + 1]; // 所有数据左移，低位仍掉
    filter_sum += filter_adc2_buf[i];
  }
  return (int)filter_sum / (FILTER_N - 1);
}

/**
 * @brief 计算偏置值
 * 
 */
void compute_up_bias(void)
{
  int32_t relative_adc1, relative_adc2, relative_average;

  // 计算传感器的相对增加量
  relative_adc1 = g_load_info.adc1_data - ram_flash_data.single_adc1;
  relative_adc2 = g_load_info.adc2_data - ram_flash_data.single_adc2;
  relative_average = g_load_info.average_ad - ram_flash_data.no_load_ad;

  // 平均相对增加量 小于400/AD 不进行计算 重量过轻 无实际意义
  if(relative_average < 400)
  {
    g_load_info.up_bias = 0;
    return ;
  }

  // 计算偏置率 
  if( relative_adc1 > relative_average) // 右偏置
  {
    g_load_info.up_bias = ((relative_adc1 - relative_average)*100)/relative_average;
  }

  if(relative_adc2 > relative_average)	// 左偏置
  {
    g_load_info.up_bias = (((relative_adc2 - relative_average)*100)/relative_average) * (-1);
  }
  
  // 限制偏置率最大值
  if(g_load_info.up_bias > 99) 
  {
    g_load_info.up_bias = 99;
  }
  else if(g_load_info.up_bias < -99)
  {
    g_load_info.up_bias = -99;
  }
  #ifdef CS1238_DEBUG
  printf("up_bias %d %d\n",  g_load_info.up_bias, relative_average);
  #endif

}

/**
 * @brief 传感器异常监测
 * 
 * @param[in] data 
 * @param[in] adc_num 
 */
void compute_sensor_error(int data, int adc_num)
{
  if(adc_num == 1)
  {
    #ifdef CS1238_DEBUG
    printf("data_temp a: %d %d\n", data, error_adc1);
    #endif
    if (abs(data) > error_adc1)            // adc1超出最大值
    {
      error_adc1_num++;                    // 错误次数+1
      if (error_adc1_num > 5)              // 5次后认为传感器异常
      {
        g_load_info.sensor_state = 1;
        error_adc1_num = 0;
        right_adc1_num = 0;
      }
    }
    else if(g_load_info.sensor_state != 2) // ADC1 正常且 ADC2 也正常
    {
      g_load_info.sensor_state = 0;        // 认为传感器正常
      error_adc1_num = 0;
      if(right_adc1_num < 10)              // 10 次正常后 开启限幅滤波限制
      {
        right_adc1_num++;
      }
    }
  }
  else if(adc_num == 2)
  {
    #ifdef CS1238_DEBUG
    printf("data_temp b: %d %d\n", data, error_adc2);
    #endif
    if(abs(data) > error_adc2)
    {
      error_adc2_num++;
      if (error_adc2_num > 4)
      {
        g_load_info.sensor_state = 2;
        error_adc2_num = 0;
        right_adc2_num = 0;
      }
    }
    else if(g_load_info.sensor_state != 1)
    {
      g_load_info.sensor_state = 0;
      error_adc2_num = 0;
      if(right_adc2_num < 10)
      {
        right_adc2_num++;
      }
    }
  }
}

/**
 * @brief 计算工作循环 
 * 
 */
void compute_work_cycle(void)
{
  if((g_load_info.real_load > ram_flash_data.cycle_load[0]) && (g_load_info.real_load < ram_flash_data.load_alarm*2 ))
  {
    // 循环开始 初始化数值
    if(work_cycle_start_flag == 0)
    {
      work_cycle_start_flag = 1;
      g_load_info.work_data.cycle_info = 0;   
      g_load_info.work_data.max_load = 0;
      g_load_info.work_data.max_up_bias = 0;
      g_load_info.work_data.up_state = 0;
      g_load_info.work_data.end_time = 0;
      g_load_info.work_data.start_time = g_device_info.running_time;
    }
    // 记录循环期间最大值
    if(g_load_info.work_data.max_load < g_load_info.real_load)
    {
      g_load_info.work_data.max_load = g_load_info.real_load;
    }
    if(g_load_info.work_data.max_up_bias < g_load_info.up_bias)
    {
      g_load_info.work_data.max_up_bias = g_load_info.up_bias;
    }
    if(g_load_info.work_data.up_state == 0)
    {
      g_load_info.work_data.up_state = g_load_info.up_state;
    }
  }
  else if (g_load_info.real_load < ram_flash_data.cycle_load[1])
  {
    // 循环结束 
    if(work_cycle_start_flag == 1)
    {
      work_cycle_start_flag = 0;
      g_load_info.work_data.end_time = g_device_info.running_time;
      g_load_info.work_data.cycle_info = 1;
      //printf("work_cycle_start_flag： %d\n",work_cycle_start_flag);
    }
  }
}


/**
 * @brief 计算载重相关数据
 */
void compute_load_data(void)
{
  // 计算平均AD
  g_load_info.average_ad = (g_load_info.adc1_data) / 2 + (g_load_info.adc2_data) / 2;

  // 计算实际载重 大于空载AD 显示数据
  if (g_load_info.average_ad > ram_flash_data.no_load_ad)
  {
    g_load_info.real_load = (g_load_info.average_ad - ram_flash_data.no_load_ad) / (ram_flash_data.per_kg_ad);
  }
  else
  {
    g_load_info.real_load = 0;
  }

  // 计算偏置率
  compute_up_bias();

  // 判断是否预报警
  g_load_info.up_state = 0;
  if (g_load_info.real_load > ram_flash_data.load_warn)
  {
    g_load_info.up_state = 1;
  }
  if (g_load_info.real_load > ram_flash_data.load_alarm)
  {
    g_load_info.up_state = 2;
  }
  // 工作循环
  compute_work_cycle();
}

/**
 * @brief 初始化cs1238 
 * 
 */
void new_cs1238_init(void)
{
	// 设置 cs1238时钟引脚 
	set_sclk_mode_out();

	// 设置 cs1238数据脚  
	set_dout_mode_in();

	// 上电所需最长建立时间
	nrf_delay_ms(100);

	// 传感器状态位 初始化正常
	g_load_info.sensor_state = 0;

	// 初始设置 通道A 读A操作
	cs1238_state = operation_read_a;
	set_cs1238_config(CHANNEL_A_CONFIG);

  // 初始化检测传感器参数
  error_adc1 = ram_flash_data.load_alarm * ram_flash_data.per_kg_ad * 1.2 - ram_flash_data.single_adc1;
  error_adc2 = ram_flash_data.load_alarm * ram_flash_data.per_kg_ad * 1.2 - ram_flash_data.single_adc2;
  error_adc1_num = 0;
  right_adc1_num = CS1238_RIGHT_NUM;
  error_adc2_num = 0;
  right_adc2_num = CS1238_RIGHT_NUM;

#ifdef CS1238_DEBUG
	printf("cs1238 init ok\n");
	#endif
}

/**
 * @brief 顺序读取CS1238 通道A 通道B ADC数值
 */
void polling_read_cs1238_data(void)
{
	int32_t data_temp = 0;

	switch (cs1238_state)
	{
	case operation_read_a:      // 读A通道 后状态切换为 设置B
		data_temp = read_cs1238_ad_data();
    compute_sensor_error(data_temp, 1);
		if(data_temp == 1)
		{
			cs1238_state = operation_read_err;
			#ifdef CS1238_DEBUG
			printf("adc1 error\n");
			#endif
		}
		else
		{
			if(filter_adc1_init < FILTER_N)
			{
				filter_adc1_buf[filter_adc1_init] = data_temp;
				filter_adc1_init++;
			}
			else
			{
				g_load_info.adc1_data = filter_adc1(data_temp);
				#ifdef CS1238_DEBUG
				printf("adc1: %d\n", g_load_info.adc1_data);
				#endif
			}
			cs1238_state = operation_select_b;
		}
		break;

	case operation_select_b:   // 设置B 后状态切换为 读B通道	
		if(set_cs1238_config(CHANNEL_B_CONFIG))
		{
			cs1238_state = operation_set_err;
			#ifdef CS1238_DEBUG
			printf("set B error\n");
			#endif
		}
		else
		{
			if(filter_adc2_init >= FILTER_N )
			{
				compute_load_data();// 滤波初始化后开始计算
			}
			cs1238_state = operation_read_b;
			cs1238_set_error_num = 0;
		}
		break;

	case operation_read_b:    // 读B通道 后状态切换为 设置A
		data_temp = read_cs1238_ad_data();
    compute_sensor_error(data_temp, 2);
		if(data_temp == 1)
		{
			cs1238_state = operation_read_err;
			#ifdef CS1238_DEBUG
			printf("adc2 error\n");
			#endif
		}
		else
		{
			if(filter_adc2_init < FILTER_N)
			{
				filter_adc2_buf[filter_adc2_init] = data_temp;
				filter_adc2_init++;
			}
			else
			{
				g_load_info.adc2_data = filter_adc2(data_temp);
				#ifdef CS1238_DEBUG
				printf("adc2: %d\n",g_load_info.adc2_data);
				#endif				
			}
			cs1238_state = operation_select_a;
		}
		break;

	case operation_select_a: // 设置A 后状态切换为 读A通道
		if(set_cs1238_config(CHANNEL_A_CONFIG))
		{	
			cs1238_state = operation_set_err;
			#ifdef CS1238_DEBUG
			printf("set A error\n");
			#endif
		}
		else
		{
			if(filter_adc2_init >= FILTER_N )
			{
				compute_load_data();// 滤波初始化后开始计算
			}
			cs1238_state = operation_read_a;
			cs1238_set_error_num = 0;
		}
		break;

	case operation_read_err: // 读错误状态处理
		cs1238_set_error_num++;
		if(cs1238_set_error_num >10)
		{
			cs1238_state = operation_cs1238_err;	
		}
		else
		{
			set_sclk_mode_out();
			set_dout_mode_in();
			cs1238_state = operation_read_a;
		}
		break;

	case operation_set_err: // 设置错误状态处理
		cs1238_set_error_num++;
		if(cs1238_set_error_num >10)
		{
			cs1238_state = operation_cs1238_err;	
		}
		else
		{
			set_sclk_mode_out();
			set_dout_mode_in();
			cs1238_state = operation_select_a;
		}
		break;

	case operation_cs1238_err:
		g_load_info.sensor_state = 3;
		cs1238_set_error_num = 0;
		new_cs1238_init();
		break;	

	default:
		break;
	}

}

/**
 * @brief make cs1238 sleep
 * 
 */
void sleep_cs1238(void)
{
	IO_CLK_AD_H();
}

int temp_data = 0;		// 缓存称重数据
int power_flag = 0;   // 延时计数
int power_state = 1;	// 称重采样工作状态 0 低功率 1 高功率

void low_power_read_cs1238(void)
{
  if (power_state) // 称重 高功率模式
  {
    power_flag++;
    polling_read_cs1238_data(); // 称重采样

    if (abs(temp_data - g_load_info.real_load) <= 2) // 称重稳定
    {
      if (power_flag > CS1238_LOW_TIME * 15) // 稳定15s 后进入低功耗
      {
        power_flag = 0;  // 清除延时累加
        power_state = 0; // 开启低功耗
      }
      if (!(power_flag % 20)) // 由于滤波存在 因此1s刷新缓存 避免刷新过快误判为一直稳定
      {
        temp_data = g_load_info.real_load;
      }
    }
    else // 称重不稳定
    {
      power_flag = 0;
      temp_data = g_load_info.real_load;
    }
  }
  else             // 称重 低功率模式
  {
    power_flag++;

    if (power_flag <= CS1238_LOW_TIME * 1) // 低功率下 1s 采样 5s睡眠
    {
      polling_read_cs1238_data(); // 称重采集

      if (!(power_flag % 20)) // 1s采集后进行判断 避免刷新过快误判为一直稳定
      {
        //printf("1:%d 2:%d\n", temp_data, g_load_info.real_load);
        if (abs(temp_data - g_load_info.real_load) > 5) // 称重不稳定 超过5Kg唤醒
        {
          power_flag = 0;  // 清除延时累加
          power_state = 1; // 开启高功耗
        }
        temp_data = g_load_info.real_load;
      }
    }
    else if (power_flag > CS1238_LOW_TIME * ram_flash_data.weigh_sample_inv) // 一个周期结束 进入下一个周期
    {
      power_flag = 0;
    }
    else
    {
      sleep_cs1238(); // 称重进入休眠
    }
  }

  // 检测蓝牙是否已连接 已连接情况下不进入低功耗
  if (g_device_info.ble_status == 2)
  {
    power_state = 1; // 开启高功耗
  }
}
