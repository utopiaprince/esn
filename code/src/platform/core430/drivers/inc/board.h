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
void wdt_clear(void);

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

#endif
