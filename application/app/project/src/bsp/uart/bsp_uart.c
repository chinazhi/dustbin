
/**
 * @file bsp_uart.c
 * @author zgg
 * @brief 
 * @version 1.0.0
 * @date 2022-01-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include "gd32f30x.h"
#include "bsp_uart.h"


/**
 * @brief Serial port sends a byte
 * 
 * @param[in] usart_periph 
 * @param[in] ch 
 */
void usart_send_byte(uint32_t usart_periph, uint8_t ch)
{
    /* Sends a byte of data to the USART*/
    usart_data_transmit(usart_periph, ch);

    /* Wait for the send data register to be empty */
    while (RESET == usart_flag_get(usart_periph, USART_FLAG_TBE));
}

/**
 * @brief Serial port sends multiple bytes
 * 
 * @param[in] pucStr Data to be sent
 * @param[in] ulNum  Length to be sent
 */
void uart_send_str_pack(uint32_t usart_periph, char const *pucStr, uint16_t ulNum)
{
    uint32_t i;

    for (i = 0; i < ulNum; i++)
    {
        /* Sends the specified byte of data */
        usart_send_byte(usart_periph, *pucStr++);
    }
}


void bsp_uart0_485_init(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);

    return ;
}


void bsp_uart1_232_init(void)
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART1);

    /* connect port to USARTx_Tx */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

    /* connect port to USARTx_Rx */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    /* USART configure */
    usart_deinit(USART1);
    usart_baudrate_set(USART1, 115200U);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_enable(USART1);

    return ;
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t)ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    return ch;
}
