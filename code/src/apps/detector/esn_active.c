/**
 * @brief       : if esn get alarm data, start carma
 * @file        : esn_active.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-10
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-10  v0.0.1  gang.cheng    first version
 */
#include "FreeRTOS.h"
#include "task.h"
#include "esn.h"


void esn_active_task(void *param)
{
    while (1)
    {
        vTaskDelay(20 / portTICK_RATE_MS);
        //@todo: realy sensor detect  
    }
}

bool_t esn_active_init(void)
{
    xTaskCreate(esn_active_task,
                "esn active task",
                1000,
                NULL,
                ESN_ACTIVE_PRIORITY,
                NULL);

    return TRUE;
}










