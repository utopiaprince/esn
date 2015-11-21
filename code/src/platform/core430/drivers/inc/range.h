/**
 * @brief       : this
 * @file        : range.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-11-11
 * change logs  :
 * Date       Version     Author        Note
 * 2015-11-11  v0.0.1  gang.cheng    first version
 */

#ifndef __RANGE_H__
#define __RANGE_H__

#include "osel_arch.h"

#define RANGE_PORT          (UART_3)

#define GAIN_RANGE          (0x02)
#define RANGE_CMD_START     (0x00)
#define RANGE_DATA_END      (0x01)
#define RANGE_DATA_SEND     (0x02)

typedef enum
{
    RANGE_STATUS_RUNNING,
    RANGE_STATUS_END,
} range_status_enum_t;

typedef void (*range_data_cb_t)(fp32_t distance);

void range_sensor_init(uint8_t uart_id, uint32_t baud,
                       QueueHandle_t queue,
                       range_data_cb_t cb);

void range_cmd(void);

fp32_t range_sensor_get(void);

void range_handle(esn_msg_t *msg);

#endif
