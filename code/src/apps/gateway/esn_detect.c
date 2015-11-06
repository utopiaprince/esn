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
#include "osel_arch.h"
#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

#include "drivers.h"
#include "esn.h"



void esn_detect_task(void *param)
{
    adxl_sensor_init();
    uint16_t time_cnt = 0;
    
    while (1) {
        vTaskDelay(20 / portTICK_RATE_MS);
        //@todo: realy sensor detect
        if (time_cnt++ > 50 * 30) { //*< 30S 采样一次CAM
            time_cnt = 0;
            esn_msg_t esn_msg;
            esn_msg.event = GAIN_CAM_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
            
            esn_msg.event = GAIN_ATMO_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
        }
    }
}

bool_t esn_detect_init(void)
{
    xTaskCreate(esn_detect_task,
                "esn detect task",
                500,
                NULL,
                ESN_DETECT_PRIORITY,
                NULL);

    return TRUE;
}
