/**
* @brief       :
*
* @file        : adxl345.c
* @author      : gang.cheng
* @version     : v0.0.1
* @date        : 2015/11/5
*
* -----------------------------------------------
* head | addr | cmd | info1 | info2 | sh-sl | end
* 1byte|  1   |  1  |  var  | var   |  2    |  1
* Change Logs  :
*
* Date        Version      Author      Notes
* 2015/11/5    v0.0.1      gang.cheng    first version
*/

#include <lib.h>
#include "osel_arch.h"

#include <drivers.h>


#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

uint16_t photo_byte_sum = 0;
uint8_t  photo_pack_sum = 0;
uint8_t  photo_pack_pos = 0;

bool_t   camera_last_byte = FALSE;
uint16_t camera_uart_index = 0;
uint8_t  camera_uart_mode;

static bool_t camera_has_start = FALSE;

static uint8_t camera_uart_data_buf[530];
uint8_t *camera_uart_buf = camera_uart_data_buf; //*< cmd

static camera_data_cb_t camer_data_cb;
static QueueHandle_t cam_send_queue;

static TickType_t cam_old_tick = 0; //*< 4字节
static TickType_t cam_new_tick = 0;

static uint8_t cmd_temp = CAM_CMD_PHONE;


static uint8_t photo_info(uint8_t *pdata)
{
	uint16_t i = 0;
	i = pdata[4];
	photo_byte_sum = ( i << 0x08) + pdata[0x05];
	if (photo_byte_sum == 0)
	{
		photo_pack_sum = 0;
	}
	else
	{
		photo_pack_sum = (photo_byte_sum-1) / CAM_FRM_MAX_LEN + 1;
	}
	return 0x00;
}

static uint16_t check_sum(uint8_t *pdata, uint16_t len)
{
	uint16_t i = 0x0000;
	uint16_t isum = 0x0000;
	for (; i < len; i++)
	{
		isum += pdata[i];
	}
	return isum;
}

static uint8_t camera_frm_create(uint8_t *pdata, uint8_t cmd,
								 uint8_t info1, uint8_t info2)
{
	uint8_t pos = 0x00;
	uint8_t isum = 0x00;
	uint8_t addr = CAM_ADDR;
	uint16_t ck_sum = 0x0000;
	pdata[pos++] = CAM_FRM_HEAD;
	
	if ( (addr == CAM_FRM_HEAD) || (addr == CAM_FRM_CHECK) )
	{
		pdata[pos++] = CAM_FRM_CHECK;
		pdata[pos++] = (addr ^ CAM_FRM_CHECK);
	}
	else
	{
		pdata[pos++] = addr;
	}
	
	if ( (cmd == CAM_FRM_HEAD) || (cmd == CAM_FRM_CHECK) )
	{
		pdata[pos++] = CAM_FRM_CHECK;
		pdata[pos++] = (cmd ^ CAM_FRM_CHECK);
	}
	else
	{
		pdata[pos++] = cmd;
	}
	
	if ( (info1 == CAM_FRM_HEAD) || (info1 == CAM_FRM_CHECK) )
	{
		pdata[pos++] = CAM_FRM_CHECK;
		pdata[pos++] = (info1 ^ CAM_FRM_CHECK);
	}
	else
	{
		pdata[pos++] = info1;
	}
	
	if ( (info2 == CAM_FRM_HEAD) || (info2 == CAM_FRM_CHECK) )
	{
		pdata[pos++] = CAM_FRM_CHECK;
		pdata[pos++] = (info2 ^ CAM_FRM_CHECK);
	}
	else
	{
		pdata[pos++] = info2;
	}
	
	ck_sum = check_sum( &pdata[0x01], (pos - 1));
	
	isum = (ck_sum & 0xFF00) >> 0x08;
	
	if ( (isum == CAM_FRM_HEAD) || (isum == CAM_FRM_CHECK) )
	{
		pdata[pos++] = CAM_FRM_CHECK;
		pdata[pos++] = (isum ^ CAM_FRM_CHECK);
	}
	else
	{
		pdata[pos++] = isum;
	}
	
	isum = (uint8_t) (ck_sum & 0x00FF);
	
	if ( (isum == CAM_FRM_HEAD) || (isum == CAM_FRM_CHECK) )
	{
		pdata[pos++] = CAM_FRM_CHECK;
		pdata[pos++] = (isum ^ CAM_FRM_CHECK);
	}
	else
	{
		pdata[pos++] = isum;
	}
	
	pdata[pos++] = CAM_FRM_HEAD;
	
	return pos;
}

static void camera_uart_clear(void)
{
	camera_uart_mode = RE_A1_STOP;
	camera_uart_index = 0;
	camera_last_byte = FALSE;
}
static void camera_uart_start(void)
{
	camera_uart_mode = RE_A1_START;
	camera_uart_index = 0;
	camera_last_byte = FALSE;
}
static void camera_uart_stop(void)
{
	camera_uart_mode = RE_A1_STOP;
	camera_last_byte = FALSE;
}

portBASE_TYPE camera_data_parse(uint8_t *data_ptr)
{
	esn_msg_t esn_msg;
	portBASE_TYPE xTaskWoken;
	switch (data_ptr[0x02])
	{
	case CAM_CMD_PHONE:
		esn_msg.event = (GAIN_CAM << 8) | ENUM_PHOTO_ACK;
		xQueueSendFromISR(cam_send_queue, &esn_msg, &xTaskWoken);
		break;
		
	case CAM_CMD_DATA:
		esn_msg.event = (GAIN_CAM << 8) | ENUM_DATA_ACK;
		xQueueSendFromISR(cam_send_queue, &esn_msg, &xTaskWoken);
		break;
		
	case CAM_CMD_POWERDOWN:
		esn_msg.event = (GAIN_CAM << 8) | ENUM_DOWN_ACK;
		xQueueSendFromISR(cam_send_queue, &esn_msg, &xTaskWoken);
		break;
		
	default:
		break;
	}
	
	return xTaskWoken;
}

