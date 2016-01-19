/**
* @brief       : this is cycle detect time for sensor
* @file        : esn_detect.c
* @version     : v0.0.1
* @author      : gang.cheng
* @date        : 2016-01-18
* change logs  :
* Date       Version     Author        Note
* 2016-01-18  v0.0.1  gang.cheng    first version
*/
#include "osel_arch.h"
#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

#include "drivers.h"
#include "esn.h"

DBG_THIS_MODULE("esn_gain")


QueueHandle_t esn_gain_queue = NULL;

static void esn_gain_task(void *param)
{
    uint8_t type;
    esn_msg_t esn_msg;
    
    while(1)
    {
        if (xQueueReceive(esn_gain_queue,
                          &esn_msg,
                          portMAX_DELAY))
        {
            type = esn_msg.event >> 8;
            switch (type)
            {
            case :
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
                      400,
                      NULL,
                      ESN_GAIN_PRIORITY,
                      NULL);
    
    if (res != pdTRUE)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn gain task init failed\r\n");
    }
    
    esn_gain_queue = xQueueCreate(15, sizeof(esn_msg_t));
    if (esn_gain_queue == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn_gain_queue init failed\r\n");
    }
    
    adxl_sensor_init(); //*< 被动接收数据

    atmos_sensor_init(UART_1, 9600, esn_gain_queue, atmos_recv_data_handle);
    camera_init(UART_2, 9600, esn_gain_queue, camera_recv_data_handle);
    range_sensor_init(UART_3, 9600, esn_gain_queue, range_recv_data_handle);
}








