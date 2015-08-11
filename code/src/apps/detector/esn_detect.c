/**
 * @brief       : this is cycle detect time for sensor
 * @file        : esn_detect.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-05
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-05  v0.0.1  gang.cheng    first version
 */
#include "FreeRTOS.h"
#include "task.h"
#include "esn.h"

void esn_detect_task(void *param)
{
    while (1)
    {
        vTaskDelay(20 / portTICK_RATE_MS);
        //@todo: realy sensor detect  
    }
}

bool_t esn_detect_init(void)
{
    xTaskCreate(esn_detect_task,
                "esn test task",
                1000,
                NULL,
                configMAX_PRIORITIES,
                NULL);

    return TRUE;
}
