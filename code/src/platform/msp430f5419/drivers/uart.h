/**
 * @brief       : 
 *
 * @file        : uart.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/8/10
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/8/10    v0.0.1      gang.cheng    first version
 */
#ifndef __UART_H__
#define __UART_H__

#ifndef __DATA_TYPE_DEF_H__
#error "include data_type_def.h must appear in source file before include uart.h"
#endif

typedef enum 
{
    UART_1,                         /**< 串口1 */
    UART_2,                         /**< 串口2 */
    UART_3,                         /**< 串口3 */
    UART_4,                         /**< 串口4 */
} uart_id_t;

void uart_send(uint8_t id, uint8_t ch);

#endif
