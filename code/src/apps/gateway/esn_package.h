#pragma once
#include <data_type_def.h>
#include <pbuf.h>
#define ID_MAX  (17u)
enum frame_type_e
{
    FRAME_BEGIN = 0,
    DATA = 1,
    HEART = 7,
    FRAME_END,
};
enum message_type_e
{
    M_WIRE = 0x20,
    M_SHOCK = 0x30,
    M_DISTANCE = 0x31,
    M_TEMPERATURE = 0x32,
	M_ACCE		=0x33,
	M_ATMO		=0x34,
	M_CAME		=0x35,
};

#pragma pack(1)
typedef struct
{
	uint16_t temperature:1,		//温度
wind_direction_speed:1,			//风向风速
pressure:1,						//气压
compass:1,						//电子罗盘
hyetometer:1;					//雨量计
}driver_state_t;

typedef struct
{
	uint16_t reserve	:8,
mms:1,
mmm:1,
mmh:1;
}rainfall_streng_unit_t;
typedef struct
{
	driver_state_t driver_state;
	uint16_t wind_direction;	//风向:转换成十进制整型
	uint32_t wind_speed;		//风速:转换成浮点数
	uint32_t temperature;		//温度:转换成浮点数
	uint32_t humidity;			//湿度:转换成浮点数
	uint32_t pressure;			//气压:转换成浮点数
	uint16_t compass;			//电子罗盘
	uint16_t rainfall_state;	//降雨状态
	uint32_t rainfall_streng;	//降雨强度
	uint32_t rainfall_total;	//累积降雨量
	rainfall_streng_unit_t rainfall_streng_unit;	//降雨强度单位
}atmo_data_t;	//
#pragma pack()

#pragma pack(1)
typedef struct
{
	uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
	atmo_data_t atmo_data;
}atmo_t;
typedef struct
{
	uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
	uint16_t x;
	uint16_t y;
	uint16_t z;
}acceleration_t;	//加速度
typedef struct
{
    uint8_t umonitor[ID_MAX];
	uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
    uint8_t thresh_tap; //*< 震动阈值
    uint8_t dur;        //*< 震动持续时间
}shock_t;			//震动
typedef struct
{
	uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
	float val;		
}distance_t;		//距离
typedef struct
{
	uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
	float val;		
}temperature_t;		//温度

typedef struct
{
	uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
	uint16_t index;
	uint16_t cnt;
}camera_t;

typedef struct
{
    uint8_t umonitor[ID_MAX];
    uint8_t frame_type;
    uint8_t message_type;
    uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
    uint16_t alarm;
} esn_package_t;
#pragma pack()

bool_t esn_gprs_send(uint8_t *data, uint16_t length);
bool_t shock_send(uint8_t *pdata, uint16_t len);
bool_t distance_send(uint8_t *pdata, uint16_t len);
void atmo_send(uint8_t *pdata, uint16_t len);
void acceleration_send(uint8_t *pdata, uint16_t len);				//加速度:没有调用
void temperature_send(uint8_t *pdata, uint16_t len);						
void camera_send(camera_t *info, uint8_t *pdata, uint16_t len);


