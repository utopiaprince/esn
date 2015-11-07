/**
* @brief       :
*
* @file        : main.c
* @author      : gang.cheng
* @version     : v0.0.1
* @date        : 2015/8/10
*
* Change Logs  :
*
* Date        Version      Author      Notes
* 2015/8/10    v0.0.1      gang.cheng    first version
*/
#include "lib.h"
#include "bsp.h"
#include "osel_arch.h"

#include "stack.h"
#include "esn.h"

void driver_init(void)
{
	gprs_info_t gprs_info;
	gprs_info.dip[0] = 58;
	gprs_info.dip[1] = 214;
	gprs_info.dip[2] = 236;
	gprs_info.dip[3] = 152;
	gprs_info.port = 8056;
	gprs_info.mode = TRUE;
	gprs_info.uart_port = UART_3;
	gprs_info.uart_speed = 38400;
	gprs_driver.set(&gprs_info);
	gprs_driver.init();
	
	P5DIR |= BIT4 | BIT5;	//LED
	P5OUT |= BIT4 | BIT5;
}

int main(void)
{
	bsp_init();
	
	debug_init(DBG_LEVEL_TRACE | DBG_LEVEL_INFO | DBG_LEVEL_WARNING |
			   DBG_LEVEL_ERROR);
	
	esn_detect_init();
	esn_active_init();
	esn_gain_init();
	
	driver_init();
	stack_init();
	
	vTaskStartScheduler();
	
	return 0;
}