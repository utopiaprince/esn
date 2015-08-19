#ifndef __OSEL_ARCH_H__
#define __OSEL_ARCH_H__

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#define hal_int_state_t         char
#define HAL_ENTER_CRITICAL(s)   s=0;taskENTER_CRITICAL()
#define HAL_EXIT_CRITICAL(s)    s=s;taskEXIT_CRITICAL()

#define osel_memset             memset
#define osel_memcpy             memcpy
#define osel_memcmp             memcmp
#define osel_mem_alloc          pvPortMalloc

typedef enum
{
	MSG_LOW_PRIO,
	MSG_HIGH_PRIO,
} osel_eblock_prio_t;

typedef portBASE_TYPE osel_signal_t;

typedef void *		osel_param_t;

typedef struct
{
    osel_signal_t event;
    osel_param_t param;
} osel_event_t;


#endif
