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

#define GAIN_CAM_START      ((GAIN_CAM<<8) | CAM_CMD_PHONE)

#define GAIN_ATMO_START     ((GAIN_ATMO)<<8 | ATMOS_CMD_START)

extern QueueHandle_t esn_gain_queue;

void esn_gain_init(void);

#endif
