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


static bool_t range_can_sent = FALSE;
static TickType_t range_old_tick = 0; //*< 4字节
static TickType_t range_new_tick = 0;
    
static void range_app_handle(void)
{
    static uint8_t range_time_cnt = 0;  //*< 1秒调用一次接口
    static uint16_t distance_time_cnt = 0;
    fp32_t distance = 1000.0;
    if(++range_time_cnt < 60)           //*< 1分钟调用一次距离传感器
    {
        distance = 1000.0;
    }
    else
    {
        range_time_cnt = 0;
        esn_msg_t esn_msg;
        esn_msg.event = GAIN_RANGE_START;
        xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
	
        vTaskDelay(5);  //*< 延时10ms采样一下数据
        
        //*< 获取到距离数据
        fp32_t distance = range_sensor_get();
    }
	
	if (distance < RANGE_MIN_THRESHOLD)
	{
        range_new_tick = xTaskGetTickCount();
        if (range_new_tick > range_old_tick)
        {
            //*< 10分钟以内只触发一次
            if ((range_new_tick - range_old_tick) > RANGE_DATA_TIME * configTICK_RATE_HZ)
            {
                range_old_tick = range_new_tick;
                range_can_sent = TRUE;
            }
        }
        else
        {
            if (((portMAX_DELAY - range_old_tick) + range_new_tick) > RANGE_DATA_TIME * configTICK_RATE_HZ)
            {
                range_old_tick = range_new_tick;
                range_can_sent = TRUE;
            }
        }
        
        if (range_can_sent)
        {
            range_can_sent = FALSE;
            /** 刷新计数器 */
            distance_time_cnt = 0;
            esn_msg_t esn_msg;
            esn_msg.event = GAIN_RANGE_SEND;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
            
            //@note 启动摄像头采集数据
            esn_msg.event = GAIN_CAM_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
        }
	}
	
	if (distance_time_cnt++ > RANGE_DATA_TIME)
	{
		distance_time_cnt = 0;
        
        distance_time_cnt = 0;
        esn_msg_t esn_msg;
        esn_msg.event = GAIN_RANGE_SEND;
        xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
	}
}

static void angle_app_handle(void)
{
    static uint16_t angle_time_cnt = 0;
    
    if(angle_time_cnt++ >= ANGLE_DATA_TIME)
    {
        angle_time_cnt = 0;
        
        esn_msg_t esn_msg;
        esn_msg.event = GAIN_ANGLE_START;
        xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
    }
}

static fp32_t temp_sensor_get(void)
{
	//@TODO: 该温度采集接口需要更改
	return 32.0;
}

//static void temp_app_handle(void)
//{
//	
//	static uint16_t temp_time_cnt = 0;
//	fp32_t temp = temp_sensor_get();
//	
//	if (temp > TEMP_MAX_THRESHOLD)
//	{
//		temp_time_cnt = 0;
//		//@TODO: 添加温度异常报警发送接口
//		//
//	}
//	
//	if (temp_time_cnt++ > TEMP_DATA_TIME)
//	{
//		temp_time_cnt = 0;
//		//@TODO: 添加温度异常报警发送接口
//		//
//	}
//}

static void atmos_app_handle(void)
{
	static uint16_t atmos_time_cnt = 0;
	
	if (atmos_time_cnt++ > ATMOS_DATA_TIME)
	{
		atmos_time_cnt = 0;
		//@note 启动气象站采集数据
		esn_msg_t esn_msg;
		esn_msg.event = GAIN_ATMO_START;
		xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
	}
}

uint8_t a[] = ":010322A01F0038C28F3CF5333341E7666642743E144468000C000166664256000042F70201A6";
static void test_app_handle(void)
{
	//@TODO:加速度
//    static uint16_t test_time_cnt = 0;
//    if(test_time_cnt++ > 10)
//    {
//        test_time_cnt = 0;
//        acceleration_t info;
//        osel_memset(&info, 0, sizeof(acceleration_t));
//        mac_addr_get(info.bmonitor);
//        info.collect_time = 0;
//        info.x = 0x0010;
//        info.y = 0x2010;
//        info.z = 0x3010;
//        acceleration_send((uint8_t *)&info,sizeof(acceleration_t));
//    }
	
	//@TODO:气象
//	atmos_recv_data_handle(a,50);
	
	//@TODO 照相
	uint8_t pdata[170];
	camera_t info;
	osel_memset(&info, 0, sizeof(camera_t));
	mac_addr_get(info.bmonitor);
	info.collect_time = 0;
	info.cnt = 10;
	info.index = 1;
	osel_memset(pdata, 0, 170);
	pdata[0]=0xff;
	pdata[1]=0xd8;
	pdata[169] = 0xee;
	//camera_send(&info, (uint8_t *)pdata, 170);
	osel_delay(configTICK_RATE_HZ*20);
}

static void camera_app_handle(void)
{
	static uint16_t camera_time_cnt = 0;
	
	if (camera_status_get())
	{
		camera_status_clr();
		/** 刷新计数器 */
		camera_time_cnt = 0;
	}
	
	if (camera_time_cnt++ > CAMERA_DATA_TIME)
	{
		camera_time_cnt = 0;
		//@note 启动摄像头采集数据
		esn_msg_t esn_msg;
		esn_msg.event = GAIN_CAM_START;
		xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
	}
}

void esn_detect_task(void *param)
{
	while (1)
	{
		vTaskDelay(1*configTICK_RATE_HZ - 65); //*< 5s采集一次原始数据
#if 1
        
        angle_app_handle();
        
        range_app_handle();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        camera_app_handle();
        vTaskDelay(200 / portTICK_PERIOD_MS);

        atmos_app_handle();
        vTaskDelay(200 / portTICK_PERIOD_MS);
#else
		test_app_handle();
#endif
	}
}

bool_t esn_detect_init(void)
{
	xTaskCreate(esn_detect_task,
				"esn detect task",
				300,
				NULL,
				ESN_DETECT_PRIORITY,
				NULL);
	
	return TRUE;
}
