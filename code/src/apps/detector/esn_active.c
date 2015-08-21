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
#include "osel_arch.h"
     
#include "sbuf.h"
#include "pbuf.h"

#include "esn.h"
#include "mac.h"
#include "mac_prim.h"
#include "module.h"
     
DBG_THIS_MODULE("esn_active")

static QueueHandle_t esn_active_queue = NULL;
static TimerHandle_t esn_cycle_timer = NULL;

static uint8_t data_seq = 0;

static bool_t distance_data_trigger_flag = FALSE;
static bool_t temp_data_trigger_flag = TRUE;

static bool_t esn_data_send(esn_frames_head_t *esn_frm_hd,
                            const void *const payload,
                            uint8_t len)
{
    uint8_t data_send_cnt = ESN_SEND_WAIT_CNT;
    if (esn_frm_hd->frames_ctrl.alarm == ALARM_N) {
        data_send_cnt = 1;
    }

    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    if (sbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "sbuf alloc failed\n");
        return FALSE;
    }

    pbuf_t *pbuf = pbuf_alloc((sizeof(esn_frames_head_t) + len) __PLINE1);
    if (pbuf == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "pbuf alloc failed\n");
        sbuf_free(&sbuf __SLINE2);
        return FALSE;
    }

    sbuf->orig_layer    = APP_LAYER;
    sbuf->up_down_link  = DOWN_LINK;
    sbuf->primargs.pbuf = pbuf;

    osel_memcpy(pbuf->data_p, esn_frm_hd, sizeof(esn_frames_head_t));
    pbuf->data_len += sizeof(esn_frames_head_t);
    osel_memcpy(pbuf->data_p, payload, len);
    pbuf->data_len += len;

    pbuf->attri.need_ack = ALARM_N;

    osel_event_t msg;
    msg.event = M_PRIM_DATA_REQ_EVENT;
    msg.param = sbuf;

    for (uint8_t i = 0; i < data_send_cnt; i++)
    {
        if (!mac_queue_send(&msg)) {
            continue;
        }

        if (mac_sent_get(ESN_SEND_WAIT_TIME)) { //@note wait for mac send data ok semphore
            return TRUE;
        }

        vTaskDelay(configTICK_RATE_HZ * random(ESN_SEND_BACKOFF_MIN, ESN_SEND_BACKOFF_MAX)); 
    }

    pbuf_free(&pbuf __PLINE2);
    sbuf_free(&sbuf __SLINE2);

    return FALSE;
}

/**
 * @brief
 */
static void esn_timeout_handle(void)
{
    esn_frames_head_t esn_frames_head;

    if (distance_data_trigger_flag)
    {
        distance_data_trigger_flag = FALSE;
    }
    else
    {
        //@note send distance_data
        esn_frames_head.frames_ctrl.frame_type = DATATYPE_DISTANCE;
        esn_frames_head.frames_ctrl.alarm      = ALARM_N;
        esn_frames_head.frames_ctrl.reserved   = 0;
        esn_frames_head.seq = data_seq++;
        //@todo: need get distance sensor data
        esn_distance_payload_t esn_distance_pd;
        esn_distance_pd.distance = sensor_distance_get();

        esn_data_send(&esn_frames_head,
                      &esn_distance_pd,
                      sizeof(esn_distance_payload_t));
    }

    if (temp_data_trigger_flag)
    {
        temp_data_trigger_flag = FALSE;
    }
    else
    {
        esn_frames_head.frames_ctrl.frame_type = DATATYPE_TEMPERATURE;
        esn_frames_head.frames_ctrl.alarm      = ALARM_N;
        esn_frames_head.frames_ctrl.reserved   = 0;
        esn_frames_head.seq = data_seq++;

        esn_temp_payload_t esn_temp_pd;
        esn_temp_pd.temperature = sensor_temp_get();

        esn_data_send(&esn_frames_head,
                      &esn_temp_pd,
                      sizeof(esn_temp_payload_t));
    }
}

