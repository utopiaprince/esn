/**
 * @brief       : this
 * @file        : esn_frames.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-10
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-10  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_FRAMES_H__
#define __ESN_FRAMES_H__


typedef enum
{
    ESN_DATA = 0,
    ESN_CTRL = 1,
} esn_frame_enum_t;

typedef enum
{
    ALARM_N = 0,
    ALARM_T = 1,
} esn_alarm_enum_t;

typedef enum
{
    DATATYPE_VIBRATION   = 0,
    DATATYPE_DISTANCE,
    DATATYPE_PICTURE ,
    DATATYPE_ANGLE   ,
    DATATYPE_ATMO    ,
} ens_frame_data_enum_t;

typedef enum
{
    PICTURE_START = 0,
    PICTURE_DATA  = 1,
    PICTURE_END   = 2,
} esn_frame_picture_enum_t;

#pragma pack(1)

typedef struct
{
    uint8_t frame_type  : 2,
            alarm       : 1,
            data_type   : 5;
} esn_frames_ctrl_t;

typedef struct
{
    esn_frames_ctrl_t frames_ctrl;
    uint8_t           seq;
    uint8_t           nums;
    uint8_t           sub_num;
} esn_frames_head_t;

typedef struct
{
    uint8_t time_duty;
    uint8_t alx;
} esn_vibration_payload_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} esn_angle_payload_t;

typedef struct
{
    fp32_t distance;
} esn_distance_payload_t;


typedef struct 
{
    uint16_t cnt;
    uint16_t index;
    uint8_t buf[128];
} esn_camera_payload_t;

#pragma pack()


#define CAMRA_PAYLOAD_SUB_SIZE  (0x20)

#endif
