/**
 * @brief       : this
 * @file        : lora.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-01
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-01  v0.0.1  gang.cheng    first version
 */
#ifndef __LOAR_H__
#define __LOAR_H__

#define UM_REG_TYPE				0x00
#define UM_REG_VER				0x01
#define UM_REG_FREQ_3			0x02
#define UM_REG_FREQ_2			0x03
#define UM_REG_FREQ_1			0x04
#define UM_REG_RF_BPS			0x05
#define UM_REG_POWER			0x06
#define UM_REG_UART_BPS			0x07
#define UM_REG_UART_CHECK		0x08
#define UM_REG_WAKEUP_CYCLE		0x09
#define UM_REG_WAKEUP_DELAY		0x0a
#define UM_REG_CCA_DELAY		0x0b
#define UM_REG_FRM_LEN			0x0c
#define UM_REG_HEAD_H			0x0d
#define UM_REG_HEAD_L			0x0e
#define UM_REG_FEATURE			0x0f

typedef enum 
{
	NORMAL_MODE,
	WAKEUP_MODE,
	LOWPOW_MODE,
	SETTING_MODE,
};

void lora_init(void);

#endif
