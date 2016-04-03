/**
 * @brief       : this
 * @file        : mac_frames.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-16
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-16  v0.0.1  gang.cheng    first version
 */
#ifndef __MAC_FRAMES_H__
#define __MAC_FRAMES_H__

#include "pbuf.h"

#define MAC_FCS_SIZE                    0u      //!< MAC的FCS长度
#define MAC_HEAD_CTRL_SIZE              2u      //!< MAC的控制头长度
#define MAC_HEAD_SEQ_SIZE               1u      //!< MAC的序列号长度
#define MAC_ADDR_SHORT_SIZE             4u      //!< MAC的短地址长度
#define MAC_ADDR_LONG_SIZE              8u      //!< MAC的长地址长度
     
#define MAC_BROADCAST_ADDR              0xffff

extern uint16_t phy_addr;

typedef enum
{
	MAC_FRAMES_TYPE_DATA = 0x00,
	MAC_FRAMES_TYPE_ACK  = 0x01,
	MAC_FRAMES_TYPE_CTRL = 0x02,
} mac_frames_type_enum_t;

typedef enum
{
	MAC_ADDR_MODE_NONE 	= 0x00,
	MAC_ADDR_MODE_SHORT = 0x02,
	MAC_ADDR_MODE_LONG  = 0x03,
} mac_addr_mode_enum_t;

typedef enum
{
	MAC_CTRL_ASSOC_REQ  = 0x01,
	MAC_CTRL_ASSOC_RESP = 0x02,
} mac_ctrl_enum_t;

typedef enum
{
	MAC_ASSOC_STATUS_OK   = 0x00,
	MAC_ASSOC_STATUS_FULL = 0x01,
	MAC_ASSOC_STATUS_LIN  = 0x02,
} mac_assoc_status_enum_t;

#define MAC_CTRL_LEN		(1u)

typedef struct
{
	uint8_t  frame_type	 : 2,
	         ack_request : 1,
	         dst_mode	 : 2,
	         src_mode	 : 2,
	         reserved	 : 1;
} mac_frames_ctrl_t;


typedef struct
{
	mac_frames_ctrl_t frames_ctrl;
	uint8_t mhr_size;
	uint8_t seq;
	uint64_t dst_addr;
	uint64_t src_addr;
} mac_frames_hd_t;


typedef struct
{
    uint8_t assoc_type;
	uint8_t license_info[10];
} mac_assoc_req_t;

typedef struct
{
    uint8_t assoc_type;
	mac_assoc_status_enum_t status;
} mac_assoc_resp_t;


int8_t mac_frm_hd_get(pbuf_t *pbuf, mac_frames_hd_t *mac_frm_hd);

int8_t mac_frm_hd_fill(pbuf_t *pbuf, mac_frames_hd_t *mac_frm_hd);


int8_t mac_frm_data_get(pbuf_t *pbuf, void *datap);

int8_t mac_frm_data_fill(pbuf_t *pbuf, void *datap, uint8_t len);


int8_t mac_frm_assoc_req_get(pbuf_t *pbuf, mac_assoc_req_t *assoc_req);

int8_t mac_frm_assoc_req_fill(pbuf_t *pbuf, mac_assoc_req_t *assoc_req);


int8_t mac_frm_assoc_resp_get(pbuf_t *pbuf, mac_assoc_resp_t *assoc_resp);

int8_t mac_frm_assoc_resp_fill(pbuf_t *pbuf, mac_assoc_resp_t *assoc_resp);

#endif