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

typedef enum
{
	ESN_TIMEOUT_EVENT   = 0x00,
	ESN_VIBRATION_EVENT = 0x01,
	ESN_DISTANCE_EVENT  = 0x02,
	ESN_TEMP_EVENT      = 0x03,
} esn_event_enum_t;

typedef enum
{
    uint32_t event;
    int32_t  param;
} esn_msg_t;

#define ESN_TIMER_CYCLE			(10*60ul)

bool_t esn_active_init(void);

#endif