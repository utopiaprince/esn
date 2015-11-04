
#include "osel_arch.h"

#include "sbuf.h"
#include "pbuf.h"
#include "phy_packet.h"

#include "m_tran.h"
#include "mac_frames.h"

int8_t mac_frm_hd_get(pbuf_t *pbuf, mac_frames_hd_t *mac_frm_hd)
{
	if (pbuf == NULL)
	{
		return NULL;
	}

	uint8_t *datap = pbuf->head + PHY_HEAD_SIZE;
	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
	memcpy(&(mac_frm_hd->frames_ctrl), pbuf->data_p, MAC_HEAD_CTRL_SIZE);
	pbuf->data_p += MAC_HEAD_CTRL_SIZE;

	mac_frm_hd->seq = *pbuf->data_p;
	pbuf->data_p += MAC_HEAD_SEQ_SIZE;

	if (mac_frm_hd->frames_ctrl.dst_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&mac_frm_hd->dst_addr,
		       pbuf->data_p, MAC_ADDR_LONG_SIZE);
		pbuf->data_p += MAC_ADDR_LONG_SIZE;
	}
	else if (mac_frm_hd->frames_ctrl.dst_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&mac_frm_hd->dst_addr,
		       pbuf->data_p, MAC_ADDR_SHORT_SIZE);
		pbuf->data_p += MAC_ADDR_SHORT_SIZE;
	}

	if (mac_frm_hd->frames_ctrl.src_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy((uint8_t *)&mac_frm_hd->src_addr,
		       pbuf->data_p, MAC_ADDR_LONG_SIZE);
		pbuf->data_p += MAC_ADDR_LONG_SIZE;
	}
	else if (mac_frm_hd->frames_ctrl.src_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy((uint8_t *)&mac_frm_hd->src_addr,
		       pbuf->data_p, MAC_ADDR_SHORT_SIZE);
		pbuf->data_p += MAC_ADDR_SHORT_SIZE;
	}

	mac_frm_hd->mhr_size = pbuf->data_p - datap;
	pbuf->data_len -= (mac_frm_hd->mhr_size + PHY_HEAD_SIZE);

	return (mac_frm_hd->mhr_size);
}

int8_t mac_frm_hd_fill(pbuf_t *pbuf, mac_frames_hd_t *mac_frm_hd)
{
	if (pbuf == NULL)
	{
		return 0;
	}

	uint8_t *datap = pbuf->head + PHY_HEAD_SIZE;
	pbuf->data_p = pbuf->head + PHY_HEAD_SIZE;
	memcpy(pbuf->data_p, &(mac_frm_hd->frames_ctrl), MAC_HEAD_CTRL_SIZE);
	pbuf->data_p += MAC_HEAD_CTRL_SIZE;

	memcpy(pbuf->data_p, &(mac_frm_hd->seq), MAC_HEAD_SEQ_SIZE);
	pbuf->data_p += MAC_HEAD_SEQ_SIZE;

	if (mac_frm_hd->frames_ctrl.dst_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy(pbuf->data_p, (uint8_t *)&mac_frm_hd->dst_addr,
		       MAC_ADDR_LONG_SIZE);
		pbuf->data_p += MAC_ADDR_LONG_SIZE;
	}
	else if (mac_frm_hd->frames_ctrl.dst_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy(pbuf->data_p, (uint8_t *)&mac_frm_hd->dst_addr,
		       MAC_ADDR_SHORT_SIZE);
		pbuf->data_p += MAC_ADDR_SHORT_SIZE;
	}

	if (mac_frm_hd->frames_ctrl.src_mode == MAC_ADDR_MODE_LONG)
	{
		memcpy(pbuf->data_p, (uint8_t *)&mac_frm_hd->src_addr,
		       MAC_ADDR_LONG_SIZE);
		pbuf->data_p += MAC_ADDR_LONG_SIZE;
	}
	else if (mac_frm_hd->frames_ctrl.src_mode == MAC_ADDR_MODE_SHORT)
	{
		memcpy(pbuf->data_p, (uint8_t *)&mac_frm_hd->src_addr,
		       MAC_ADDR_SHORT_SIZE);
		pbuf->data_p += MAC_ADDR_SHORT_SIZE;
	}

	uint8_t len = pbuf->data_p - datap;
	pbuf->data_len += len;

	pbuf->attri.seq 	  = mac_frm_hd->seq;
	pbuf->attri.send_mode = CSMA_SEND_MODE;
	pbuf->attri.need_ack  = mac_frm_hd->frames_ctrl.ack_request;
	if(mac_frm_hd->frames_ctrl.frame_type == MAC_FRAMES_TYPE_ACK)
	{
		pbuf->attri.is_ack = TRUE;
	}
	
	if (MAC_ADDR_MODE_NONE == mac_frm_hd->frames_ctrl.dst_mode)
	{
		pbuf->attri.dst_id = MAC_BROADCAST_ADDR;
	}
	else
	{
		pbuf->attri.dst_id = (uint16_t)mac_frm_hd->dst_addr;
	}

	return len;
}

