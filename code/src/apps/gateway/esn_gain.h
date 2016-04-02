/**
 * @brief       : this
 * @file        : esn_gain.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-21
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-21  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_GAIN_H__
#define __ESN_GAIN_H__

typedef struct
{
    uint16_t a;
    uint16_t b;
} f_t;


#define GAIN_CAM_START      ((GAIN_CAM<<8) | CAM_CMD_PHONE)
#define GAIN_CAM_SEND_START ((GAIN_CAM<<8) | CAM_CMD_SEND)

#define GAIN_ATMO_START     ((GAIN_ATMO<<8) | ATMOS_CMD_START)

#define GAIN_RANGE_START	((GAIN_RANGE<<8) | RANGE_CMD_START)
#define GAIN_RANGE_SEND     ((GAIN_RANGE<<8) | RANGE_DATA_SEND)

#define GAIN_ANGLE_START    ((GAIN_ANGLE<<8) | ANGLE_CMD_START) 

#define GAIN_STOCK_START    ((GAIN_STOCK<<8) | STOCK_CMD_START)

extern QueueHandle_t esn_gain_queue;

void esn_gain_init(void);

void atmos_recv_data_handle(uint8_t *pdata, uint16_t len);

void cache_alarm(uint8_t alarm);

void toINT(void *des);

void tofloat(void *des);

void CharToHex(char * dest, char * buffer , int len);
#endif
