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
    int16_t x, y, z;
    adxl_sensor_init();
    uint16_t time_cnt = 0;

    while (1) {
        vTaskDelay(20 / portTICK_RATE_MS);
        //adxl_get_xyz(&x, &y, &z);

        //@todo: realy sensor detect
        uint8_t time = 60;
        if (time_cnt++ > 50 * time) { //*< 30S 采样一次CAM
            time_cnt = 0;
            esn_msg_t esn_msg;
            esn_msg.event = GAIN_CAM_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);

            esn_msg.event = GAIN_ATMO_START;
            //xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);

            //@todo: test
#include "esn_package.h"
#define GAIN_DISTANCE_START     ((GAIN_DISTANCE<<8) | 0)
#define GAIN_TEMPERATURE_START  ((GAIN_TEMPERATURE<<8) | 0)
            esn_msg.event = GAIN_STOCK_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
			vTaskDelay(1000 / portTICK_RATE_MS);
            esn_msg.event = GAIN_DISTANCE_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
			vTaskDelay(1000 / portTICK_RATE_MS);
            esn_msg.event = GAIN_TEMPERATURE_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
			vTaskDelay(1000 / portTICK_RATE_MS);
        }
    }
}

bool_t esn_detect_init(void)
{
    xTaskCreate(esn_detect_task,
                "esn detect task",
                200,
                NULL,
                ESN_DETECT_PRIORITY,
                NULL);

    return TRUE;
}
