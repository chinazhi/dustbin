/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2017-02-10, V1.0.0, firmware for GD32F30x
    \version 2018-10-10, V1.1.0, firmware for GD32F30x
    \version 2018-12-25, V2.0.0, firmware for GD32F30x
    \version 2020-09-30, V2.1.0, firmware for GD32F30x 
*/

#include "gd32f30x.h"
#include "systick.h"

volatile static uint32_t delay;

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U)){
        /* capture error */
        while (1){
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief      delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    unsigned char i, j, k;
    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();

    for (i = 288; i > 0; i--)
    {
        for (j = 10; j > 0; j--)
        {
            for (k = count; k > 0; k--)
                ;
        }
    }
    OS_CRITICAL_EXIT();
    // delay = count;

    // while(0U != delay){
    // }
}

/*!
    \brief      delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    if (0U != delay){
        delay--;
    }
}

void delay_1us(unsigned int data)
{
    unsigned int i, j;

    CPU_SR_ALLOC();
    OS_CRITICAL_ENTER();
    for (i = 28; i > 0; i--)
    {
        for (j = data; j > 0; j--)
        {
            ;
        }
    }
    OS_CRITICAL_EXIT();
}
