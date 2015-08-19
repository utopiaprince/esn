/**
 * @brief       : this
 * @file        : mac_prim.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-16
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-16  v0.0.1  gang.cheng    first version
 */

#include "osel_arch.h"
#include "lib.h"
#include "module.h"
#include "mac.h"

#include "sbuf.h"
#include "pbuf.h"

#include "mac_frames.h"
#include "mac_ctrl.h"
#include "mac_prim.h"

DBG_THIS_MODULE("mac_prim")

static mac_line_enum_t mac_line_flag = ON_LINE;

#ifdef NODE_TYPE_DETECTOR
TimerHandle_t mac_line_cycle_timer = NULL;

mac_line_enum_t mac_online_get(void)
{
	return mac_line_flag;
}

bool_t mac_online_set(mac_line_enum_t flag)
{
	hal_int_state_t s;
	HAL_ENTER_CRITICAL(s);
	mac_line_flag = flag;
	HAL_EXIT_CRITICAL(s);

	return TRUE;
}

bool_t mac_online_start(void)
{
	if (mac_online_get() == OFF_LINE)
	{
		osel_event_t msg;
		msg.event = M_PRIM_LINK_REQ_EVENT;
		msg.param = NULL;

		mac_queue_send(&msg);
        
        return TRUE;
	}
    
    return FALSE;
}

static void mac_line_cycle_timeout_cb( TimerHandle_t pxTimer )
{
	configASSERT( pxTimer );

	osel_event_t msg;
	msg.event = M_PRIM_LINK_REQ_EVENT;
	msg.param = NULL;

	mac_queue_send(&msg);
}

#endif


static void mac_prim_line_handle(void)
{
	switch (mac_line_flag)
	{
	case OFF_LINE:
		//@todo if time restart ,what will happend???
		if (xTimerStart(mac_line_cycle_timer, 0) != pdPASS)
		{
			DBG_LOG(DBG_LEVEL_ERROR, "mac line cycle timer restart failed\r\n");
			return;
		}

		mac_online_set(START_LINE);

		mac_ctrl_assoc_req_start(0xFFFF);
		break;

	case START_LINE:
		if (xTimerStart(mac_line_cycle_timer, 0) != pdPASS)
		{
			DBG_LOG(DBG_LEVEL_ERROR, "mac line cycle timer restart failed\r\n");
			return;
		}

		mac_ctrl_assoc_req_start(0xFFFF);
		break;

	case ON_LINE:
		xTimerStop(mac_line_cycle_timer, 0);
		break;

	default:
		break;
	}
}

void m_prim_event_handler(const osel_event_t *const pmsg)
{
	DBG_ASSERT(NULL != pmsg __DBG_LINE);

	if (NULL != pmsg)
	{
		switch (pmsg->event)
		{
		case M_PRIM_DATA_REQ_EVENT:
			//@todo add code for data request
			break;

		case M_PRIM_LINK_REQ_EVENT:
			mac_prim_line_handle();
			break;

		default:
			break;
		}
	}

}

void m_prim_init(void)
{
#ifdef NODE_TYPE_DETECTOR
	mac_online_set(OFF_LINE);

	mac_line_cycle_timer =
	    xTimerCreate("MacLineTimer",
	                 (MAC_LINE_TIMER_CYCLE * configTICK_RATE_HZ),
	                 pdTRUE,
	                 NULL,
	                 mac_line_cycle_timeout_cb);
	if (mac_line_cycle_timer == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn cycle timer create failed\r\n");
	}
	else
	{
		mac_prim_line_handle();
	}
#endif
}
