
#include "osel_arch.h"
#include "lib.h"

#include "pbuf.h"
#include "sbuf.h"

#include "phy_packet.h"

#include "module.h"
#include "m_tran.h"

#include "mac.h"
#include "mac_frames.h"
#include "mac_recv.h"
#include "mac_ctrl.h"

mac_frames_hd_t mac_frm_head_info;


static pbuf_t *mac_frm_get(void)
{
	pbuf_t *frame = NULL;
	frame = phy_get_packet();

	return frame;
}


static bool_t mac_addr_filter(uint8_t frm_type, void *dst_addr, uint8_t addr_mode)
{
	uint64_t dst_long_addr;
	uint32_t dst_short_addr;

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
	mac_frm_hd_get(pbuf, &mac_frm_head_info);

	if (!mac_addr_filter(mac_frm_head_info.frames_ctrl.frame_type,
	                     (void *) & (mac_frm_head_info.dst_addr),
	                     mac_frm_head_info.frames_ctrl.dst_mode))
	{
		return FALSE;
	}

	pbuf->attri.need_ack    = mac_frm_head_info.frames_ctrl.ack_request;
	pbuf->attri.seq         = mac_frm_head_info.seq;
	pbuf->attri.src_id      = mac_frm_head_info.src_addr;
	if (mac_frm_head_info.frames_ctrl.frame_type == MAC_FRAMES_TYPE_ACK)
	{
		pbuf->attri.is_ack = TRUE;
	}
	else
	{
		pbuf->attri.is_ack = FALSE;
	}

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

	m2n_data_ind->src_mode = mac_frm_head_info.frames_ctrl.src_mode;
	m2n_data_ind->dst_mode = mac_frm_head_info.frames_ctrl.dst_mode;

	if (m2n_data_ind->dst_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&m2n_data_ind->dst_addr,
		       (uint8_t *)&mac_frm_head_info.dst_addr,
		       MAC_ADDR_LONG_SIZE);
	}
	else if (m2n_data_ind->dst_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&m2n_data_ind->dst_addr,
		       (uint8_t *)&mac_frm_head_info.dst_addr,
		       MAC_ADDR_SHORT_SIZE);
	}

	if (m2n_data_ind->src_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&m2n_data_ind->src_addr,
		       (uint8_t *)&mac_frm_head_info.src_addr,
		       MAC_ADDR_LONG_SIZE);
	}
	else if (m2n_data_ind->src_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&m2n_data_ind->src_addr,
		       (uint8_t *)&mac_frm_head_info.src_addr,
		       MAC_ADDR_SHORT_SIZE);
	}

	m2n_data_ind->msdu_length = pbuf->data_len; //*< 已经经过mac_frame_head_info_parse()处理，去掉了MAC帧长
	pbuf->data_len      = m2n_data_ind->msdu_length;
	m2n_data_ind->msdu  = pbuf->data_p;
	sbuf->primargs.pbuf = pbuf;

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

	switch (mac_frm_head_info.frames_ctrl.frame_type)
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

	if (result)
	{
		mac_sent_set();
	}
}

static void ack_tx_ok_callback(sbuf_t *sbuf, uint8_t res)
{

}

static void mac_send_ack(uint8_t seqno)
{
	pbuf_t *pbuf = pbuf_alloc(SMALL_PBUF_BUFFER_SIZE __PLINE1);
	DBG_ASSERT(NULL != pbuf __DBG_LINE);
	if (pbuf == NULL)
	{
		return;
	}

	mac_frames_hd_t mac_frm_hd;
	mac_frm_hd.frames_ctrl.frame_type  = MAC_FRAMES_TYPE_ACK;
	mac_frm_hd.frames_ctrl.ack_request = FALSE;
	mac_frm_hd.frames_ctrl.dst_mode    = MAC_ADDR_MODE_NONE;
	mac_frm_hd.frames_ctrl.src_mode    = MAC_ADDR_MODE_NONE;

	mac_frm_hd.mhr_size = MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE;
	mac_frm_hd.seq = seqno;

	mac_frm_hd_fill(pbuf, &mac_frm_hd);

	DBG_ASSERT((pbuf->data_p - pbuf->head) <= SMALL_PBUF_BUFFER_SIZE __DBG_LINE);
	sbuf_t *sbuf = sbuf_alloc(__SLINE1);
	DBG_ASSERT(sbuf != NULL __DBG_LINE);
	if (sbuf != NULL)
	{
		sbuf->primargs.pbuf = pbuf;

		if (m_tran_can_send())
		{
			m_tran_send(sbuf, ack_tx_ok_callback, 1);
		}
		else
		{
			pbuf_free(&(sbuf->primargs.pbuf) __PLINE2);
			sbuf_free(&sbuf __SLINE2);
		}
	}
	else
	{
		pbuf_free(&pbuf __PLINE2);
	}
}


void m_recv_init(void)
{
	tran_cfg_t tran_cfg;
	tran_cfg.frm_get           = mac_frm_get;
	tran_cfg.frm_head_parse    = mac_frame_head_info_parse;
	tran_cfg.frm_payload_parse = mac_frame_parse;
	tran_cfg.tx_finish 		   = mac_tx_finish_tmp;
	tran_cfg.send_ack          = mac_send_ack;

	m_tran_cfg(&tran_cfg);
}
