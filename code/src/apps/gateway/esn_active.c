#include "osel_arch.h"
#include "lib.h"

#include <drivers.h>

#include "sbuf.h"
#include "prim.h"

#include "esn.h"
#include "mac.h"
#include "mac_prim.h"
#include "module.h"

#include "esn_frames.h"
#include "esn_active.h"



DBG_THIS_MODULE("esn_active")

static QueueHandle_t esn_active_queue = NULL;

static void esn_gprs_send(void *data_p, uint16_t len)
{

}

static void esn_vibration_handle(pbuf_t *pbuf)
{
	//@todo

}

static void esn_distance_handle(pbuf_t *pbuf)
{
	//@todo
}

static void esn_temperature_handle(pbuf_t *pbuf)
{
	//@todo
}

static void esn_camra_handle(pbuf_t *pbuf, esn_frames_head_t *esn_frm_hd)
{

}

static void esn_frames_recv_handle(sbuf_t *sbuf)
{
	if (sbuf == NULL) {
		DBG_LOG(DBG_LEVEL_ERROR, "sbuf is NULL!\r\n");
		return;
	}

	pbuf_t *pbuf = sbuf->primargs.pbuf;

	m2n_data_indication_t *m2n_data_ind =
	    &(sbuf->primargs.prim_arg.mac_prim_arg.m2n_data_indication_arg);

	pbuf->data_p = m2n_data_ind->msdu;
	pbuf->data_len = m2n_data_ind->msdu_length;
	esn_frames_head_t esn_frm_hd;
	osel_memcpy(&esn_frm_hd, pbuf->data_p, sizeof(esn_frames_head_t));
	pbuf->data_p += sizeof(esn_frames_head_t);
	pbuf->data_len -= sizeof(esn_frames_head_t);

	switch (esn_frm_hd.frames_ctrl.data_type) {
	case DATATYPE_VIBRATION:
		esn_vibration_handle(pbuf);
		break;

	case DATATYPE_DISTANCE:
		esn_distance_handle(pbuf);
		break;

	case DATATYPE_TEMPERATURE:
		esn_temperature_handle(pbuf);
		break;

	case DATATYPE_PICTURE:
		esn_camra_handle(pbuf, &esn_frm_hd);
		break;

	default:
		break;
	}

	pbuf_free(&pbuf __PLINE2 );
	sbuf_free(&sbuf __SLINE2 );
}

static void esn_active_task(void *param)
{
	esn_msg_t esn_msg;

	while (1) {
		if(xQueueReceive(esn_active_queue,
		              &esn_msg,
		              portMAX_DELAY))
        {
            switch (esn_msg.event) {
            case ESN_SSN_EVENT:
                esn_frames_recv_handle((sbuf_t *)esn_msg.param);
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
	                  300,
	                  NULL,
	                  ESN_ACTIVE_PRIORITY,
	                  NULL);
	if (res != pdTRUE) {
		DBG_LOG(DBG_LEVEL_ERROR, "esn handle task init failed\r\n");
	}

	esn_active_queue = xQueueCreate(10, sizeof(esn_msg_t));
	if (esn_active_queue == NULL) {
		DBG_LOG(DBG_LEVEL_ERROR, "esn_active_queue init failed\r\n");
	}
}

