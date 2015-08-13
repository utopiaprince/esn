/**
 * @brief       : this
 * @file        : mac.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-12
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-12  v0.0.1  gang.cheng    first version
 */

#include "osel_arch.h"

#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

#ifdef NODE_TYPE_DETECTOR
#ifdef NODE_TYPE_GATEWAY
#error "NODE_TYPE_DETECTOR and NODE_TYPE_GATEWAY must define ONE"
#endif
#endif

DBG_THIS_MODULE("mac")

xQueueHandle mac_queue = NULL;

bool_t mac_status = FALSE;  //*< 表示当前网络是否已经在网

static void mac_task(void *p)
{
    osel_event_t *event = NULL;
    while (1)
    {
        xQueueReceive(mac_queue,        //*< the handle of received queue
                      event,            //*< pointer to data received
                      portMAX_DELAY);   //*< time out

    }
}

bool_t mac_queue_send(osel_event_t *event)
{
    portBASE_TYPE res = pdTRUE;

    res = xQueueSendToBack(mac_queue, event, 0); //*< send wait for 10s max
    if (res == errQUEUE_FULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "mac queue is full\r\n");
        return FALSE;
    }

    return TRUE;
}


void mac_init(void)
{
    portBASE_TYPE res = pdTRUE;
    res = xTaskCreate(mac_task,                   //*< task body
                      "MacTask",                  //*< task name
                      200,                        //*< task heap
                      NULL,                       //*< tasK handle param
                      configMAX_PRIORITIES - 2,   //*< task prio
                      NULL);                      //*< task pointer
    if (res != pdTRUE)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "mac task init failed\r\n");
    }

    mac_queue = xQueueCreate(10, sizeof(osel_event_t));
    if (mac_queue == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "mac_queue init failed\r\n");
    }
    
#ifdef NODE_TYPE_GATEWAY
    mac_status = TRUE;
#endif
}










