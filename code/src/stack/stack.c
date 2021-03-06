/**
 * @brief       :
 *
 * @file        : stack.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <lib.h>
#include "osel_arch.h"

#include "stack.h"

//#include <nwk.h>
//#include <app.h>
//#include <hal_board.h>

void stack_init(void)
{
	pbuf_init();
	bool_t res2 = sbuf_init();
	DBG_ASSERT(res2 != FALSE __DBG_LINE);

	mac_init();
}