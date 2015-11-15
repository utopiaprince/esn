/**
* @brief       : this is cycle detect time for sensor
* @file        : esn_detect.c
* @version     : v0.0.1
* @author      : gang.cheng
* @date        : 2015-08-05
* change logs  :
* Date       Version     Author        Note
* 2015-08-05  v0.0.1  gang.cheng    first version
*/
#include "osel_arch.h"
#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

#include "drivers.h"
#include "esn.h"
#include "esn_package.h"
DBG_THIS_MODULE("esn_gain")

QueueHandle_t esn_gain_queue = NULL;

// TickType_t xTaskGetTickCount( void )

typedef struct{
	uint16_t a;
	uint16_t b;
}f_t;

static void CharToHex(char * dest, char * buffer , int len)
{
	int i = 0;
	int j = 0;
	unsigned char temp;
	while (i < len)
	{
		temp = buffer[i];
		if ((temp >= 0x30) && (temp <= 0x39))
		{
			temp = temp - 0x30;
			dest[j] = temp << 4;
		}
		else if ((temp >= 0x41) && (temp <= 0x46))
		{
			temp = temp - 0x41 + 0x0A;
			dest[j] = temp << 4;
		}
		else if ((temp >= 0x61) && (temp <= 0x66))
		{
			temp = temp - 0x61 + 0x0A;
			dest[j] = temp << 4;
		}
		else
		{
			dest[j] = 0x00;
		}
		temp = buffer[i + 1];
		if ((temp >= 0x30) && (temp <= 0x39))
		{
			temp = temp - 0x30;
			dest[j] = dest[j] | temp;
			
		}
		else if ((temp >= 0x41) && (temp <= 0x46))
		{
			temp = temp - 0x41 + 0x0A;
			dest[j] = dest[j] | temp;
		}
		else if ((temp >= 0x61) && (temp <= 0x66))
		{
			temp = temp - 0x61 + 0x0A;
			dest[j] = dest[j] | temp;
		}
		else
		{
			dest[j] = dest[j] | 0x00;
		}
		
		i = i + 2;
		j = j + 1;
	}
	return;
}

void tofloat(void *des)
{
	f_t *f = (f_t *)des;
	f->a = S2B_UINT16(f->a);
	f->b = S2B_UINT16(f->b);
}

void toINT(void *des)
{
	uint16_t *b = (uint16_t *)des;
	*b =S2B_UINT16(*b);
}

static void camera_recv_data_handle(uint16_t cnt, uint16_t index,
									uint8_t *pdata, uint16_t len)
{
	camera_t info;
	osel_memset(&info, 0, sizeof(camera_t));
	mac_addr_get(info.bmonitor);
	info.collect_time = 0;
	info.cnt = cnt;
	info.index = index;
	camera_send(&info, pdata, len);
	osel_delay(configTICK_RATE_HZ);
}

void atmos_recv_data_handle(uint8_t *pdata, uint16_t len)
{
	atmo_t info;
	osel_memset(&info, 0, sizeof(atmo_t));
	mac_addr_get(info.bmonitor);
	info.collect_time = 0;
	
	pdata += 7;
	CharToHex((char *)&info.atmo_data, (char *)pdata, 2*sizeof(atmo_data_t));
	atmo_data_t *adata = &info.atmo_data;
	toINT((uint16_t *)&adata->driver_state);
	toINT((uint16_t *)&adata->wind_direction);
	tofloat((f_t *)&adata->wind_speed);
	tofloat((f_t *)&adata->temperature);
	tofloat((f_t *)&adata->humidity);
	tofloat((f_t *)&adata->pressure);
	toINT((uint16_t *)&adata->compass);
	toINT((uint16_t *)&adata->rainfall_state);
	tofloat((f_t *)&adata->rainfall_streng);
	tofloat((f_t *)&adata->rainfall_total);
	toINT((uint16_t *)&adata->rainfall_streng_unit);
	
	atmo_send((uint8_t *)&info, sizeof(atmo_t));
}

