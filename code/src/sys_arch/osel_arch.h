#ifndef __OSEL_ARCH_H__
#define __OSEL_ARCH_H__

#include "FreeRTOS.h"
#include "task.h"

#define hal_int_state_t         char
#define HAL_ENTER_CRITICAL(s)   s=0;taskENTER_CRITICAL()
#define HAL_EXIT_CRITICAL(s)    s=s;taskEXIT_CRITICAL()

#define osel_memset             memset
#define osel_memcpy             memcpy
#define osel_memcmp             memcmp
#define osel_mem_alloc          pvPortMalloc


#endif
