#ifndef CS1238_H
#define CS1238_H

#include "type.h"

#define CS1238_RIGHT_NUM 10            // 称重传感器连续成功次数
#define CS1238_LOW_TIME  20            // 延时时间 50ms * 20 = 1s
#define CHANNEL_A_CONFIG 0x14          // 通道1 2倍放大 40Hz
#define CHANNEL_B_CONFIG 0x15          // 通道2 2倍放大 40Hz

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
  uint8_t  cycle_info;     // 工作循环 信息 0 无工作循环 1有工作循环

  int32_t  max_load; 
  int32_t  max_up_bias;    // 最大 偏置值
  int32_t  up_state;       // 工作循环期间 载重状态 0 normal 1 warnning 2 alarm
  uint32_t start_time;	   // 开始时间   
  uint32_t end_time;	     // 结束时间  
}work_cycle;

typedef struct
{
  int32_t adc1_data;       // 通道1 ADC数值
  int32_t adc2_data;       // 通道2 ADC数值
  int32_t average_ad;      // 两个通道的平均载重 AD

  int32_t real_load;       // 实时载重 kg
  int32_t up_bias;         // 偏置值
  int32_t up_state;        // 载重状态 0 normal 1 warnning 2 alarm

  int32_t sensor_state;    // 传感器状态 0 正常 1 右异常 2 左异常 3 芯片异常
  work_cycle work_data;    // 工作循环数据
} cs1238_load_info_t;

/**
 * @brief 称重相关数据
 * 
 */
extern cs1238_load_info_t g_load_info;

/**
 * @brief 初始化cs1238 
 * 
 */
void new_cs1238_init(void);

/**
 * @brief 顺序读取CS1238 通道A 通道B ADC数值
 */
void polling_read_cs1238_data(void);

#endif