static void range_recv_data_handle(fp32_t distance)
{
	;
}

static void esn_gain_task(void *param)
{
	uint8_t type;
	esn_msg_t esn_msg;
	TickType_t stock_old_tick = 0; //*< 4字节
	TickType_t stock_new_tick = 0;
	TickType_t cam_old_tick = 0; //*< 4字节
	TickType_t cam_new_tick = 0;
	while (1)
	{
		if (xQueueReceive(esn_gain_queue,
						  &esn_msg,
						  portMAX_DELAY))
		{
			type = esn_msg.event >> 8;
			switch (type)
			{
			case GAIN_CAM:
				{
					static bool_t cam_can_sent = TRUE;
					cam_new_tick = xTaskGetTickCount();
					if (cam_new_tick > cam_old_tick)
					{
						//*< 300S以内只触发一次
						if ((cam_new_tick - cam_old_tick) > 10 * configTICK_RATE_HZ)
						{
							cam_old_tick = cam_new_tick;
                            cam_can_sent = TRUE;
						}
					}
					else
					{
						if (((portMAX_DELAY - cam_old_tick) + cam_new_tick) > 10 * configTICK_RATE_HZ)
						{
							cam_old_tick = cam_new_tick;
                            cam_can_sent = TRUE;
						}
					}
					
					if (cam_can_sent)
					{
                        cam_can_sent = FALSE;
						camera_handle(esn_msg.event);
					}
					
					break;
				}
				
			case GAIN_ATMO:
				atmos_handle(&esn_msg);
				break;
                
            case GAIN_RANGE:
                range_handle(&esn_msg);
                break;
				
			case GAIN_STOCK:
				{
					static bool_t stock_can_sent = TRUE;
					stock_new_tick = xTaskGetTickCount();
					if (stock_new_tick > stock_old_tick)
					{
						//*< 10S以内只触发一次
						if ((stock_new_tick - stock_old_tick) > 10 * configTICK_RATE_HZ)
						{
							stock_old_tick = stock_new_tick;
                            stock_can_sent = TRUE;
						}
					}
					else
					{
						if (((portMAX_DELAY - stock_old_tick) + stock_new_tick) > 10 * configTICK_RATE_HZ)
						{
							stock_old_tick = stock_new_tick;
                            stock_can_sent = TRUE;
						}
					}
					
					if (stock_can_sent)
					{
                        stock_can_sent = FALSE;
						int16_t x, y, z;
						adxl_get_triple_angle(&x, &y, &z);
						//@TODO: 添加震动数据发送接口
						shock_t info;
						osel_memset(&info, 0, sizeof(shock_t));
						mac_addr_get(info.bmonitor);
						info.collect_time = 0;
						shock_send((uint8_t *)&info, sizeof(shock_t));
						
						//@note 启动摄像头采集数据
						esn_msg_t esn_msg;
						esn_msg.event = GAIN_CAM_START;
						xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
					}
					break;
				}
			default:
				break;
			}
		}
	}
}

void esn_gain_init(void)
{
	portBASE_TYPE res;
	
	res = xTaskCreate(esn_gain_task,
					  "esn_gain_task",
					  400,
					  NULL,
					  ESN_GAIN_PRIORITY,
					  NULL);
	
	if (res != pdTRUE)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn gain task init failed\r\n");
	}
	
	esn_gain_queue = xQueueCreate(10, sizeof(esn_msg_t));
	if (esn_gain_queue == NULL)
	{
		DBG_LOG(DBG_LEVEL_ERROR, "esn_gain_queue init failed\r\n");
	}
	
	adxl_sensor_init(); //*< 被动接收数据
	
	atmos_sensor_init(UART_1, 9600, esn_gain_queue, atmos_recv_data_handle);
	camera_init(UART_2, 9600, esn_gain_queue, camera_recv_data_handle);
//	range_sensor_init(UART_3, 9600, esn_gain_queue, range_recv_data_handle);
}




