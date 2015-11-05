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
  UART_1,                         /**< 串口1:485 */
  UART_2,                         /**< 串口2:DBG */
  UART_3,                         /**< 串口3:TTL */
  UART_4,                         /**< 串口4:lora */
} uart_id_t;

typedef void (*uart_interupt_cb_t)(uint8_t id, uint8_t ch);

/**
 * Initializes the serial communications peripheral and GPIO ports
 * to communicate with the peripheral device.
 *
 * @param  uart_id which uart should be operated
 * @param  baud_rate uart baud rate min.300 and max.115200
 */
void uart_init(uint8_t uart_id, uint32_t baud_rate);


/**
 * register uart id to task
 *
 * @param uart_id which uart to be registered
 * @param task_id which task to be registered
 */
void uart_send_char(uint8_t id, uint8_t value);

/**
 * Send a string by uart
 *
 * @param id  which uart should be operated
 * @param *string Pointer to a string containing the data to be sended
 * @param length Max value is 255
 */
void uart_send_string(uint8_t id, uint8_t *string, uint16_t length);

void uart_recv_enable(uint8_t uart_id);

void uart_recv_disable(uint8_t uart_id);

void uart_int_cb_reg(uart_interupt_cb_t cb);


#endif
