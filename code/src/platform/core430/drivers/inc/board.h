/**
 * @brief       : this
 * @file        : board.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-01
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-01  v0.0.1  gang.cheng    first version
 */
#ifndef __BOARD_H__
#define __BOARD_H__
#include <data_type_def.h>
void hard_wdt_clear(void);
enum led_e
{
	LED_RED,
	LEN_GREEN,			//用于GPRS连接状态
};
/**
 * @biref open or close the led
 * @param led
 *  - 0 RED
 *  - 1 GREEN
 * @param res
 *  - TRUE open
 *  - FLASE close
 */
void led_set(uint8_t led, bool_t res);

void board_init(void);

void mac_addr_get(uint8_t *mac_addr);


void sys_wdt_clear(void);
#endif
