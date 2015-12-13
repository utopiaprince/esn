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
	gprs_info.dip[0] = 218;
	gprs_info.dip[1] = 90;
	gprs_info.dip[2] = 181;
	gprs_info.dip[3] = 54;
	gprs_info.port = 8888;
	gprs_info.mode = TRUE;
	gprs_info.uart_port = UART_4;
	gprs_info.uart_speed = 9600;
	gprs_driver.set(&gprs_info);
	gprs_driver.init();
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
	//stack_init();
	
	vTaskStartScheduler();
	
	return 0;
}