static bool_t camera_recv_ch_cb(uint8_t id, uint8_t ch)
{
	portBASE_TYPE xTaskWoken = pdFALSE;
	
	if (camera_uart_mode == RE_A1_STOP)
	{
		return xTaskWoken;
	}
	
	if ( camera_last_byte == TRUE)
	{
		camera_uart_buf[camera_uart_index++] = (ch ^ CAM_FRM_CHECK);
		camera_last_byte = FALSE;
		return xTaskWoken;
	}
	
	if ( ch == CAM_FRM_CHECK)
	{
		camera_last_byte = TRUE;
		return xTaskWoken;
	}
	
	switch (camera_uart_mode)
	{
	case RE_A1_START:
		if (ch == CAM_FRM_HEAD)
		{
			camera_uart_mode = RE_A1_RUN;
			camera_uart_buf[camera_uart_index++] = ch;
		}
		break;
		
	case RE_A1_RUN:
		camera_uart_buf[camera_uart_index++] = ch;
		if (ch == CAM_FRM_HEAD)
		{
			camera_uart_buf[camera_uart_index] = 0x00;
			camera_uart_mode = RE_A1_STOP;
			xTaskWoken = camera_data_parse(camera_uart_buf);
			camera_uart_stop();
		}
		break;
	case RE_A1_STOP:
		break;
	default:
		break;
	}
	
	return xTaskWoken;
}

void camera_frm_send(uint8_t cmd, uint8_t info1, uint8_t info2)
{
	uint8_t cmd_buf[0x20];
	uint8_t len = 0x00;
	osel_memset(cmd_buf, 0x00, sizeof(cmd_buf));
	
	len = camera_frm_create(cmd_buf, cmd, info1, info2);
	uart_send_string(CAM_PORT, cmd_buf, len);
}


void camera_init(uint8_t uart_id, uint32_t baud,
				 QueueHandle_t queue,
				 camera_data_cb_t cb)
{
	camera_uart_clear();
	
	uart_init(uart_id, baud);
	
	camer_data_cb = cb;
	cam_send_queue = queue;
	uart_int_cb_reg(uart_id, camera_recv_ch_cb);
}

void camera_cmd(uint8_t cmd, uint8_t cnt)
{
	camera_uart_start();
	switch (cmd)
	{
	case CAM_CMD_PHONE:
		camera_frm_send(cmd, PHOTO_INFOR1, PHOTO_INFOR2);
		break;
		
	case CAM_CMD_DATA:
		camera_frm_send(cmd, 0x00, cnt);
		break;
		
	case CAM_CMD_POWERDOWN:
		camera_frm_send(cmd, 0x00, 0x00);
		break;
		
	default:
		break;
	}
}

bool_t camera_status_get(void)
{
	return camera_has_start;
}

void camera_status_clr(void)
{
	camera_has_start = FALSE;
}


void camera_handle(uint16_t cmd)
{
	cmd_temp = cmd & 0x00FF;
	switch (cmd_temp)
	{
	case CAM_CMD_SEND:		//*< 有报警需要直接发送数据
		{
			cam_new_tick = xTaskGetTickCount();
			cam_old_tick = cam_new_tick;
			camera_has_start = TRUE;
			cmd_temp = CAM_CMD_PHONE;
			camera_cmd(cmd_temp, 0);
		}
		break;
	case CAM_CMD_PHONE:
		{
			bool_t cam_can_sent = FALSE;
			
			cam_new_tick = xTaskGetTickCount();
			if(cam_old_tick == 0)
			{
				cam_old_tick = cam_new_tick;
				cam_can_sent = TRUE;
			}
			else if (cam_new_tick > cam_old_tick)
			{
				//*< 600S以内只触发一次
				if ((cam_new_tick - cam_old_tick) > 600 * configTICK_RATE_HZ)
				{
					cam_can_sent = TRUE;
					cam_old_tick = cam_new_tick;
				}
			}
			else
			{
				if (((portMAX_DELAY - cam_old_tick) + cam_new_tick) > 600 * configTICK_RATE_HZ)
				{
					cam_can_sent = TRUE;
					cam_old_tick = cam_new_tick;
				}
			}
			
			if (cam_can_sent)
			{
				cam_can_sent = FALSE;
				camera_has_start = TRUE;
				camera_cmd(cmd_temp, 0);
			}
			break;
		}
		
	case ENUM_PHOTO_ACK:
		photo_info(camera_uart_buf);
		cmd_temp = CAM_CMD_DATA;
		photo_pack_pos = 0x00;
		camera_cmd(cmd_temp, photo_pack_pos);
		break;
		
	case ENUM_DATA_ACK:
		photo_pack_pos++;
		if (camer_data_cb != NULL)
		{
			camer_data_cb(photo_pack_sum,
						  photo_pack_pos,
						  &camera_uart_data_buf[4], CAM_FRM_MAX_LEN);
		}
		
		osel_memset(camera_uart_data_buf, 0x00, sizeof(camera_uart_data_buf));
		if (photo_pack_pos < photo_pack_sum)
		{
			cmd_temp = CAM_CMD_DATA;
			camera_cmd(cmd_temp, photo_pack_pos);
		}
		else
		{
			cmd_temp = CAM_CMD_POWERDOWN;
			camera_cmd(cmd_temp, 0x00);
		}
		break;
		
	default:
		break;
	}
}


