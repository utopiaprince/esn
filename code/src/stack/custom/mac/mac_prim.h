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
	START_LINE = 0x01,
	ON_LINE    = 0x02,
} mac_line_enum_t;

extern TimerHandle_t mac_line_cycle_timer;

#define MAC_LINE_TIMER_CYCLE		(10)

mac_line_enum_t mac_online_get(void);

bool_t mac_online_set(mac_line_enum_t flag);

bool_t mac_online_start(void);

void m_prim_event_handler(const osel_event_t *const pmsg);

#endif