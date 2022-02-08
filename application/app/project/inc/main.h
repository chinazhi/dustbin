/**
 * @file main.h
 * @author zgg 
 * @brief 
 * @version 1.0.0
 * @date 2022-01-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MAIN_H
#define MAIN_H

// UCOS III TASK PRIORITIES
#define APP_TASK_START_PRIO              (OS_CFG_PRIO_MAX - 4u)
#define APP_TASK_STATE_PRIO              10u
#define APP_TASK_INPUT_PRIO              20u


// UCOS III TASK STACK SIZES
#define APP_TASK_START_STK_SIZE          256
#define APP_TASK_STATE_STK_SIZE          256
#define APP_TASK_INPUT_STK_SIZE          256

#endif /* MAIN_H */
