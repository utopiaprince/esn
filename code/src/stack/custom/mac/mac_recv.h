/**
 * @brief       : this
 * @file        : mac_recv.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-16
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-16  v0.0.1  gang.cheng    first version
 */
#ifndef __MAC_RECV_H__
#define __MAC_RECV_H__

#include "mac_frames.h"

extern mac_frames_hd_t mac_frm_head_info;

void m_recv_init(void);

#endif