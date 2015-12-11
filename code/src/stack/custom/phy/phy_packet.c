/**
 * @brief       :
 *
 * @file        : phy_packet.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <pbuf.h>
#include <phy_packet.h>
#include <phy_state.h>
#include <osel_arch.h>

pbuf_t *phy_get_packet(void)
{
	pbuf_t *pbuf = NULL;
	return pbuf;
}


bool_t phy_write_buf(pbuf_t *pbuf, uint8_t stamp_size)
{
	DBG_ASSERT(pbuf != NULL __DBG_LINE);
	if (pbuf != NULL)
	{
		pbuf->data_len += PHY_HEAD_SIZE;
		osel_memcpy(pbuf->head[0], &pbuf->attri.dst_id, PHY_HEAD_SIZE);

		lora_data_write(pbuf->head, pbuf->data_len + stamp_size);
	
		pbuf->data_len -= PHY_HEAD_SIZE;
	}
	return FALSE;
}


bool_t phy_send_data(void)
{
	lora_data_sent();
	return TRUE;
}
