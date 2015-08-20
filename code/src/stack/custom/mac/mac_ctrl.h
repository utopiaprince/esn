/**
 * @brief       : this
 * @file        : mac_ctrl.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-17
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-17  v0.0.1  gang.cheng    first version
 */
#ifndef __MAC_CTRL_H__
#define __MAC_CTRL_H__


void mac_ctrl_assoc_req_start(uint16_t dst_addr);

/**
 * @brief host received request
 * @param pbuf 
 */
void mac_ctrl_assoc_req_handle(pbuf_t *pbuf);

/**
 * @brief from host to detector
 * @param dst_addr
 * @param status
 */
void mac_ctrl_assoc_resp_start(uint16_t dst_addr, mac_assoc_status_enum_t status);

/**
 * @brief detector received response
 * @param pbuf [description]
 */
void mac_ctrl_assoc_resp_handle(pbuf_t *pbuf);

#endif
