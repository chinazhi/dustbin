/**
 * @file common.c
 * @author zgg 
 * @brief 
 * @version 1.0.0
 * @date 2022-01-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "includes.h"


// void system_gpio_init(void)
// {
//   unsigned char i = 0;
  
//   while (bsp_gpo_param[i].gpio_port != 0)
//   {
//     bsp_gpo_init(i);
//     i++;
//   }

//   //GPI
//   i = 0;
//   while (bsp_gpi_param[i].gpio_port != 0)
//   {
//     bsp_gpi_poll_init(i);
//     i++;
//   }
// }

void system_hw_init(void)
{
    bsp_gpi_poll_init(SMOKE_EN);
    bsp_gpi_poll_init(POS_F_EN);
    bsp_gpi_poll_init(POS_B_EN);
    bsp_gpi_poll_init(LED_BTN_IN_EN);
    bsp_gpi_poll_init(FULL_IN_EN);
    bsp_gpi_poll_init(BODY_IN_EN);

    bsp_gpo_init(LED_A0);
    // bsp_gpo_init(LED_A1);
    // bsp_gpo_init(LED_A2);
    // bsp_gpo_init(LED_COM);
    // bsp_gpo_init(LED_ERROR);
    bsp_gpo_init(DO_FAN_EN);
    bsp_gpo_init(DO_LIGHT_EN);
    bsp_gpo_init(FULL_OUT_EN);
    bsp_gpo_init(BTN_OUT_EN);
    bsp_gpo_init(RS485_EN);
    bsp_uart0_485_init();
    bsp_uart1_232_init();
    // ds18b20_init();

    motor_power_adc_init();
}

float my_temp = 0;

void zgg_gpo_test(void)
{
    // bsp_gpo_toggle(LED_A1);
    // bsp_gpo_toggle(LED_A2);
    // bsp_gpo_toggle(LED_ERROR);
    // bsp_gpo_toggle(LED_COM);
    bsp_gpo_high(DO_FAN_EN);
    bsp_gpo_high(DO_LIGHT_EN);
}



void zgg_gpi_test(void)
{
    if (bsp_gpi_state_get(SMOKE_EN))
    {
        printf("SMOKE_EN ");
    }
    if (bsp_gpi_state_get(POS_F_EN))
    {
        printf("POS_F_EN ");
    }
    if (bsp_gpi_state_get(POS_B_EN))
    {
        printf("POS_B_EN ");
    }

    if (bsp_gpi_state_get(LED_BTN_IN_EN))
    {
        bsp_gpo_high(BTN_OUT_EN);
        printf("LED_BTN_IN_EN ");
    }
    else
    {
        bsp_gpo_low(BTN_OUT_EN);
    }
    if (bsp_gpi_state_get(FULL_IN_EN))
    {
        bsp_gpo_high(FULL_OUT_EN);
        printf("FULL_IN_EN ");
    }
    else
    {
        bsp_gpo_low(FULL_OUT_EN);
    }
    
    if (bsp_gpi_state_get(BODY_IN_EN))
    {
        printf("BODY_IN_EN ");
    }
    printf("\r\n");                   
}

void hardware_function_test(void)
{
    bsp_gpo_toggle(LED_A0);
    zgg_gpo_test();
    zgg_gpi_test();

    // my_temp = ds18b20_read_temp();

    printf("function %04X\r\n", adc_value);
}
