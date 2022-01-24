/**
 * @file bsp_uart.h
 * @author zgg
 * @brief 
 * @version 1.0.0
 * @date 2022-01-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __BSP_UART_H__
#define __BSP_UART_H__



void bsp_uart0_485_init(void);

void bsp_uart1_232_init(void);

int fputc(int ch, FILE *f);







#endif
