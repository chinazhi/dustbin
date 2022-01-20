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

//#define CS1238_DEBUG 1 // �ͷ�ע�Ϳ�����ӡ

#define DOUT_GPIO_PIN 1 ///< cs1238 ���ݽ�
#define SCLK_GPIO_PIN 2 ///< ģ��ʱ�ӽ�

/* ʱ�ӽ� ���ģʽ�� �ߵͿ��� */
#define IO_CLK_AD_H() nrf_gpio_pin_set(SCLK_GPIO_PIN)
#define IO_CLK_AD_L() nrf_gpio_pin_clear(SCLK_GPIO_PIN)

/* ���ݽ� ���ģʽ�� �ߵͿ��� */
#define IO_DATA_AD_H() nrf_gpio_pin_set(DOUT_GPIO_PIN)
#define IO_DATA_AD_L() nrf_gpio_pin_clear(DOUT_GPIO_PIN)

/* ���ݽ� ����ģʽ�� ��ȡ����ֵ */
#define GET_IO_AD_DATA nrf_gpio_pin_read(DOUT_GPIO_PIN)

// �޷�ƽ���˲���
#define FILTER_MAX 10000	// �޷��˲����ֵ
#define FILTER_N 11			  // ƽ���˲����� 

int32_t filter_adc1_init = 0;		    // ��ʼ�׶��������
int32_t filter_adc1_buf[FILTER_N];	// ADC1�ĵ���ƽ���˲�����

int32_t filter_adc2_init = 0;
int32_t filter_adc2_buf[FILTER_N];

/* ȫ�ֳ������� */
cs1238_load_info_t g_load_info;
int error_adc1;          // ��������ֵ��Ϊ�������쳣 
int error_adc1_num;      // ����������ֵ�Ĵ���
int right_adc1_num;      // δ��������ֵ�Ĵ���
int error_adc2;
int error_adc2_num;
int right_adc2_num;
/* ����״̬������״̬ */
int work_cycle_start_flag = 0;       // ����ѭ����ʼ��־ 0 δ��ʼ 1��ʼ
int cs1238_set_error_num = 0;
operation_cs1238_state  cs1238_state;

/**
 * @brief �������ݹ�������ģʽΪ ��������
 * 
 */
void set_dout_mode_in(void)
{
	nrf_gpio_cfg_input(DOUT_GPIO_PIN, NRF_GPIO_PIN_PULLUP);
}

/**
 * @brief �������ݹ�������ģʽΪ ���ģʽ
 * 
 */
void set_dout_mode_out(void)
{
	nrf_gpio_cfg_output(DOUT_GPIO_PIN);
}

/**
 * @brief ����ʱ������Ϊ���ģʽ ����
 * 
 */
void set_sclk_mode_out(void)
{
	nrf_gpio_cfg_output(SCLK_GPIO_PIN);
	IO_CLK_AD_L();
}

/**
 * @brief ͨ��ʱ�� SCL ��ƽ��ʱʱ��
 *        ��Ҫ<100��s�� ������󴥷���������ģʽ�� һ�㽨�� SCL=2��s~15��s
 */
