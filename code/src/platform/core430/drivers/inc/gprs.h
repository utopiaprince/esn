#pragma once
#include <data_type_def.h>

#define GPRS_HEART	(10u)
#define GPRS_HEART_START        ((GPRS_HEART<<8) | 0)
#define GPRS_CNN	(11u)
#define GPRS_CNN_START        ((GPRS_CNN<<8) | 0)
typedef enum
{
	READY_IDLE,
	SIM_ERROR,
	GPRS_NET_ERROR,
	WORK_ON,
	WORK_DOWN,
}GPRS_STATE_E;

typedef struct
{
	bool_t gprs_state;
	uint8_t uart_port;
	uint32_t uart_speed;
	uint8_t dip[4];
	uint16_t port;
	bool_t mode;
}gprs_info_t;

struct gprs
{
	void (*get)(gprs_info_t *const info);
	void (*set)(const gprs_info_t *const info);
    bool_t (*init)(void);                               
    bool_t (*deinit)(void);                            
    void (*read)(void *cb);
    bool_t (*write)(const uint8_t *const payload, const uint16_t len);   
};

/**
* @brief 串口中断调用
* @param[in] 接收到的串口字符
*/
bool_t gprs_uart_inter_recv(uint8_t id, uint8_t ch);

extern const struct gprs gprs_driver;