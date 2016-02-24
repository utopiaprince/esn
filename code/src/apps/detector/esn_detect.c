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

static void range_app_handle(void)
{
    static uint8_t range_time_cnt = 0;  //*< 1秒调用一次接口
    static uint16_t distance_time_cnt = 0;
    static bool_t range_can_sent = FALSE;
    static TickType_t range_old_tick = 0; //*< 4字节
    static TickType_t range_new_tick = 0;

    fp32_t distance = 1000.0;
    if(++range_time_cnt < 120)          //*< 2分钟调用一次距离传感器
    {
        distance = 1000.0;
    }
    else
    {
        range_time_cnt = 0;
        esn_msg_t esn_msg;
        esn_msg.event = GAIN_RANGE_START;
        xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
        vTaskDelay(100 / portTICK_PERIOD_MS);  //*< 延时100ms采样一下数据
        //*< 获取到距离数据
        distance = range_sensor_get();
        
        if (distance <= 1.0)
        {
            //*< 在采集一次数据
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
            vTaskDelay(500 / portTICK_PERIOD_MS);  //*< 延时500ms采样一下数据
            distance = range_sensor_get();
        }
    }
	
	if (distance < RANGE_MIN_THRESHOLD)
	{
        range_new_tick = xTaskGetTickCount();
        if (range_new_tick > range_old_tick)
        {
            //*< 100s以内只触发一次
            if ((range_new_tick - range_old_tick) > (100 * configTICK_RATE_HZ))
            {
                range_old_tick = range_new_tick;
                range_can_sent = TRUE;
            }
        }
        else
        {
            if (((portMAX_DELAY - range_old_tick) + range_new_tick) > (100 * configTICK_RATE_HZ))
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
#if CAMERA_ENABLE
            esn_msg.event = GAIN_CAM_START;
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
#endif
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

void esn_detect_task(void *param)
{
    uint16_t time_ms = 1000;
    while (1)
    {
        time_ms = 1000;
        //@todo: realy sensor detect  
#if ANGLE_ENABLE 
        angle_app_handle();
#endif
        
#if ANGLE_ENABLE
        range_app_handle();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        time_ms -= 200;
#endif   
        
#if CAMERA_ENABLE
        camera_app_handle();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        time_ms -= 200;
#endif

#if ATMOS_ENABLE
        atmos_app_handle();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        time_ms -= 200;
#endif
        
        vTaskDelay(time_ms / portTICK_RATE_MS);
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
