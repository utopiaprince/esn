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

static void range_app_handle(void)
{
    static uint16_t distance_time_cnt = 0;
    esn_msg_t esn_msg;
    esn_msg.event = GAIN_RANGE_START;
    xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);

    vTaskDelay(1);  //*< 延时10ms采样一下数据

    //*< 获取到距离数据
    fp32_t distance = range_sensor_get();

    if (distance < RANGE_MIN_THRESHOLD)
    {
        /** 刷新计数器 */
        distance_time_cnt = 0;
        //@TODO: 添加测距异常数据发送接口
        //
        //

        //@note 启动摄像头采集数据
        esn_msg_t esn_msg;
        esn_msg.event = GAIN_CAM_START;
        xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);

    }

    if (distance_time_cnt++ > RANGE_DATA_TIME)
    {
        distance_time_cnt = 0;
        //@TODO: 添加测距异常数据发送接口
        //
        //
    }
}

static fp32_t temp_sensor_get(void)
{
    //@TODO: 该温度采集接口需要更改
    return 32.0;
}

static void temp_app_handle(void)
{

    static uint16_t temp_time_cnt = 0;
    fp32_t temp = temp_sensor_get();

    if (temp > TEMP_MAX_THRESHOLD)
    {
        temp_time_cnt = 0;
        //@TODO: 添加温度异常报警发送接口
        //
    }

    if (temp_time_cnt++ > TEMP_DATA_TIME)
    {
        temp_time_cnt = 0;
        //@TODO: 添加温度异常报警发送接口
        //
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
        vTaskDelay(configTICK_RATE_HZ - 1); //*< 1s采集一次原始数据
        //adxl_get_xyz(&x, &y, &z);

        range_app_handle();
        camera_app_handle();
        temp_app_handle();
        atmos_app_handle();

//        if (time_cnt++ > DETECT_TIME)   //*< 30S 采样一次CAM
//        {
//            time_cnt = 0;
//            esn_msg_t esn_msg;
//            esn_msg.event = GAIN_CAM_START;
//            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
//
//            esn_msg.event = GAIN_ATMO_START;
//            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
//
//            //@todo: test
//#include "esn_package.h"
//#define GAIN_DISTANCE_START     ((GAIN_DISTANCE<<8) | 0)
//#define GAIN_TEMPERATURE_START  ((GAIN_TEMPERATURE<<8) | 0)
//            esn_msg.event = GAIN_STOCK_START;
//            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
//            vTaskDelay(1000 / portTICK_RATE_MS);
//            esn_msg.event = GAIN_DISTANCE_START;
//            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
//            vTaskDelay(1000 / portTICK_RATE_MS);
//            esn_msg.event = GAIN_TEMPERATURE_START;
//            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
//            vTaskDelay(1000 / portTICK_RATE_MS);
//        }
    }
}

bool_t esn_detect_init(void)
{
    xTaskCreate(esn_detect_task,
                "esn detect task",
                200,
                NULL,
                ESN_DETECT_PRIORITY,
                NULL);

    return TRUE;
}
