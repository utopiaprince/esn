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

#include "drivers.h"

#include "module.h"
#include "m_tran.h"
#include "mac_prim.h"
#include "mac_recv.h"

#ifdef NODE_TYPE_DETECTOR

#elif NODE_TYPE_GATEWAY

#else
#error "NODE_TYPE_DETECTOR and NODE_TYPE_GATEWAY must define ONE"
#endif

DBG_THIS_MODULE("mac")

uint16_t mac_short_addr = 0x0101;
uint64_t mac_long_addr  = 0xaabbccddeeff0101;

uint8_t mac_seq = 0;
static xQueueHandle mac_queue = NULL;
static xSemaphoreHandle mac_sent = NULL;

static void mac_task(void *p)
{
    osel_event_t msg;
    while (1)
    {
        xQueueReceive(mac_queue,        //*< the handle of received queue
                      &msg,            //*< pointer to data received
                      portMAX_DELAY);   //*< time out

        switch (msg.event & MAC_MODUEL_ENUM_MASK)
        {
        case TRAN_MODULE:
            m_tran_event_handler(&msg);
            break;

        case PRIM_MODULE:
            m_prim_event_handler(&msg);
            break;

        default:
            break;
        }
    }
}

bool_t mac_queue_send(osel_event_t *msg)
{
    portBASE_TYPE res = xQueueSendToBack(mac_queue, msg, (10 * configTICK_RATE_HZ)); //*< send wait for 10s max
    if (res == errQUEUE_FULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "mac queue is full\r\n");
        return FALSE;
    }

    return TRUE;
}

bool_t mac_queue_send_from_isr(osel_event_t *msg)
{
    portBASE_TYPE res = xQueueSendToBackFromISR(mac_queue, msg, 0); //*< send wait for 10s max
    if (res == errQUEUE_FULL)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief get send flag wait for sec
 * @param  sec time of secends wait for send flag
 * @return
 *  - TRUE has sent ok
 *  - FALSE sent failed
 */
bool_t mac_sent_get(uint16_t sec)
{
    portBASE_TYPE res;
    res = xSemaphoreTake(mac_sent, (sec * configTICK_RATE_HZ));

    if (res == pdPASS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool_t mac_sent_set(void)
{
    xSemaphoreGive(mac_sent);
    return TRUE;
}


void mac_init(void)
{
    lora_init(UART_4, 9600);
    
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

    mac_sent = xSemaphoreCreateBinary();
    if (mac_sent == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "mac_set init failed\r\n");

    }
    
    hal_timer_init();
    m_tran_init();
    m_prim_init();
    m_recv_init();
}










