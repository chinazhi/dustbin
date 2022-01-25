#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "type.h"

#define BSP_GPO_HI      1
#define BSP_GPO_LO      0                                           

// GPO 
#define LED_A0                    0 
#define GPO0_CLK                  RCU_GPIOA        
#define GPO0_PORT                 GPIOA
#define GPO0_PIN                  GPIO_PIN_7
#define GPO0_VALUE                BSP_GPO_HI
#define GPO0_OTYPE                GPIO_MODE_OUT_PP

#define LED_A1                    1
#define GPO1_CLK                  RCU_GPIOC        
#define GPO1_PORT                 GPIOC
#define GPO1_PIN                  GPIO_PIN_4
#define GPO1_VALUE                BSP_GPO_HI
#define GPO1_OTYPE                GPIO_MODE_OUT_PP

#define LED_A2                    2
#define GPO2_CLK                  RCU_GPIOC        
#define GPO2_PORT                 GPIOC
#define GPO2_PIN                  GPIO_PIN_5
#define GPO2_VALUE                BSP_GPO_HI
#define GPO2_OTYPE                GPIO_MODE_OUT_PP

#define LED_ERROR                 3
#define GPO3_CLK                  RCU_GPIOC        
#define GPO3_PORT                 GPIOC
#define GPO3_PIN                  GPIO_PIN_1
#define GPO3_VALUE                BSP_GPO_HI
#define GPO3_OTYPE                GPIO_MODE_OUT_PP

#define LED_COM                   4 
#define GPO4_CLK                  RCU_GPIOC        
#define GPO4_PORT                 GPIOC
#define GPO4_PIN                  GPIO_PIN_2
#define GPO4_VALUE                BSP_GPO_HI
#define GPO4_OTYPE                GPIO_MODE_OUT_PP

#define RS485_EN                  5 
#define GPO5_CLK                  RCU_GPIOA        
#define GPO5_PORT                 GPIOA
#define GPO5_PIN                  GPIO_PIN_11
#define GPO5_VALUE                BSP_GPO_HI
#define GPO5_OTYPE                GPIO_MODE_OUT_PP

#define DO_FAN_EN                 6 
#define GPO6_CLK                  RCU_GPIOE        
#define GPO6_PORT                 GPIOE
#define GPO6_PIN                  GPIO_PIN_13
#define GPO6_VALUE                BSP_GPO_HI
#define GPO6_OTYPE                GPIO_MODE_OUT_PP

#define DO_LIGHT_EN               7 
#define GPO7_CLK                  RCU_GPIOE        
#define GPO7_PORT                 GPIOE
#define GPO7_PIN                  GPIO_PIN_14
#define GPO7_VALUE                BSP_GPO_HI
#define GPO7_OTYPE                GPIO_MODE_OUT_PP

#define FULL_OUT_EN               8 
#define GPO8_CLK                  RCU_GPIOE        
#define GPO8_PORT                 GPIOE
#define GPO8_PIN                  GPIO_PIN_11
#define GPO8_VALUE                BSP_GPO_HI
#define GPO8_OTYPE                GPIO_MODE_OUT_PP

#define BTN_OUT_EN                9 
#define GPO9_CLK                  RCU_GPIOB        
#define GPO9_PORT                 GPIOB
#define GPO9_PIN                  GPIO_PIN_6
#define GPO9_VALUE                BSP_GPO_HI
#define GPO9_OTYPE                GPIO_MODE_OUT_PP


/*********************** GPI ***************************/
// 烟感
#define SMOKE_EN                  0
#define GPI0_CLK                  RCU_GPIOD        
#define GPI0_PORT                 GPIOD
#define GPI0_PIN                  GPIO_PIN_14
#define GPI0_VALUE                GPIO_MODE_IN_FLOATING
// 前限位
#define POS_F_EN                  1
#define GPI1_CLK                  RCU_GPIOD        
#define GPI1_PORT                 GPIOD
#define GPI1_PIN                  GPIO_PIN_15
#define GPI1_VALUE                GPIO_MODE_IN_FLOATING
// 后限位
#define POS_B_EN                  2
#define GPI2_CLK                  RCU_GPIOC       
#define GPI2_PORT                 GPIOC
#define GPI2_PIN                  GPIO_PIN_6
#define GPI2_VALUE                GPIO_MODE_IN_FLOATING
//led button
#define LED_BTN_IN_EN             3
#define GPI3_CLK                  RCU_GPIOC       
#define GPI3_PORT                 GPIOC
#define GPI3_PIN                  GPIO_PIN_7
#define GPI3_VALUE                GPIO_MODE_IN_FLOATING
// 满溢
#define FULL_IN_EN                4
#define GPI4_CLK                  RCU_GPIOD       
#define GPI4_PORT                 GPIOD
#define GPI4_PIN                  GPIO_PIN_0
#define GPI4_VALUE                GPIO_MODE_IN_FLOATING
// 人员
#define BODY_IN_EN                5
#define GPI5_CLK                  RCU_GPIOD       
#define GPI5_PORT                 GPIOD
#define GPI5_PIN                  GPIO_PIN_1
#define GPI5_VALUE                GPIO_MODE_IN_FLOATING

/**
 * @brief init the specified gpo 
 * 
 * @param[in] index 
 */
void bsp_gpo_init(uint8_t index);

/**
 * @brief pull low the specified gpo
 * 
 * @param[in] index 
 */
void bsp_gpo_low(uint8_t index);

/**
 * @brief pull high the specified gpo
 * 
 * @param[in] index 
 */
void bsp_gpo_high(uint8_t index);

/**
 * @brief the specified gpo status toggle
 * 
 * @param[in] index 
 */
void bsp_gpo_toggle(uint8_t index);

/**
 * @brief 
 * 
 * @param[in] index 
 */
void bsp_gpi_poll_init(uint8_t index);

/**
 * @brief 
 * 
 * @param[in] index 
 * @return uint8_t 
 */
uint8_t bsp_gpi_state_get(uint8_t index);

#endif