void sclk_delay(void)
{
	nrf_delay_us(3);
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
		nrf_delay_ms(1); //��ʱ��ѯ�ķ�����Ҫ���̲�ѯ��� ����:1ms~5ms
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
		nrf_delay_ms(1); //��ʱ��ѯ�ķ�����Ҫ���̲�ѯ��� ����:1ms~5ms
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
	reg_temp = 0xCA; 				//�����Ϊ 7bits (д0x65)����1λ
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
		nrf_delay_ms(1); //��ʱ��ѯ�ķ�����Ҫ���̲�ѯ��� ����:1ms~5ms
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

/**
 * @brief �޷�ƽ���˲� ADC1
 * 
 * @param[in] read_data 
 * @return int ���� �˲������ֵ
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
    filter_adc1_buf[i] = filter_adc1_buf[i + 1]; // �����������ƣ���λ�Ե�
    filter_sum += filter_adc1_buf[i];
  }
  return (int)filter_sum / (FILTER_N - 1);
}

/**
 * @brief �޷�ƽ���˲� ADC2
 * 
 * @param[in] read_data 
 * @return int ���� �˲������ֵ
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
    filter_adc2_buf[i] = filter_adc2_buf[i + 1]; // �����������ƣ���λ�Ե�
    filter_sum += filter_adc2_buf[i];
  }
  return (int)filter_sum / (FILTER_N - 1);
}

/**
 * @brief ����ƫ��ֵ
 * 
 */
void compute_up_bias(void)
{
  int32_t relative_adc1, relative_adc2, relative_average;

  // ���㴫���������������
  relative_adc1 = g_load_info.adc1_data - ram_flash_data.single_adc1;
  relative_adc2 = g_load_info.adc2_data - ram_flash_data.single_adc2;
  relative_average = g_load_info.average_ad - ram_flash_data.no_load_ad;

  // ƽ����������� С��400/AD �����м��� �������� ��ʵ������
  if(relative_average < 400)
  {
    g_load_info.up_bias = 0;
    return ;
  }

  // ����ƫ���� 
  if( relative_adc1 > relative_average) // ��ƫ��
  {
    g_load_info.up_bias = ((relative_adc1 - relative_average)*100)/relative_average;
  }

  if(relative_adc2 > relative_average)	// ��ƫ��
  {
    g_load_info.up_bias = (((relative_adc2 - relative_average)*100)/relative_average) * (-1);
  }
  
  // ����ƫ�������ֵ
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
 * @brief �������쳣���
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
    if (abs(data) > error_adc1)            // adc1�������ֵ
    {
      error_adc1_num++;                    // �������+1
      if (error_adc1_num > 5)              // 5�κ���Ϊ�������쳣
      {
        g_load_info.sensor_state = 1;
        error_adc1_num = 0;
        right_adc1_num = 0;
      }
    }
    else if(g_load_info.sensor_state != 2) // ADC1 ������ ADC2 Ҳ����
    {
      g_load_info.sensor_state = 0;        // ��Ϊ����������
      error_adc1_num = 0;
      if(right_adc1_num < 10)              // 10 �������� �����޷��˲�����
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
 * @brief ���㹤��ѭ�� 
 * 
 */
void compute_work_cycle(void)
{
  if((g_load_info.real_load > ram_flash_data.cycle_load[0]) && (g_load_info.real_load < ram_flash_data.load_alarm*2 ))
  {
    // ѭ����ʼ ��ʼ����ֵ
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
    // ��¼ѭ���ڼ����ֵ
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
    // ѭ������ 
    if(work_cycle_start_flag == 1)
    {
      work_cycle_start_flag = 0;
      g_load_info.work_data.end_time = g_device_info.running_time;
      g_load_info.work_data.cycle_info = 1;
      //printf("work_cycle_start_flag�� %d\n",work_cycle_start_flag);
    }
  }
}


/**
 * @brief ���������������
 */
void compute_load_data(void)
{
  // ����ƽ��AD
  g_load_info.average_ad = (g_load_info.adc1_data) / 2 + (g_load_info.adc2_data) / 2;

  // ����ʵ������ ���ڿ���AD ��ʾ����
  if (g_load_info.average_ad > ram_flash_data.no_load_ad)
  {
    g_load_info.real_load = (g_load_info.average_ad - ram_flash_data.no_load_ad) / (ram_flash_data.per_kg_ad);
  }
  else
  {
    g_load_info.real_load = 0;
  }

  // ����ƫ����
  compute_up_bias();

  // �ж��Ƿ�Ԥ����
  g_load_info.up_state = 0;
  if (g_load_info.real_load > ram_flash_data.load_warn)
  {
    g_load_info.up_state = 1;
  }
  if (g_load_info.real_load > ram_flash_data.load_alarm)
  {
    g_load_info.up_state = 2;
  }
  // ����ѭ��
  compute_work_cycle();
}

/**
 * @brief ��ʼ��cs1238 
 * 
 */
void new_cs1238_init(void)
{
	// ���� cs1238ʱ������ 
	set_sclk_mode_out();

	// ���� cs1238���ݽ�  
	set_dout_mode_in();

	// �ϵ����������ʱ��
	nrf_delay_ms(100);

	// ������״̬λ ��ʼ������
	g_load_info.sensor_state = 0;

	// ��ʼ���� ͨ��A ��A����
	cs1238_state = operation_read_a;
	set_cs1238_config(CHANNEL_A_CONFIG);

  // ��ʼ����⴫��������
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
 * @brief ˳���ȡCS1238 ͨ��A ͨ��B ADC��ֵ
 */
void polling_read_cs1238_data(void)
{
	int32_t data_temp = 0;

	switch (cs1238_state)
	{
	case operation_read_a:      // ��Aͨ�� ��״̬�л�Ϊ ����B
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

	case operation_select_b:   // ����B ��״̬�л�Ϊ ��Bͨ��	
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
				compute_load_data();// �˲���ʼ����ʼ����
			}
			cs1238_state = operation_read_b;
			cs1238_set_error_num = 0;
		}
		break;

	case operation_read_b:    // ��Bͨ�� ��״̬�л�Ϊ ����A
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

	case operation_select_a: // ����A ��״̬�л�Ϊ ��Aͨ��
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
				compute_load_data();// �˲���ʼ����ʼ����
			}
			cs1238_state = operation_read_a;
			cs1238_set_error_num = 0;
		}
		break;

	case operation_read_err: // ������״̬����
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

	case operation_set_err: // ���ô���״̬����
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

int temp_data = 0;		// �����������
int power_flag = 0;   // ��ʱ����
int power_state = 1;	// ���ز�������״̬ 0 �͹��� 1 �߹���

void low_power_read_cs1238(void)
{
  if (power_state) // ���� �߹���ģʽ
  {
    power_flag++;
    polling_read_cs1238_data(); // ���ز���

    if (abs(temp_data - g_load_info.real_load) <= 2) // �����ȶ�
    {
      if (power_flag > CS1238_LOW_TIME * 15) // �ȶ�15s �����͹���
      {
        power_flag = 0;  // �����ʱ�ۼ�
        power_state = 0; // �����͹���
      }
      if (!(power_flag % 20)) // �����˲����� ���1sˢ�»��� ����ˢ�¹�������Ϊһֱ�ȶ�
      {
        temp_data = g_load_info.real_load;
      }
    }
    else // ���ز��ȶ�
    {
      power_flag = 0;
      temp_data = g_load_info.real_load;
    }
  }
  else             // ���� �͹���ģʽ
  {
    power_flag++;

    if (power_flag <= CS1238_LOW_TIME * 1) // �͹����� 1s ���� 5s˯��
    {
      polling_read_cs1238_data(); // ���زɼ�

      if (!(power_flag % 20)) // 1s�ɼ�������ж� ����ˢ�¹�������Ϊһֱ�ȶ�
      {
        //printf("1:%d 2:%d\n", temp_data, g_load_info.real_load);
        if (abs(temp_data - g_load_info.real_load) > 5) // ���ز��ȶ� ����5Kg����
        {
          power_flag = 0;  // �����ʱ�ۼ�
          power_state = 1; // �����߹���
        }
        temp_data = g_load_info.real_load;
      }
    }
    else if (power_flag > CS1238_LOW_TIME * ram_flash_data.weigh_sample_inv) // һ�����ڽ��� ������һ������
    {
      power_flag = 0;
    }
    else
    {
      sleep_cs1238(); // ���ؽ�������
    }
  }

  // ��������Ƿ������� ����������²�����͹���
  if (g_device_info.ble_status == 2)
  {
    power_state = 1; // �����߹���
  }
}
