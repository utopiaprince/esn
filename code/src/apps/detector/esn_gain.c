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

/**
* @note 每次camera接收的数据都是512字节一包，需要按照128字节拆分成4包
*/
static void camera_recv_data_handle(uint16_t cnt,
                                    uint16_t index,
                                    uint8_t *pdata,
                                    uint16_t len)
{
    pbuf_t *pbuf = pbuf_alloc(sizeof(esn_camera_payload_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    esn_camera_payload_t *datap = pbuf->datap;
    datap->cnt = cnt * 4;
    for (uint8_t i = 0; i < 4; i++)
    {
        datap->index = 4 * (index - 1) + 1 + i;
        osel_memcpy(datap->buf, (pdata + 128 * i), 128);
        pbuf->data_len = sizeof(esn_camera_payload_t);

        esn_msg_t esn_msg;
        esn_msg.event = DATATYPE_PICTURE;
        esn_msg.param = pbuf;
        xQueueSend(esn_active_queue, &esn_msg, portMAX_DELAY);
    }
}

static void atmos_recv_data_handle(uint8_t *pdata, uint16_t len)
{
    pbuf_t *pbuf = pbuf_alloc(len __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    osel_memcpy(pbuf->datap, pdata, len);
    pbuf->datap += len;
    pbuf->data_len += len;

    esn_msg_t esn_msg;
    esn_msg.event = DATATYPE_ATMO;
    esn_msg.param = pbuf;
    xQueueSend(esn_active_queue, &esn_msg, portMAX_DELAY);
}

static bool_t range_recv_data_handle(fp32_t distance)
{
    pbuf_t *pbuf = pbuf_alloc(sizeof(esn_distance_payload_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    osel_memcpy(pbuf->datap, (void *)&distance, sizeof(distance));
    pbuf->data_len += sizeof(distance);

    esn_msg_t esn_msg;
    esn_msg.event = DATATYPE_DISTANCE;
    esn_msg.param = pbuf;
    xQueueSend(esn_active_queue, &esn_msg, portMAX_DELAY);
    return TRUE;
}

static void angle_handle(void)
{
    int16_t x, y, z;
    adxl_get_triple_angle(&x, &y, &z);

    pbuf_t *pbuf = pbuf_alloc(sizeof(esn_angle_payload_t) __PLINE1);
    DBG_ASSERT(pbuf != NULL __DBG_LINE);

    osel_memcpy(pbuf->datap, (uint8_t *)&x, sizeof(int16_t));
    pbuf->datap += sizeof(int16_t);
    pbuf->data_len += sizeof(int16_t);

    osel_memcpy(pbuf->datap, (uint8_t *)&y, sizeof(int16_t));
    pbuf->datap += sizeof(int16_t);
    pbuf->data_len += sizeof(int16_t);

    osel_memcpy(pbuf->datap, (uint8_t *)&z, sizeof(int16_t));
    pbuf->datap += sizeof(int16_t);
    pbuf->data_len += sizeof(int16_t);

    esn_msg_t esn_msg;
    esn_msg.event = DATATYPE_VIBRATION;
    esn_msg.param = pbuf;
    xQueueSend(esn_active_queue, &esn_msg, portMAX_DELAY);
}

static void shock_handle(void)
{
    bool_t stock_can_sent = FALSE;
    static TickType_t stock_old_tick = 0; //*< 4字节
    static TickType_t stock_new_tick = 0;

    stock_new_tick = xTaskGetTickCount();
    if (stock_old_tick == 0)
    {
        stock_old_tick = stock_new_tick;
        stock_can_sent = TRUE;
    }

    if (stock_new_tick > stock_old_tick)
    {
        //*< 30S以内只触发一次
        if ((stock_new_tick - stock_old_tick) > (30 * configTICK_RATE_HZ))
        {
            stock_old_tick = stock_new_tick;
            stock_can_sent = TRUE;
        }
    }
    else
    {
        if (((portMAX_DELAY - stock_old_tick) + stock_new_tick) > (30 * configTICK_RATE_HZ))
        {
            stock_old_tick = stock_new_tick;
            stock_can_sent = TRUE;
        }
    }

    if (stock_can_sent)
    {
        stock_can_sent = FALSE;

        esn_msg_t shock_msg;
        shock_msg.event = DATATYPE_VIBRATION;
        shock_msg.param = NULL;
        xQueueSend(esn_active_queue, &shock_msg, portMAX_DELAY);

        //@note 启动摄像头采集数据
        esn_msg_t cam_msg;
        cam_msg.event = GAIN_CAM_START;
        xQueueSend(esn_gain_queue, &cam_msg, portMAX_DELAY);
    }
}

static void esn_gain_task(void *param)
{
    uint8_t type;
    esn_msg_t esn_msg;

    while (1)
    {
        if (xQueueReceive(esn_gain_queue,
                          &esn_msg,
                          portMAX_DELAY))
        {
            type = esn_msg.event >> 8;
            switch (type)
            {
            case GAIN_CAM:
                camera_handle(esn_msg.event);
                break;
                
            case GAIN_ATMO:
                atmos_handle(&esn_msg);
                break;

            case GAIN_RANGE:
                range_handle(&esn_msg);
                break;

            case GAIN_ANGLE:
                angle_handle();
                break;

            case GAIN_STOCK:
                shock_handle();
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








