
#include "oserl_arch.h"
#include "lib.h"

#include "pbuf.h"
#include "sbuf.h"

#include "phy_packet.h"

#include "module.h"
#include "m_tran.h"

#include "mac.h"
#include "mac_frames.h"
#include "mac_recv.h"

static mac_frames_hd_t mac_frm_head_info;


static pbuf_t *mac_frm_get(void)
{
	pbuf_t *frame = NULL;
	frame = phy_get_packet();

	return frame;
}


static bool_t mac_addr_filter(uint8_t frm_type, void *dst_addr, uint8_t addr_mode)
{
	uint64_t dst_long_addr;
	uint16_t dst_short_addr;

	if (frm_type != MAC_FRAMES_TYPE_ACK)
	{
		if (addr_mode == MAC_ADDR_MODE_SHORT)
		{
			memcpy(&dst_short_addr, dst_addr, MAC_ADDR_SHORT_SIZE);

			if ((dst_short_addr != mac_short_addr)
			        && (dst_short_addr != MAC_BROADCAST_ADDR))
			{
				return FALSE;
			}
		}

		if (addr_mode == MAC_ADDR_MODE_LONG)
		{
			memcpy(&dst_long_addr, dst_addr, MAC_ADDR_LONG_SIZE);
			if (dst_long_addr != mac_long_addr)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


static bool_t mac_frame_head_info_parse(pbuf_t *pbuf)
{
	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
	memcpy(&(mac_frm_head_info.frm_ctrl), pbuf->data_p, MAC_HEAD_CTRL_SIZE);
	pbuf->data_p += MAC_HEAD_CTRL_SIZE;

	mac_frm_head_info.mac_seq = *pbuf->data_p;
	pbuf->data_p += MAC_HEAD_SEQ_SIZE;

	if (mac_frm_head_info.frm_ctrl.des_addr_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&mac_frm_head_info.addr_info.dst_addr,
		       pbuf->data_p, MAC_ADDR_LONG_SIZE);
		pbuf->data_p += MAC_ADDR_LONG_SIZE;
	}
	else if (mac_frm_head_info.frm_ctrl.des_addr_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&mac_frm_head_info.addr_info.dst_addr,
		       pbuf->data_p, MAC_ADDR_SHORT_SIZE);
		pbuf->data_p += MAC_ADDR_SHORT_SIZE;
	}

	if (mac_frm_head_info.frm_ctrl.src_addr_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&mac_frm_head_info.addr_info.src_addr,
		       pbuf->data_p, MAC_ADDR_LONG_SIZE);
		pbuf->data_p += MAC_ADDR_LONG_SIZE;
	}
	else if (mac_frm_head_info.frm_ctrl.src_addr_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&mac_frm_head_info.addr_info.src_addr,
		       pbuf->data_p, MAC_ADDR_SHORT_SIZE);
		pbuf->data_p += MAC_ADDR_SHORT_SIZE;
	}

	if (!mac_addr_filter(mac_frm_head_info.frm_ctrl.frm_type,
	                     (void *) & (mac_frm_head_info.addr_info.dst_addr),
	                     mac_frm_head_info.frm_ctrl.des_addr_mode))
	{
		return FALSE;
	}
	// mac head size
	mac_frm_head_info.mhr_size = (pbuf->data_p - pbuf->head) - PHY_HEAD_SIZE;
	// fill pbuf attri
	pbuf->attri.need_ack    = mac_frm_head_info.frm_ctrl.ack_req;
	pbuf->attri.seq         = mac_frm_head_info.mac_seq;
	pbuf->attri.src_id      = mac_frm_head_info.addr_info.src_addr;
	if (mac_frm_head_info.frm_ctrl.frm_type == MAC_FRAMES_TYPE_ACK)
	{
		pbuf->attri.is_ack = TRUE;
		pbuf->data_p += 1;
	}
	else
	{
		pbuf->attri.is_ack = FALSE;
	}

	pbuf->data_p = pbuf->head + pbuf->data_len - PHY_FCS_SIZE;
	return TRUE;
}


static void mac_data_frame_parse(pbuf_t *pbuf)
{
	sbuf_t *sbuf = sbuf_alloc(__SLINE1);
	if (sbuf == NULL)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
		return;
	}

	m2n_data_indication_t *m2n_data_ind =
	    &(sbuf->primargs.prim_arg.mac_prim_arg.m2n_data_indication_arg);

	m2n_data_ind->src_mode = mac_frm_head_info.frm_ctrl.src_addr_mode;
	m2n_data_ind->dst_mode = mac_frm_head_info.frm_ctrl.des_addr_mode;

	if (m2n_data_ind->dst_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&m2n_data_ind->dst_addr,
		       (uint8_t *)&mac_frm_head_info.addr_info.dst_addr,
		       MAC_ADDR_LONG_SIZE);
	}
	else if (m2n_data_ind->dst_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&m2n_data_ind->dst_addr,
		       (uint8_t *)&mac_frm_head_info.addr_info.dst_addr,
		       MAC_ADDR_SHORT_SIZE);
	}

	if (m2n_data_ind->src_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&m2n_data_ind->src_addr,
		       (uint8_t *)&mac_frm_head_info.addr_info.src_addr,
		       MAC_ADDR_LONG_SIZE);
	}
	else if (m2n_data_ind->src_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&m2n_data_ind->src_addr,
		       (uint8_t *)&mac_frm_head_info.addr_info.src_addr,
		       MAC_ADDR_SHORT_SIZE);
	}

	m2n_data_ind->msdu_length = pbuf->data_len - PHY_HEAD_SIZE - PHY_FCS_SIZE
	                            - mac_frm_head_info.mhr_size - MAC_FCS_SIZE;
	pbuf->data_len      = m2n_data_ind->msdu_length;
	m2n_data_ind->msdu  = pbuf->data_p;
	sbuf->primargs.pbuf = pbuf;

	//@todo send data to app
#include "esn_active.h"
	esn_msg_t esn_msg;
	esn_msg.param = sbuf;
	esn_msg.event = ESN_SSN_EVENT;

	esn_active_queue_send(&esn_msg);
}

static bool_t mac_frame_parse(pbuf_t *pbuf)
{
	if (NULL == pbuf)
	{
		return FALSE;
	}

	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE + mac_frm_head_info.mhr_size;

	switch (mac_frm_head_info.frm_ctrl.frm_type)
	{
	case MAC_FRAMES_TYPE_DATA:
		mac_data_frame_parse(pbuf);
		break;

	case MAC_FRAMES_TYPE_CTRL:
		mac_ctrl_parse(pbuf);
		pbuf_free(&pbuf __PLINE2);
		break;

	case MAC_FRAMES_TYPE_ACK:
		pbuf_free(&pbuf __PLINE2);
		break;

	default:
		pbuf_free(&pbuf __PLINE2);
		break;
	}

	return TRUE;
}


static void mac_tx_finish_tmp(sbuf_t *sbuf, bool_t result)
{
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	pbuf_free(&(sbuf->primargs.pbuf) __PLINE2 );
	sbuf_free(&sbuf __SLINE2 );
}

static void mac_send_ack(uint8_t seqno)
{
	//@todo
}


void m_recv_init(void)
{
	tran_cfg_t tran_cfg;
	tran_cfg.frm_get           = mac_frm_get;
	tran_cfg.frm_head_parse    = mac_frm_hd_parse;
	tran_cfg.frm_payload_parse = mac_frm_pd_parse;
	tran_cfg.tx_finish 		   = mac_tx_finish_tmp;
	tran_cfg.send_ack          = mac_send_ack;

	m_tran_cfg(&mac_tran_cb);
}
