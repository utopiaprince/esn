/**
 * @brief       : this is lora-network application. 
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

QueueHandle_t esn_active_queue = NULL;

static uint8_t data_seq = 0;

static bool_t esn_data_send(esn_frames_head_t *esn_frm_hd,
                            const void *const payload,
                            uint8_t len)
{
    uint8_t data_send_cnt = ESN_SEND_WAIT_CNT;
    if (esn_frm_hd->frames_ctrl.alarm == ALARM_N) {
        data_send_cnt = 1;
    }

    sbuf_t *sbuf = sbuf_alloc(__SLINE1);
    if (sbuf == NULL) {
        DBG_LOG(DBG_LEVEL_ERROR, "sbuf alloc failed\n");
        return FALSE;
    }

    pbuf_t *pbuf = pbuf_alloc((sizeof(esn_frames_head_t) + len) __PLINE1);
    if (pbuf == NULL) {
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

    pbuf->attri.need_ack = esn_frm_hd->frames_ctrl.alarm;

    osel_event_t msg;
    msg.event = M_PRIM_DATA_REQ_EVENT;
    msg.param = sbuf;

    for (uint8_t i = 0; i < data_send_cnt; i++) {
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

static void esn_camera_handle(pbuf_t *pbuf)
{
    esn_frames_head_t esn_frames_head;

    esn_frames_head.frames_ctrl.frame_type = ESN_DATA;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.data_type  = DATATYPE_PICTURE;
    esn_frames_head.seq = data_seq++;
    esn_frames_head.nums = 1;
    esn_frames_head.sub_num = 1;

    esn_camera_payload_t esn_camera_pd;

    esn_data_send(&esn_frames_head, &esn_camera_pd, sizeof(esn_camera_payload_t));
}

static void esn_vibration_handle(void)
{
    esn_frames_head_t esn_frames_head;

    esn_frames_head.frames_ctrl.frame_type = ESN_DATA;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.data_type  = DATATYPE_VIBRATION;
    esn_frames_head.seq = data_seq++;
    esn_frames_head.nums = 1;
    esn_frames_head.sub_num = 1;

    esn_vibration_payload_t esn_vib_pd;
    //@todo change by realy data
    esn_vib_pd.time_duty = 0xA5;
    esn_vib_pd.alx       = 0x20;

    esn_data_send(&esn_frames_head, &esn_vib_pd, sizeof(esn_vib_pd));
}

static void esn_distance_handle(pbuf_t *pbuf)
{
    esn_frames_head_t esn_frames_head;

    esn_frames_head.frames_ctrl.frame_type = ESN_DATA;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.data_type  = DATATYPE_DISTANCE;
    esn_frames_head.seq = data_seq++;
    esn_frames_head.nums = 1;
    esn_frames_head.sub_num = 1;
    
    esn_distance_payload_t esn_distance_pd;
    if(pbuf != NULL)
    {
        osel_memcpy(&esn_distance_pd, pbuf->head, sizeof(esn_distance_payload_t));
        pbuf_free(&pbuf __PLINE2);
    }
    else
    {
        esn_distance_pd.distance = 2.0;
    }

    esn_data_send(&esn_frames_head, &esn_distance_pd,
                  sizeof(esn_distance_payload_t));
}


static void esn_angle_handle(pbuf_t *pbuf)
{
    esn_frames_head_t esn_frames_head;
    
    esn_frames_head.frames_ctrl.frame_type = ESN_DATA;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.data_type  = DATATYPE_ANGLE;
    esn_frames_head.seq = data_seq++;
    esn_frames_head.nums = 1;
    esn_frames_head.sub_num = 1;
    
    esn_angle_payload_t esn_angle_pd;
    
    if(pbuf != NULL)
    {
        osel_memcpy(&esn_angle_pd, pbuf->head, sizeof(esn_angle_payload_t));
        pbuf_free(&pbuf __PLINE2);
    }
    else
    {
        return;
    }

    esn_data_send(&esn_frames_head, &esn_angle_pd,
                  sizeof(esn_angle_payload_t));
}


static void esn_atmo_handle(pbuf_t *pbuf)
{
    esn_frames_head_t esn_frames_head;
    
    esn_frames_head.frames_ctrl.frame_type = ESN_DATA;
    esn_frames_head.frames_ctrl.alarm      = ALARM_T;
    esn_frames_head.frames_ctrl.data_type  = DATATYPE_ATMO;
    esn_frames_head.seq = data_seq++;
    esn_frames_head.nums = 1;
    esn_frames_head.sub_num = 1;
    
    uint8_t atmo_buf[100];
    uint8_t len = 0;
    if(pbuf != NULL)
    {
        len = pbuf->data_len;
        osel_memcpy(atmo_buf, pbuf->head, len);
        pbuf_free(&pbuf __PLINE2);
    }
    else
    {
        return;
    }

    esn_data_send(&esn_frames_head, atmo_buf, len);
}

static void esn_data_event_handle(esn_msg_t *esn_msg)
{
    uint8_t data_type = esn_msg->event & 0xFF;
    switch(data_type)
    {
    case DATATYPE_VIBRATION:
        esn_vibration_handle();
        break;

    case DATATYPE_DISTANCE:
        esn_distance_handle(esn_msg->param);
        break;
    
    case DATATYPE_PICTURE:
        esn_camera_handle(esn_msg->param);
        break;

    case DATATYPE_ANGLE:
        esn_angle_handle(esn_msg->param);
        break;

    case DATATYPE_ATMO:
        esn_atmo_handle(esn_msg->param);
        break;
        
    default:
        break;
    }
}

static void esn_ssn_event_handle(esn_msg_t *esn_msg)
{
    //@todo 从无线网络接收到的数据处理函数
    ;
}

static void esn_active_task(void *param)
{
    esn_msg_t esn_msg;

    while (1) {
        xQueueReceive(esn_active_queue,
                      &esn_msg,
                      portMAX_DELAY);

        if (mac_online_get() != ON_LINE) {
            DBG_LOG(DBG_LEVEL_WARNING, "mac offline, data lost\r\n");
            mac_online_start();
        } else {
            DBG_LOG(DBG_LEVEL_WARNING, "mac online, data event\r\n");
            uint16_t event_type = esn_msg.event >> 8;
            switch (event_type) {
            case ESN_DATA_EVENT:
                esn_data_event_handle(&esn_msg);
                break;
                
            case ESN_SSN_EVENT:
                esn_ssn_event_handle(&esn_msg);
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
    if (res == errQUEUE_FULL) {
        DBG_LOG(DBG_LEVEL_ERROR, "esn active queue is full\r\n");
        return FALSE;
    }

    return TRUE;
}

bool_t esn_active_init(void)
{
    portBASE_TYPE res = pdTRUE;
    res = xTaskCreate(esn_active_task,
                      "EsnActiveTask",
                      300,
                      NULL,
                      ESN_ACTIVE_PRIORITY,
                      NULL);
    if (res != pdTRUE) {
        DBG_LOG(DBG_LEVEL_ERROR, "esn active task init failed\r\n");
    }

    esn_active_queue = xQueueCreate(10, sizeof(esn_msg_t));
    if (esn_active_queue == NULL) {
        DBG_LOG(DBG_LEVEL_ERROR, "esn_active_queue init failed\r\n");
    }

    return TRUE;
}










