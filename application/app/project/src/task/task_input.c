/**
 * @file task_input.c
 * @author zgg (linuxgengzhi@163.com)
 * @brief 
 * @version 1.0.0
 * @date 2021-08-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <includes.h>

void app_task_input(void *p_arg)
{
  OS_ERR err;
//  int32_t cur_ticks = 0;
//  int32_t chk_ticks = 0;

  while (DEF_TRUE)
  {
    // OSSemPend(&SemOutput, 0, OS_OPT_PEND_BLOCKING, NULL, &err);    

    // cur_ticks = OSTimeGet(&err);          // get now time

    // if (cur_ticks - chk_ticks > 200)
    // {

    //     chk_ticks = cur_ticks;
    // }
    

    printf("function %04X\r\n", adc_value);
    //printf("This is a input collection task\n");

    // OSSemPost(&SemInput, OS_OPT_POST_ALL, &err);
    OSTimeDlyHMSM(0, 0, 0, 10, OS_OPT_TIME_DLY, &err); 
  }
}






