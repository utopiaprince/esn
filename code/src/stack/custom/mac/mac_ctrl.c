#include "osel_arch.h"
#include "lib.h"

#include "pbuf.h"
#include "sbuf.h"

#include "m_tran.h"
#include "module.h"
#include "phy_packet.h"

#include "mac.h"
#include "mac_frames.h"
#include "mac_prim.h"
#include "mac_ctrl.h"
#include "mac_recv.h"
#include "drivers.h"
DBG_THIS_MODULE("mac_ctrl")

static void mac_assoc_req_tx(sbuf_t *sbuf, uint8_t res)
{
	pbuf_t *pbuf = sbuf->primargs.pbuf;

	if (pbuf != NULL)
	{
		pbuf_free(&pbuf __PLINE2);
	}

	if (sbuf != NULL)
	{
		sbuf_free(&sbuf __SLINE2);
	}

	m_tran_recv();
}

void mac_ctrl_assoc_req_start(uint32_t dst_addr)
{
	sbuf_t *sbuf = sbuf_alloc(__SLINE1);
	if (sbuf == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "sbuf alloc failed\n");
		return;
	}

	pbuf_t *pbuf = pbuf_alloc((sizeof(mac_frames_hd_t) +
	                           MAC_CTRL_LEN +
	                           sizeof(mac_assoc_req_t) +
	                           PHY_HEAD_SIZE) __PLINE1);
	if (pbuf == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "pbuf alloc failed\n");
		sbuf_free(&sbuf __SLINE2);
		return;
	}

	sbuf->orig_layer    = MAC_LAYER;
	sbuf->up_down_link  = UP_LINK;
	sbuf->primargs.pbuf = pbuf;
    
	mac_frames_hd_t mac_frm_hd;
	mac_frm_hd.frames_ctrl.frame_type  = MAC_FRAMES_TYPE_CTRL;
	mac_frm_hd.frames_ctrl.ack_request = FALSE;
	mac_frm_hd.frames_ctrl.dst_mode    = MAC_ADDR_MODE_SHORT;
	mac_frm_hd.frames_ctrl.src_mode    = MAC_ADDR_MODE_LONG;
	mac_frm_hd.frames_ctrl.reserved    = 0x00;
	mac_frm_hd.seq                     = mac_seq++;
	mac_frm_hd.dst_addr                = dst_addr;
	mac_frm_hd.src_addr                = mac_short_addr;	//@todo get NUI from FLASH

	mac_frm_hd_fill(pbuf, &mac_frm_hd);

	mac_assoc_req_t mac_assoc_req;
    mac_assoc_req.assoc_type = MAC_CTRL_ASSOC_REQ;
	memset(mac_assoc_req.license_info, 0xa5, 10);
	mac_frm_assoc_req_fill(pbuf, &mac_assoc_req);

	m_tran_send(sbuf, mac_assoc_req_tx, 3);
}

/**
 * @brief recevied assoc req
 */
void mac_ctrl_assoc_req_handle(pbuf_t *pbuf)
{
	if (pbuf == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "pbuf is NULL\r\n");
	}

	mac_assoc_req_t mac_assoc_req;
	mac_frm_assoc_req_get(pbuf, &mac_assoc_req);

	//@todo get assoc resp the check license
	uint32_t dst_addr = 0x0000;
	memcpy((void *)&dst_addr, &(mac_frm_head_info.src_addr), sizeof(uint32_t));

	mac_ctrl_assoc_resp_start(dst_addr, MAC_ASSOC_STATUS_OK);
}

static void mac_assoc_resp_tx(sbuf_t *sbuf, uint8_t res)
{
	pbuf_t *pbuf = sbuf->primargs.pbuf;
	if (pbuf != NULL)
	{
		pbuf_free(&pbuf __PLINE2);
	}

	if (sbuf != NULL)
	{
		sbuf_free(&sbuf __SLINE2);
	}

	m_tran_recv();
}

void mac_ctrl_assoc_resp_start(uint32_t dst_addr, mac_assoc_status_enum_t status)
{
	sbuf_t *sbuf = sbuf_alloc(__SLINE1);
	if (sbuf == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "sbuf alloc failed\n");
		return;
	}

	pbuf_t *pbuf = pbuf_alloc((sizeof(mac_frames_hd_t) +
	                           MAC_CTRL_LEN +
	                           sizeof(mac_assoc_resp_t) +
	                           PHY_HEAD_SIZE) __PLINE1);
	if (pbuf == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "pbuf alloc failed\n");
		sbuf_free(&sbuf __SLINE2);
		return;
	}

	sbuf->orig_layer    = MAC_LAYER;
	sbuf->up_down_link  = DOWN_LINK;
	sbuf->primargs.pbuf = pbuf;

	mac_frames_hd_t mac_frm_hd;
	mac_frm_hd.frames_ctrl.frame_type  = MAC_FRAMES_TYPE_CTRL;
	mac_frm_hd.frames_ctrl.ack_request = TRUE;
	mac_frm_hd.frames_ctrl.dst_mode    = MAC_ADDR_MODE_SHORT;
	mac_frm_hd.frames_ctrl.src_mode    = MAC_ADDR_MODE_SHORT;
	mac_frm_hd.frames_ctrl.reserved    = 0x00;
	mac_frm_hd.seq                     = mac_seq++;
	mac_frm_hd.dst_addr                = dst_addr;
	mac_frm_hd.src_addr                = mac_short_addr;	//@todo get NUI from FLASH

	mac_frm_hd_fill(pbuf, &mac_frm_hd);

	mac_assoc_resp_t mac_assoc_resp;
    mac_assoc_resp.assoc_type = MAC_CTRL_ASSOC_RESP;
	mac_assoc_resp.status = status;
	mac_frm_assoc_resp_fill(pbuf, &mac_assoc_resp);

	m_tran_send(sbuf, mac_assoc_resp_tx, 3);
}

void mac_ctrl_assoc_resp_handle(pbuf_t *pbuf)
{
	if (pbuf == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "pbuf is NULL\r\n");
	}

	coord_addr = mac_frm_head_info.src_addr;

	mac_assoc_resp_t mac_assoc_resp;
	mac_frm_assoc_resp_get(pbuf, &mac_assoc_resp);

	if (mac_assoc_resp.status == MAC_ASSOC_STATUS_OK)
	{
		mac_online_set(ON_LINE);
        led_set(LEN_GREEN, TRUE);
		xTimerStop(mac_line_cycle_timer, 0);
	}
}


void mac_ctrl_parse(pbuf_t *pbuf)
{
	uint8_t ctrl_frm_type = *(pbuf->data_p);
//	pbuf->data_p += sizeof(uint8_t);
    
	switch (ctrl_frm_type)
	{
	case MAC_CTRL_ASSOC_REQ:
#ifdef NODE_TYPE_GATEWAY
		mac_ctrl_assoc_req_handle(pbuf);
#endif
		break;

	case MAC_CTRL_ASSOC_RESP:
#ifdef NODE_TYPE_DETECTOR
		mac_ctrl_assoc_resp_handle(pbuf);
#endif
		break;

	default:
		break;
	}
}
