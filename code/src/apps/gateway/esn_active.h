/**
 * @brief       : this
 * @file        : esn_active.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-21
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-21  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_ACTIVE_H__
#define __ESN_ACTIVE_H__

typedef enum
{
	ESN_TIMEOUT_EVENT   = 0x00,
	ESN_UART_EVENT		= 0x01,
	ESN_SSN_EVENT		= 0x02,
} esn_event_enum_t;

void esn_active_init(void);

bool_t esn_active_queue_send(esn_msg_t *esn_msg);

#endif