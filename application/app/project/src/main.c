/**
 * @file main.c
 * @author zgg 
 * @brief 
 * @version 1.0.0
 * @date 2022-01-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "includes.h"

// Ucos III Task Stack
CPU_STK app_task_start_stk[APP_TASK_START_STK_SIZE];
CPU_STK app_task_state_stk[APP_TASK_STATE_STK_SIZE];

// Ucos III Task Control Block
OS_TCB app_task_start_tcb;
OS_TCB app_task_state_tcb;

// Funtion prototypes
static void app_task_start(void *pvParameters);
static void app_task_state(void *pvParameters);

/**
 * @brief main
 * 
 * @return int 
 */
int main(void)
{
    OS_ERR err;
    // set nvic adderss
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x00);
    /* Configures the priority grouping: 4 bits pre-emption priority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
    // enable clock
    rcu_periph_clock_enable(RCU_PMU);
    
    SysTick_Config(SystemCoreClock / OS_CFG_TICK_RATE_HZ);
    
    OSInit(&err);

    if (OS_ERR_NONE != err)
    {
        while (1);
    }

    /* first task */
    OSTaskCreate((OS_TCB *)&app_task_start_tcb,
                 (CPU_CHAR *)"app first task start",
                 (OS_TASK_PTR)app_task_start,
                 (void *)0,
                 (OS_PRIO)APP_TASK_START_PRIO,
                 (CPU_STK *)&app_task_start_stk[0],
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP),
                 (OS_ERR *)&err);
    /* Start scheduler */
    OSStart(&err);
}

/**
 * @brief Create multiple tasks
 *
 * @param[in] pvParameters not used
 */
void app_task_start(void *pvParameters)
{
    OS_ERR err;

    OSStatTaskCPUUsageInit((OS_ERR *)&err);
    
    bsp_gpo_init(LED_A0);
//    bsp_gpo_init(LED_A1);
//    bsp_gpo_init(LED_A2);
//    bsp_gpo_init(LED_COM);
//    bsp_gpo_init(LED_ERROR);
      bsp_gpo_init(RS485_EN); 
      gpio_bit_write(GPIOA, GPIO_PIN_11, SET); 
      bsp_uart0_485_init();
      bsp_uart1_232_init();
    //ds18b20_init();

    OSTaskCreate((OS_TCB *)&app_task_state_tcb,
                 (CPU_CHAR *)"monitoring application status",
                 (OS_TASK_PTR)app_task_state,
                 (void *)0,
                 (OS_PRIO)APP_TASK_STATE_PRIO,
                 (CPU_STK *)&app_task_state_stk[0],
                 (CPU_STK_SIZE)APP_TASK_STATE_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_STATE_STK_SIZE,
                 (OS_MSG_QTY)0,
                 (OS_TICK)0,
                 (void *)0,
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                 (OS_ERR *)&err);

    OSTaskDel((OS_TCB *)0, &err);
    // Not Come Here Forever
    for (;;)
    {
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_PERIODIC, &err);
    }
}

float my_temp;

/**
 * @brief task
 * 
 * @param[in] pvParameters 
 */
void app_task_state(void *pvParameters)
{
    OS_ERR err;
    
    while (1)
    {
        bsp_gpo_toggle(LED_A0);
//        bsp_gpo_toggle(LED_A1);
//        bsp_gpo_toggle(LED_A2);
//        bsp_gpo_toggle(LED_ERROR);
//        bsp_gpo_toggle(LED_COM);
        //my_temp = ds18b20_read_temp();
        printf("0102030405\r\n");
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_PERIODIC, &err);
    }
}


