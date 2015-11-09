#pragma once
#include <data_type_def.h>
#include <pbuf.h>
#define ID_MAX              (17u)

#define GAIN_DISTANCE       (8u)
#define GAIN_TEMPERATURE    (9u)
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
};

#pragma pack(1)
typedef struct
{
    uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
    float val;
} esn_part_t;

typedef struct
{
    uint8_t umonitor[ID_MAX];
    uint8_t frame_type;
    uint8_t message_type;
    uint8_t bmonitor[ID_MAX];
    uint32_t collect_time;
    uint16_t alarm;
    float val;
} esn_package_t;
#pragma pack()

pbuf_t *shock_package(esn_part_t *info);                //震动
pbuf_t *distance_package(esn_part_t *info);             //距离
pbuf_t *temperature_package(esn_part_t *info);          //温度
