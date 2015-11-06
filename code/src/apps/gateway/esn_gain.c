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
#include "esn_package.h"
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

static void shock_recv_data_handle(uint8_t *pdata, uint16_t len)       //震动：被监测ID、采集时间
{
//@todo: SEND data to gprs
    if (len > sizeof(esn_part_t))
        return;
    esn_part_t info;
    osel_memcpy(&info, pdata, len);
    pbuf_t *pbuf = shock_package(&info);

    pbuf_free(&pbuf __PLINE2);
}

static void distance_recv_data_handle(uint8_t *pdata, uint16_t len)    //距离：被监测ID、采集时间、距离(float)
{
//@todo: SEND data to gprs
    if (len > sizeof(esn_part_t))
        return;
    esn_part_t info;
    osel_memcpy(&info, pdata, len);
    pbuf_t *pbuf = distance_package(&info);

    pbuf_free(&pbuf __PLINE2);
}

static void temperature_recv_data_handle(uint8_t *pdata, uint16_t len)  //导线温度：被监测ID、采集时间、温度(float)
{
//@todo: SEND data to gprs
    if (len > sizeof(esn_part_t))
        return;
    esn_part_t info;
    osel_memcpy(&info, pdata, len);
    pbuf_t *pbuf = temperature_package(&info);

    pbuf_free(&pbuf __PLINE2);
}

static void esn_gain_task(void *param)
{
    uint8_t type;
    esn_msg_t esn_msg;
    while (1) {
        if (xQueueReceive(esn_gain_queue,
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
            case GAIN_STOCK:
            {
                esn_part_t info;
                osel_memset( info.bmonitor, 0, 17);
                info.bmonitor[0] = 0xbb;
                info.collect_time = 100;
                shock_recv_data_handle((uint8_t *)&info, sizeof(esn_part_t));
                break;
            }
            case GAIN_DISTANCE:
            {
                esn_part_t info;
                osel_memset( info.bmonitor, 0, 17);
                info.bmonitor[0] = 0xbb;
                info.collect_time = 100;
                info.val = 13.14;
                distance_recv_data_handle((uint8_t *)&info, sizeof(esn_part_t));
                break;
            }
            case GAIN_TEMPERATURE:
            {
                esn_part_t info;
                osel_memset( info.bmonitor, 0, 17);
                info.bmonitor[0] = 0xbb;
                info.collect_time = 100;
                info.val = 20.21;
                temperature_recv_data_handle((uint8_t *)&info, sizeof(esn_part_t));
                break;
            }
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




