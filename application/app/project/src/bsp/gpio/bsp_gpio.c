/**
 * @file bsp_gpio.c
 * @author zgg
 * @brief 
 * @version 1.0.0
 * @date 2022-01-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "bsp_gpio.h"
#include "gd32f30x.h"

typedef struct
{
    rcu_periph_enum gpiox_clk;   // eg: BSP_GPO1_CLK ...
    uint32_t gpio_port;          // eg: BSP_GPO1_PORT ...
    uint32_t gpio_pin;           // eg: BSP_GPO1_PIN ...
    uint32_t gpio_value;         // eg: BSP_GPO1_PIN ...
    uint8_t gpio_otype;
}bsp_gpiox_param_t;

bsp_gpiox_param_t bsp_gpo_param[] =
{
    {GPO0_CLK, GPO0_PORT, GPO0_PIN, GPO0_VALUE, GPO0_OTYPE},
    {GPO1_CLK, GPO1_PORT, GPO1_PIN, GPO1_VALUE, GPO1_OTYPE},
    {GPO2_CLK, GPO2_PORT, GPO2_PIN, GPO2_VALUE, GPO2_OTYPE},
    {GPO3_CLK, GPO3_PORT, GPO3_PIN, GPO3_VALUE, GPO3_OTYPE},
    {GPO4_CLK, GPO4_PORT, GPO4_PIN, GPO4_VALUE, GPO4_OTYPE},
    {GPO5_CLK, GPO5_PORT, GPO5_PIN, GPO5_VALUE, GPO5_OTYPE},
    {GPO6_CLK, GPO6_PORT, GPO6_PIN, GPO6_VALUE, GPO6_OTYPE},
    {GPO7_CLK, GPO7_PORT, GPO7_PIN, GPO7_VALUE, GPO7_OTYPE},
    {GPO8_CLK, GPO8_PORT, GPO8_PIN, GPO8_VALUE, GPO8_OTYPE},
    {GPO9_CLK, GPO9_PORT, GPO9_PIN, GPO9_VALUE, GPO9_OTYPE},                 
    {(rcu_periph_enum)0, 0, 0, 0, 0}
};

bsp_gpiox_param_t bsp_gpi_param[] =
{
    {GPI0_CLK, GPI0_PORT, GPI0_PIN, GPI0_VALUE},
    {GPI1_CLK, GPI1_PORT, GPI1_PIN, GPI1_VALUE},
    {GPI2_CLK, GPI2_PORT, GPI2_PIN, GPI2_VALUE},
    {GPI3_CLK, GPI3_PORT, GPI3_PIN, GPI3_VALUE},
    {GPI4_CLK, GPI4_PORT, GPI4_PIN, GPI4_VALUE},
    {GPI5_CLK, GPI5_PORT, GPI5_PIN, GPI5_VALUE},           
    {(rcu_periph_enum)0, 0, 0, 0}
};


/**
 * @brief pull low the specified gpo
 * 
 * @param[in] index 
 */
void bsp_gpo_low(uint8_t index)
{
    GPIO_BC(bsp_gpo_param[index].gpio_port) = bsp_gpo_param[index].gpio_pin;
}

/**
 * @brief pull high the specified gpo
 * 
 * @param[in] index 
 */
void bsp_gpo_high(uint8_t index)
{
    GPIO_BOP(bsp_gpo_param[index].gpio_port) = bsp_gpo_param[index].gpio_pin;
}

/**
 * @brief the specified gpo status toggle
 * 
 * @param[in] index 
 */
void bsp_gpo_toggle(uint8_t index)
{
    gpio_bit_write(bsp_gpo_param[index].gpio_port, bsp_gpo_param[index].gpio_pin,
                   (bit_status)(1 - gpio_input_bit_get(bsp_gpo_param[index].gpio_port, bsp_gpo_param[index].gpio_pin)));
}

/**
 * @brief init the specified gpo 
 * 
 * @param[in] index 
 */
void bsp_gpo_init(uint8_t index)
{
    /* enable GPO clock */
    rcu_periph_clock_enable(bsp_gpo_param[index].gpiox_clk);
    /* configure GPO port */
    gpio_init(bsp_gpo_param[index].gpio_port, bsp_gpo_param[index].gpio_otype, GPIO_OSPEED_50MHZ, bsp_gpo_param[index].gpio_pin);

    if (bsp_gpo_param[index].gpio_value == BSP_GPO_LO)
    {
        bsp_gpo_low(index);
    }
    else
    {
        bsp_gpo_high(index);
    }
}

/**
 * @brief 
 * 
 * @param[in] index 
 */
void bsp_gpi_poll_init(uint8_t index)
{
    rcu_periph_clock_enable(bsp_gpi_param[index].gpiox_clk);
    //rcu_periph_clock_enable(RCU_AF);

    /* configure button pin as input */
    gpio_init(bsp_gpi_param[index].gpio_port, bsp_gpi_param[index].gpio_value, GPIO_OSPEED_50MHZ, bsp_gpi_param[index].gpio_pin);
}

/**
 * @brief 
 * 
 * @param[in] index 
 * @return uint8_t 
 */
uint8_t bsp_gpi_state_get(uint8_t index)
{
    return gpio_input_bit_get(bsp_gpi_param[index].gpio_port, bsp_gpi_param[index].gpio_pin);
}