static void esn_vibration_handle(void)
{
    esn_frames_head_t esn_frames_head;

    esn_frames_head.frames_ctrl.frame_type = DATATYPE_VIBRATION;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.reserved   = 0;
    esn_frames_head.seq = data_seq++;

    esn_data_send(&esn_frames_head,
                  NULL,
                  0);


    //@todo start carma data stream
    //...
}

static void esn_distance_handle(void)
{
    esn_frames_head_t esn_frames_head;

    esn_frames_head.frames_ctrl.frame_type = DATATYPE_DISTANCE;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.reserved   = 0;
    esn_frames_head.seq = data_seq++;

    distance_data_trigger_flag = TRUE;

    esn_distance_payload_t esn_distance_pd;
    esn_distance_pd.distance = sensor_distance_get();

    esn_data_send(&esn_frames_head,
                  &esn_distance_pd,
                  sizeof(esn_distance_payload_t));


    //@todo start carma data stream
    //...
}

static void esn_temp_handle(void)
{
    esn_frames_head_t esn_frames_head;

    esn_frames_head.frames_ctrl.frame_type = DATATYPE_TEMPERATURE;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.reserved   = 0;
    esn_frames_head.seq = data_seq++;

    temp_data_trigger_flag = TRUE;

    esn_temp_payload_t esn_temp_pd;
    esn_temp_pd.temperature = sensor_temp_get();

    esn_data_send(&esn_frames_head,
                  &esn_temp_pd,
                  sizeof(esn_temp_payload_t));
}

static void esn_active_task(void *param)
{
    esn_msg_t esn_msg;
    while (1)
    {
        xQueueReceive(esn_active_queue,
                      &esn_msg,
                      portMAX_DELAY);

        if (mac_online_get() != ON_LINE)
        {
            DBG_LOG(DBG_LEVEL_WARNING, "mac offline, data lost\r\n");
            mac_online_start();
        }
        else
        {
            DBG_LOG(DBG_LEVEL_WARNING, "mac online, data event\r\n");
            switch (esn_msg.event)
            {
            case ESN_TIMEOUT_EVENT:
                esn_timeout_handle();
                break;

            case ESN_VIBRATION_EVENT:
                esn_vibration_handle();
                break;

            case ESN_DISTANCE_EVENT:
                esn_distance_handle();
                break;

            case ESN_TEMP_EVENT:
                esn_temp_handle();
                break;

            case ESN_UART_EVENT:
                //@todo
                break;

            case ESN_CONFIG_EVENT:
                //@todo
                break;

            default:
                break;
            }
        }
    }
}

bool_t esn_active_queue_send(esn_msg_t *esn_msg)
{
    portBASE_TYPE res = pdTRUE;

    res = xQueueSendToBack(esn_active_queue, esn_msg, 0);
    if (res == errQUEUE_FULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn active queue is full\r\n");
        return FALSE;
    }

    return TRUE;
}

void esn_cycle_timeout_cb( TimerHandle_t pxTimer )
{
    configASSERT( pxTimer );

    esn_msg_t esn_msg;
    esn_msg.event = ESN_TIMEOUT_EVENT;
    esn_msg.param = NULL;
    esn_active_queue_send(&esn_msg);
}


bool_t esn_active_init(void)
{
    portBASE_TYPE res = pdTRUE;
    res = xTaskCreate(esn_active_task,
                      "EsnActiveTask",
                      500,
                      NULL,
                      ESN_ACTIVE_PRIORITY,
                      NULL);
    if (res != pdTRUE)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn active task init failed\r\n");
    }

    esn_active_queue = xQueueCreate(10, sizeof(esn_msg_t));
    if (esn_active_queue == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn_active_queue init failed\r\n");
    }

    esn_cycle_timer = xTimerCreate("EsnTimer",
                                   (ESN_TIMER_CYCLE * configTICK_RATE_HZ),
                                   pdTRUE,
                                   NULL,
                                   esn_cycle_timeout_cb);
    if (esn_cycle_timer == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn cycle timer create failed\r\n");
    }
    else
    {
        if (xTimerStart(esn_cycle_timer , 0) != pdPASS)
        {
            DBG_LOG(DBG_LEVEL_ERROR, "esn cycle timer start failed\r\n");
        }
    }

    return TRUE;
}










