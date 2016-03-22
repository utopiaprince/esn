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
#include "esn_package.h"



DBG_THIS_MODULE("esn_active")

static QueueHandle_t esn_active_queue = NULL;

static uint8_t get_message_type(uint8_t frame_type)
{
	uint8_t message_type = 0x00;
	switch (frame_type)
	{
	case DATATYPE_VIBRATION:
		message_type = M_SHOCK;
		break;

	case DATATYPE_DISTANCE:
		message_type = M_DISTANCE;
		break;

	case DATATYPE_PICTURE:
		message_type = M_CAME;
		break;

	case DATATYPE_ANGLE:
		message_type = M_ACCE;
		break;

	case DATATYPE_ATMO:
		message_type = M_ATMO;
		break;

	default:
		break;
	}

	return message_type;
}

static void esn_recv_send(uint64_t srd_id,
                          uint8_t frame_type,
                          void *pdata,
                          uint16_t len)
{
	uint8_t data[200];
	uint16_t length = 0;
	uint8_t *p = data;
	p += 4;
    uint8_t message_type;
	message_type = get_message_type(frame_type);
    
    switch(message_type)
    {
    case M_SHOCK:
        {
            shock_t info;
            osel_memset(&info, 0, sizeof(shock_t));
            osel_memcpy(info.umonitor, &srd_id, sizeof(uint64_t));
            mac_addr_get(info.bmonitor);
            info.collect_time = 0;
            info.thresh_tap = ((uint8_t *)pdata)[0];
            info.dur = ((uint8_t *)pdata)[1];
            shock_send((uint8_t *)&info, sizeof(shock_t));
        }
        break;
    }
    
//    uint8_t bmonitor[ID_MAX];
//    mac_addr_get(bmonitor);
//	osel_memcpy(package.bmonitor, bmonitor, ID_MAX);
//    //*< 采集时间修改
//	package.collect_time = 0xFFFFFFFF;
//	package.alarm = 0;
//
//	osel_memcpy(p, &package, sizeof(esn_package_t));
//	length += sizeof(esn_package_t);
//	p += sizeof(esn_package_t);
//    
//    uint16_t cnt = 1;
//    uint16_t index = 1;
//    if(frame_type == DATATYPE_PICTURE)
//    {
//        osel_memcpy(&cnt, pdata, sizeof(cnt));
//        pdata = (uint8_t *)pdata + sizeof(cnt);
//        osel_memcpy(&index, pdata, sizeof(index));
//        pdata = (uint8_t *)pdata + sizeof(index);
//    }
//
//	osel_memcpy(p, &cnt, sizeof(uint16_t));
//	length += 2; p += 2;
//	osel_memcpy(p, &index, sizeof(uint16_t));
//	length += 2; p += 2;
//    
//	osel_memcpy(p, pdata, len);
//	length += len;
//	esn_gprs_send(data, length);
}

static void esn_frames_recv_handle(sbuf_t *sbuf)
{
	if (sbuf == NULL)
	{
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
    
    esn_recv_send(m2n_data_ind->src_addr,
                  esn_frm_hd.frames_ctrl.data_type,
                  pbuf->data_p,
                  pbuf->data_len);
//	switch (esn_frm_hd.frames_ctrl.data_type) {
//	case DATATYPE_VIBRATION:
//		esn_vibration_handle(pbuf);
//		break;

//	case DATATYPE_DISTANCE:
//		esn_distance_handle(pbuf);
//		break;
//
//	case DATATYPE_TEMPERATURE:
//		esn_temperature_handle(pbuf);
//		break;
//
//	case DATATYPE_PICTURE:
//		esn_camra_handle(pbuf, &esn_frm_hd);
//		break;

//	default:
//		break;
//	}

	pbuf_free(&pbuf __PLINE2 );
	sbuf_free(&sbuf __SLINE2 );
}

static void esn_active_task(void *param)
{
	esn_msg_t esn_msg;

	while (1)
	{
		if (xQueueReceive(esn_active_queue,
		                  &esn_msg,
		                  portMAX_DELAY))
		{
			switch (esn_msg.event)
			{
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
	                  300,
	                  NULL,
	                  ESN_ACTIVE_PRIORITY,
	                  NULL);
	if (res != pdTRUE)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn handle task init failed\r\n");
	}

	esn_active_queue = xQueueCreate(20, sizeof(esn_msg_t));
	if (esn_active_queue == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn_active_queue init failed\r\n");
	}
}

