#ifndef BSP_CS123X_H
#define BSP_CS123X_H

#include "type.h"

#define CS1238_RIGHT_NUM 10            // ���ش����������ɹ�����
#define CS1238_LOW_TIME  20            // ��ʱʱ�� 50ms * 20 = 1s
#define CHANNEL_A_CONFIG 0x14          // ͨ��1 2���Ŵ� 40Hz
#define CHANNEL_B_CONFIG 0x15          // ͨ��2 2���Ŵ� 40Hz

typedef enum
{
    operation_select_a = 0,
    operation_read_a,
    operation_select_b,
    operation_read_b,
    operation_cs1238_err,
    operation_read_err,
    operation_set_err
}operation_cs1238_state;

typedef struct
{
  int32_t adc1_data;       // ͨ��1 ADC��ֵ
  int32_t adc2_data;       // ͨ��2 ADC��ֵ
  int32_t average_ad;      // ����ͨ����ƽ������ AD

  int32_t real_load;       // ʵʱ���� kg
  int32_t up_bias;         // ƫ��ֵ
  int32_t up_state;        // ����״̬ 0 normal 1 warnning 2 alarm

  int32_t sensor_state;    // ������״̬ 0 ���� 1 ���쳣 2 ���쳣 3 оƬ�쳣
} cs1238_load_info_t;

/**
 * @brief �����������
 * 
 */
extern cs1238_load_info_t g_load_info;

/**
 * @brief ��ʼ��cs1238 
 * 
 */
void new_cs1238_init(void);

/**
 * @brief ˳���ȡCS1238 ͨ��A ͨ��B ADC��ֵ
 */
void polling_read_cs1238_data(void);

#endif