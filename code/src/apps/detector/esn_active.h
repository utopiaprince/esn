/**
 * @brief       : if esn get alarm data, start carma
 * @file        : esn_active.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-10
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-10  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_ACTIVE_H__
#define __ESN_ACTIVE_H__

#define ESN_TIMER_CYCLE			(10*60ul)

#define ESN_SEND_WAIT_TIME		(10)	// the secends wait for data sent ok
#define ESN_SEND_WAIT_CNT		(3)		// the cnt resend for data sent failed

#define ESN_SEND_BACKOFF_MAX    (5u)
#define ESN_SEND_BACKOFF_MIN    (1u)

typedef enum
{
	ESN_TIMEOUT_EVENT   = 0x00,
	ESN_UART_EVENT		= 0x01,
	ESN_SSN_EVENT		= 0x02,
    ESN_DATA_EVENT      = 0x03,
} esn_event_enum_t;

extern QueueHandle_t esn_active_queue;

bool_t esn_active_init(void);

bool_t esn_active_queue_send(esn_msg_t *esn_msg);



#endif