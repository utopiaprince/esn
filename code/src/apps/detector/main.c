/**
 * @brief       : 
 *
 * @file        : main.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/8/10
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/8/10    v0.0.1      gang.cheng    first version
 */

#include "FreeRTOS.h"
#include "task.h"

#include "esn.h"


int main(void)
{
    
    esn_detect_init();


    vTaskStartScheduler();

    while(1);
    return 0;
}