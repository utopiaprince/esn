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
    
    return FALSE;
}


bool_t phy_send_data(void)
{
    return (phy_set_state(PHY_TX_STATE));
}
