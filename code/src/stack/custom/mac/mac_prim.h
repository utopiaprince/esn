/**
 * @brief       : this
 * @file        : mac_prim.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-16
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-16  v0.0.1  gang.cheng    first version
 */
#ifndef __MAC_PRIM_H__
#define __MAC_PRIM_H__

typedef enum
{
	OFF_LINE   = 0x00,
	ON_LINE    = 0x01,
} mac_line_enum_t;

#define MAC_LINE_TIMER_CYCLE		(10)

bool_t mac_online_get(void);

bool_t mac_online_set(bool_t flag);

bool_t mac_online_start(void);

void m_prim_event_handler(const osel_event_t *const pmsg);

#endif