#include <lib.h>
#include "osel_arch.h"

#include <drivers.h>


#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

const uint8_t atmosphere_cmd_data[] = {0x3A, 0x30, 0x31, 0x30, 0x33,
                                       0x30, 0x30, 0x30, 0x30, 0x30,
                                       0x30, 0x31, 0x31, 0x45, 0x42,
                                       0x0D, 0x0A, 0x00
                                      };


#define MOD_RECV_DATA_SIZE 0x56
uint8_t atmosphere_uart_data_buf[MOD_RECV_DATA_SIZE];
uint8_t *atmosphere_uart_buf = &atmosphere_uart_data_buf[0];
uint8_t atmosphere_uart_mode;
uint8_t atmosphere_uart_index;

static atmos_data_cb_t atmo_data_cb;
static QueueHandle_t atmo_send_queue;

static void atmosphere_uart_clear(void)
{
    atmosphere_uart_mode = ATMOS_START_IDLE;
    atmosphere_uart_index = 0;
}

static portBASE_TYPE atmosphere_cmd(void)
{
    atmosphere_uart_clear();
    MODBUS_TX();
    osel_delay(1 / portTICK_RATE_MS);
	
//	portENTER_CRITICAL();
    uart_send_string(ATMO_PORT, (uint8_t *)atmosphere_cmd_data , strlen((char const*)atmosphere_cmd_data));
//	portEXIT_CRITICAL();
	MODBUS_RX();
    return pdTRUE;
}

static bool_t atmo_recv_ch_cb(uint8_t id, uint8_t ch)
{
    portBASE_TYPE xTaskWoken = pdFALSE;
	
	ch &= 0x7F;
    if (ch == 0x3A)
    {
        atmosphere_uart_mode = ATMOS_START_RUNING;
    }

    switch (atmosphere_uart_mode)
    {
    case ATMOS_START_RUNING:
        atmosphere_uart_buf[atmosphere_uart_index++] = ch;
        if (ch == 0x0D)
        {
            atmosphere_uart_mode = ATMOS_START_END;
        }
        break;

    case ATMOS_START_END:
        if (ch == 0x0A)
        {
            atmosphere_uart_buf[atmosphere_uart_index++] = ch;
            esn_msg_t esn_msg;
            esn_msg.event = (GAIN_ATMO << 8) | ATMOS_START_END;
            esn_msg.param = (void *)atmosphere_uart_index;
            xQueueSendFromISR(atmo_send_queue, &esn_msg, &xTaskWoken);
        }
        atmosphere_uart_clear();
        break;

    default:
        atmosphere_uart_clear();
        break;
    }

    return xTaskWoken;
}

void atmos_sensor_init(uint8_t uart_id, uint32_t baud,
                       QueueHandle_t queue,
                       atmos_data_cb_t cb)
{
    atmosphere_uart_clear();
    uart_init(uart_id, baud);

    atmo_data_cb = cb;
    atmo_send_queue = queue;
    uart_int_cb_reg(uart_id, atmo_recv_ch_cb);

    MODBUS_DIR(1);
    MODBUS_RX();
}

void atmos_handle(esn_msg_t *msg)
{
    uint8_t cmd = msg->event & 0x00FF;
    uint32_t index = 0;
    switch (cmd)
    {
    case ATMOS_CMD_START:
        atmosphere_cmd();
        break;

    case ATMOS_START_END:
        index = (uint32_t)(msg->param);
        if (atmo_data_cb != NULL)
        {
            atmo_data_cb(atmosphere_uart_data_buf, index);
        }
        break;

    default:
        break;
    }
}



