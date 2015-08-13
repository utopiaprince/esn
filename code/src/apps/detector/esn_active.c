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
#include "esn.h"

static QueueHandle_t esn_active_queue = NULL;
static TimerHandle_t esn_cycle_timer = NULL;


void esn_active_task(void *param)
{
    esn_msg_t esn_msg;
    while (1)
    {
        xQueueReceive(esn_active_queue,
                      &esn_msg,
                      portMAX_DELAY);

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

        default:
            break;
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
    int32_t lArrayIndex;
    const int32_t xMaxExpiryCountBeforeStopping = 10;

    configASSERT( pxTimer );

    esn_msg_t esn_msg;
    esn_msg.event = ESN_TIMEOUT_EVENT;
    esn_msg.param = 0;
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










