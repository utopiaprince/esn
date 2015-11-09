#ifndef __ATMOSPHERE_H__
#define __ATMOSPHERE_H__

#include "osel_arch.h"
#include "pbuf.h"
#include "sbuf.h"
#include "prim.h"

#define     MODBUS_CONTROL_PORT         P3DIR           // P10.5 LED1
#define     MODBUS_CONTROL_PIN         	BIT6
#define		MODBUS_DIR(x)				if((x) == 1){MODBUS_CONTROL_PORT |=  MODBUS_CONTROL_PIN;}else{MODBUS_CONTROL_PORT &=  ~MODBUS_CONTROL_PIN;}
#define     MODBUS_TX()					P3OUT |= MODBUS_CONTROL_PIN
#define		MODBUS_RX()					P3OUT &= ~MODBUS_CONTROL_PIN

#define ATMO_PORT                   (UART_1)

#define GAIN_ATMO                   (0x01)

#define ATMOS_CMD_START             (0x00)
#define ATMOS_START_IDLE            (0x10)
#define ATMOS_START_RUNING          (0x11)
#define ATMOS_START_END             (0x12)

typedef void (*atmos_data_cb_t)(uint8_t *pdata, uint16_t len);

void atmos_sensor_init(uint8_t uart_id, uint32_t baud, 
                       QueueHandle_t queue,
                       atmos_data_cb_t cb);

void atmos_handle(esn_msg_t *msg);

#endif
