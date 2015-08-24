#include "osel_arch.h"
#include "lib.h"

#include "prim.h"
#include "esn_frames.h"
#include "esn_active.h"

DBG_THIS_MODULE("esn_active")

static QueueHandle_t esn_active_queue = NULL;

static void esn_active_task(void *param)
{
	esn_msg_t esn_msg;
	while (1)
	{
		xQueueReceive(esn_active_queue,
		              &esn_msg,
		              portMAX_DELAY);

		switch (esn_msg.event)
		{
		case ESN_SSN_EVENT:
			//@todo recevied new ssn data
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
		DBG_LOG(DBG_LEVEL_ERROR, "esn handle queue is full\r\n");
		return FALSE;
	}

	return TRUE;
}

void esn_active_init(void)
{
	portBASE_TYPE res;
	res = xTaskCreate(esn_active_task,
	                  "esn_active_task",
	                  500,
	                  NULL,
	                  ESN_HANDLE_PRIORITY,
	                  NULL);
	if (res != pdTRUE)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn handle task init failed\r\n");
	}

	esn_active_queue = xQueueCreate(10, sizeof(esn_msg_t));
	if (esn_active_queue == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn_active_queue init failed\r\n");
	}
}

