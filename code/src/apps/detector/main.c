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
#include "lib.h"
#include "bsp.h"
#include "osel_arch.h" 
#include "esn.h"
#include "stack.h"


int main(void)
{
    bsp_init();
    
    esn_detect_init();
    esn_active_init();
    
    stack_init();

    vTaskStartScheduler();

    return 0;
}