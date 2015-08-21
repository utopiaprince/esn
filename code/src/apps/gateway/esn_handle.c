#include "osel_arch.h"
#include "lib.h"

#include "esn_frames.h"
#include "esn_handle.h"

DBG_THIS_MODULE("esn_handle")

static QueueHandle_t esn_handle_queue = NULL;

static void esn_handle_task(void *param)
{
	esn_msg_t esn_msg;
	while (1)
	{



	}
}

void esn_handle_init(void)
{
	portBASE_TYPE res;
	res = xTaskCreate(esn_handle_task,
	                  "esn_handle_task",
	                  500,
	                  NULL,
	                  ESN_HANDLE_PRIORITY,
	                  NULL);
	if (res != pdTRUE)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn handle task init failed\r\n");
	}

	esn_handle_queue = xQueueCreate(10, sizeof(esn_msg_t));
	if (esn_handle_queue == NULL)
    {
        DBG_LOG(DBG_LEVEL_ERROR, "esn_handle_queue init failed\r\n");
    }
}

