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

DBG_THIS_MODULE("esn_gain")

QueueHandle_t esn_gain_queue = NULL;

static void camera_recv_data_handle(uint8_t *pdata, uint16_t len)
{
//@todo: SEND data to gprs
}

static void atmos_recv_data_handle(uint8_t *pdata, uint16_t len)
{
//@todo: SEND data to gprs
}

static void esn_gain_task(void *param)
{
    uint8_t type;
    esn_msg_t esn_msg;
    while (1) {
        if(xQueueReceive(esn_gain_queue,
                      &esn_msg,
                      portMAX_DELAY))
        {
            type = esn_msg.event >> 8;
            switch (type) {
            case GAIN_CAM:
                camera_handle(esn_msg.event);
                break;
                
            case GAIN_ATMO:
                atmos_handle(&esn_msg);
                break;

            default:

                break;
            }
        }
    }
}

void esn_gain_init(void)
{
    portBASE_TYPE res;

    res = xTaskCreate(esn_gain_task,
                      "esn_gain_task",
                      300,
                      NULL,
                      ESN_GAIN_PRIORITY,
                      NULL);

    if (res != pdTRUE) {
        DBG_LOG(DBG_LEVEL_ERROR, "esn gain task init failed\r\n");
    }

    esn_gain_queue = xQueueCreate(10, sizeof(esn_msg_t));
    if (esn_gain_queue == NULL) {
        DBG_LOG(DBG_LEVEL_ERROR, "esn_gain_queue init failed\r\n");
    }


    camera_init(CAM_PORT, 115200, esn_gain_queue, camera_recv_data_handle);
    
    atmos_sensor_init(ATMO_PORT, 9600, esn_gain_queue, atmos_recv_data_handle);
}




