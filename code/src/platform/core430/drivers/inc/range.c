/**
 * @brief       : this
 * @file        : range.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-11-11
 * change logs  :
 * Date       Version     Author        Note
 * 2015-11-11  v0.0.1  gang.cheng    first version
 */
#include <lib.h>
#include <math.h>
#include "osel_arch.h"

#include "drivers.h"

#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

static uint8_t range_uart_data_buf[12];
uint8_t *range_uart_buf = range_uart_data_buf;

const uint8_t range_cmd_data[] = {0x0a, 0x4f, 0x4e, 0x0a};

uint8_t range_uart_index = 0;
uint8_t range_uart_id = 0;
static uint8_t range_uart_mode = RANGE_STATUS_RUNNING;

static range_data_cb_t range_data_cb;
static QueueHandle_t range_send_queue;

static fp32_t range_var = 100.0;   

static void range_uart_clear(void)
{
    range_uart_mode = RANGE_STATUS_RUNNING;
    range_uart_index = 0;
    osel_memset(range_uart_data_buf, 0x00, sizeof(range_uart_data_buf));
}

void range_cmd(void)
{
    uart_send_string(range_uart_id, (uint8_t *)range_cmd_data, sizeof(range_cmd_data));
}


static bool_t range_recv_ch_cb(uint8_t id, uint8_t ch)
{
    portBASE_TYPE xTaskWoken = pdFALSE;

    if((ch == '?') || (ch == '-'))
    {
        return xTaskWoken;
    }

    switch (range_uart_mode)
    {
    case RANGE_STATUS_RUNNING:
        range_uart_data_buf[range_uart_index++] = ch;
        if (ch == 0x0D)
        {
            range_uart_mode = RANGE_STATUS_END;
        }
        break;

    case RANGE_STATUS_END:
        if (ch == 0x0a)
        {
            range_uart_data_buf[range_uart_index++] = ch;
            esn_msg_t esn_msg;
            esn_msg.event = (GAIN_RANGE << 8) | RANGE_DATA_END;
            esn_msg.param = (void *)range_uart_index;
            xQueueSendFromISR(range_send_queue, &esn_msg, &xTaskWoken);
        }
        break;

    default:
        range_uart_clear();
        break;
    }
    
    return xTaskWoken;
}

static fp32_t range_change(void)
{
    uint8_t data_len = range_uart_index - 4;    //*< 空格， m， 回车， 换行，4个字节
    fp32_t distance = 0;
    uint8_t interge_len = data_len - 2;         //*< 小数点、小数 ，2个字节

    distance = ((fp32_t)(range_uart_data_buf[data_len-1]-0x30))/10.0;

    for(uint8_t i=0;i<interge_len;i++)
    {
        distance += (range_uart_data_buf[i]-0x30)*pow(10, interge_len-1-i);
    }

    return distance;
}

void range_sensor_init(uint8_t uart_id, uint32_t baud,
                       QueueHandle_t queue,
                       range_data_cb_t cb)
{
    range_uart_clear();
    range_uart_id = uart_id;
    uart_init(uart_id, baud);
    range_data_cb = cb;
    range_send_queue = queue;

    uart_int_cb_reg(uart_id, range_recv_ch_cb);
}

void range_handle(esn_msg_t *msg)
{
    uint8_t cmd = msg->event & 0x00FF;
    uint32_t index = 0;

    switch (cmd)
    {
    case RANGE_CMD_START:
        range_cmd();
        break;

    case RANGE_DATA_END:
        portENTER_CRITICAL();
        range_var = range_change();
        range_uart_clear();
        portEXIT_CRITICAL();
        if (range_data_cb != NULL)
        {
            range_data_cb(range_var);
        }
        break;

    default:
        break;
    }
}

fp32_t range_sensor_get(void)
{
    return range_var;
}