int8_t mac_frm_data_get(pbuf_t *pbuf, void *datap)
{
	if (pbuf == NULL)
	{
		return -1;
	}

	if (datap == NULL)
	{
		return -1;
	}

	memcpy(datap, pbuf->data_p, pbuf->data_len);
	pbuf->data_p += pbuf->data_len;

	pbuf->data_len = 0;

	return pbuf->data_len;
}

int8_t mac_frm_data_fill(pbuf_t *pbuf, void *datap, uint8_t len)
{
	if (pbuf == NULL)
	{
		return -1;
	}

	if (datap == NULL)
	{
		return -1;
	}

	uint8_t offset = PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE +
	                 MAC_ADDR_SHORT_SIZE + MAC_ADDR_LONG_SIZE;
	pbuf->data_p = pbuf->head + offset;

	memcpy(pbuf->data_p, datap, len);
	pbuf->data_p += len;
	pbuf->data_len += len;

	return len;
}

int8_t mac_frm_assoc_req_get(pbuf_t *pbuf, mac_assoc_req_t *assoc_req)
{
	if (pbuf == NULL)
	{
		return 0;
	}

	//@noto make sure pbuf->data_p pointer to assoc req payload
	uint8_t len = sizeof(mac_assoc_req_t);
	memcpy(assoc_req, pbuf->data_p, len);
	pbuf->data_p += len;
	pbuf->data_len -= len;
	return len;
}

int8_t mac_frm_assoc_req_fill(pbuf_t *pbuf, mac_assoc_req_t *assoc_req)
{
	if (pbuf == NULL)
	{
		return 0;
	}

	uint8_t offset = PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE +
	                 MAC_ADDR_SHORT_SIZE + MAC_ADDR_LONG_SIZE;
	pbuf->data_p = pbuf->head + offset;

	uint8_t len = sizeof(mac_assoc_req_t);
	memcpy(pbuf->data_p, assoc_req, len);
	pbuf->data_p += len;
	pbuf->data_len += len;

	return len;
}

int8_t mac_frm_assoc_resp_get(pbuf_t *pbuf, mac_assoc_resp_t *assoc_resp)
{
	if (pbuf == NULL)
	{
		return 0;
	}

	uint8_t offset = PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE +
	                 MAC_ADDR_SHORT_SIZE + MAC_ADDR_LONG_SIZE;
	pbuf->data_p = pbuf->head + offset;

	uint8_t len = sizeof(mac_assoc_resp_t);
	memcpy(assoc_resp, pbuf->data_p, len);
	pbuf->data_p += len;
	pbuf->data_len -= len;

	return len;
}

int8_t mac_frm_assoc_resp_fill(pbuf_t *pbuf, mac_assoc_resp_t *assoc_resp)
{
	if (pbuf == NULL)
	{
		return 0;
	}

	uint8_t offset = PHY_HEAD_SIZE + MAC_HEAD_CTRL_SIZE + MAC_HEAD_SEQ_SIZE +
	                 MAC_ADDR_SHORT_SIZE + MAC_ADDR_LONG_SIZE;
	pbuf->data_p = pbuf->head + offset;

	uint8_t len = sizeof(mac_assoc_resp_t);
	memcpy(pbuf->data_p, assoc_resp, len);
	pbuf->data_p += len;
	pbuf->data_len += len;

	return len;
}


