/**
 * @brief       : this
 * @file        : cam.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-01
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-01  v0.0.1  gang.cheng    first version
 */
#ifndef __CAM_H__
#define __CAM_H__

#include "osel_arch.h"

#define CAM_PORT                (UART_2)

#define CAM_FRM_MAX_LEN         (512)

#define GAIN_CAM                (0x00)

#define CAM_ADDR                (CAM_ADDR_BASE + CAM_ADDR_OFFSET)
#define CAM_ADDR_BASE           (0x10)
#define CAM_ADDR_OFFSET         (0x00)

#define CAM_FRM_HEAD            (0x7E)
#define CAM_FRM_CHECK           (0x1B)

#define CAM_CMD_PHONE           (0x00)
#define CAM_CMD_DATA            (0x02)
#define CAM_CMD_POWERDOWN       (0x0D)

#define PHOTO_INFOR1            0x00
#define PHOTO_INFOR2            0x12//0x04,0x14 //图片格式05：表示160x120小图   11表示320x240中图13: 表示640x480大图


typedef enum {
    RE_A1_START,
    RE_A1_RUN,
    RE_A1_STOP,
} re_mode_enum_t;

typedef enum {
    ENUM_PHOTO_ACK = 0x10,
    ENUM_DATA_ACK  = 0x12,
    ENUM_DOWN_ACK  = 0x1D,
} cmd_ack_enum_t;

typedef void (*camera_data_cb_t)(uint8_t frames_cnt,
                                 uint8_t index,
                                 uint8_t *pdata, uint16_t len);

void camera_init(uint8_t uart_id, uint32_t baud,
                 QueueHandle_t queue,
                 camera_data_cb_t cb);

void camera_cmd(uint8_t cmd, uint8_t cnt);

bool_t camera_status_get(void);

void camera_status_clr(void);

void camera_handle(uint16_t cmd);

#endif
