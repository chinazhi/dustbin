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
#define GPO0_OTYPE                GPIO_MODE_OUT_OD

#define LED_A1                    1
#define GPO1_CLK                  RCU_GPIOC        
#define GPO1_PORT                 GPIOC
#define GPO1_PIN                  GPIO_PIN_4
#define GPO1_VALUE                BSP_GPO_HI
#define GPO1_OTYPE                GPIO_MODE_OUT_OD

#define LED_A2                    2
#define GPO2_CLK                  RCU_GPIOC        
#define GPO2_PORT                 GPIOC
#define GPO2_PIN                  GPIO_PIN_5
#define GPO2_VALUE                BSP_GPO_HI
#define GPO2_OTYPE                GPIO_MODE_OUT_OD

#define LED_ERROR                 3
#define GPO3_CLK                  RCU_GPIOC        
#define GPO3_PORT                 GPIOC
#define GPO3_PIN                  GPIO_PIN_1
#define GPO3_VALUE                BSP_GPO_HI
#define GPO3_OTYPE                GPIO_MODE_OUT_OD

#define LED_COM                   4 
#define GPO4_CLK                  RCU_GPIOC        
#define GPO4_PORT                 GPIOC
#define GPO4_PIN                  GPIO_PIN_2
#define GPO4_VALUE                BSP_GPO_HI
#define GPO4_OTYPE                GPIO_MODE_OUT_OD

#define RS485_EN                  5 
#define GPO5_CLK                  RCU_GPIOA        
#define GPO5_PORT                 GPIOA
#define GPO5_PIN                  GPIO_PIN_11
#define GPO5_VALUE                BSP_GPO_HI
#define GPO5_OTYPE                GPIO_MODE_OUT_OD



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



#endif
