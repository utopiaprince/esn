#include "gprs.h"
#include <prim.h>
#include <osel_arch.h>
#include <hal_timer.h>
#include <uart.h>
#include <drivers.h>

#define GPRS_EVENT		(0x0100)

#define SEND_SIZE		(200)
#define SIZE			(30)
#define CMD_CB_NUM		(20u)
#define	PULL_UP			(P9OUT &= ~BIT7)
#define	PULL_DOWN		(P9OUT |= BIT7)
#define GPRS_DETECT_STATUS()	(P10IN & BIT0)

#define AT			("AT\r")
#define ATE0		("ATE0\r")
#define CSMINS		("AT+CSMINS?\r")
#define CGATT		("AT+CGATT?\r")
#define CIPSTART	("AT+CIPSTART=%s,%s\r")
#define CIPCLOSE	("AT+CIPCLOSE\r")
#define CIPSEND		("AT+CIPSEND=%d\r")

static const uint8_t tcp_mode[] = {"\"TCP\""};
static const uint8_t udp_mode[] = {"\"UDP\""};
static uint8_t ipconfig[50];
static uint8_t send_data[SIZE];
static uint8_t GPRS_AT[][6] = {AT, "OK"};
static uint8_t GPRS_ATE0[][6] = {ATE0, "OK"};
static uint8_t GPRS_CSMINS[][20] = {CSMINS, "OK"};
static uint8_t GPRS_CGATT[][20] = {CGATT, "OK"};
static uint8_t GPRS_CIPSTART[][20] = {CIPSTART, "OK"};
static uint8_t GPRS_CIPCLOSE[][20] = {CIPCLOSE, "OK"};
static uint8_t GPRS_CIPSEND[][20] = {CIPSEND, "\n>"};

typedef void (*gprs_send_cb)(void);
typedef	void (*gprs_read_cb_t)(const uint8_t *const buf, const uint8_t len);
gprs_read_cb_t gprs_read_cb;
static xQueueHandle gprs_queue = NULL;
static xSemaphoreHandle gprs_mutex = NULL;

static void gprs_switch(void);
static void port_init(void);
typedef struct
{
	uint8_t *cmd;
	uint8_t *result;
	gprs_send_cb cb;
} send_cb_t;

typedef enum
{
	E_IDLE,
	E_UP,
	E_DOWN,
	E_CLOSE,
	E_REST,
	E_SEND,
	E_SEND_OK,
} E_SATE_E;

typedef struct
{
	uint8_t buf[SEND_SIZE];
	uint8_t len;
	GPRS_STATE_E state;
} send_t;

typedef struct
{
	uint8_t buf[SIZE];
	uint8_t len;
	uint8_t offset;
} recv_t;

static gprs_info_t gprs_info;
static volatile E_SATE_E e_state = E_CLOSE;
static hal_timer_t *gprs_switch_timer = NULL;
static send_t send;
static recv_t recv;


static bool_t write_fifo( uint8_t *payload,  uint16_t len);
static bool_t gprs_init();

/**
*实现strstr函数功能
*
*@param: 两个字符串
*@return: 返回在str中出现sub_str之后的字符串。
*
*/
static char const *my_strstr(const char *str, const char *sub_str)
{
	DBG_ASSERT(str != NULL __DBG_LINE);
	DBG_ASSERT(sub_str != NULL __DBG_LINE);

	for (int i = 0; str[i] != '\0'; i++)
	{
		int tem = i;
		int j = 0;

		while (str[i++] == sub_str[j++])
		{
			if (sub_str[j] == '\0')
			{
				return &str[tem];
			}
		}
		i = tem;
	}
	return NULL;
}

static uint8_t mystrlen(const char *str)
{
	DBG_ASSERT(str != NULL __DBG_LINE);

	uint8_t len = 0;

	while ((*str++) != '\0')
	{
		if (len++ > 127)
			DBG_ASSERT(FALSE __DBG_LINE);
	}
	return len;
}

static uint8_t *ustrchr(uint8_t *s, const uint8_t *const c)
{
	DBG_ASSERT(s != NULL __DBG_LINE);
	DBG_ASSERT(c != NULL __DBG_LINE);
	while (*s != '\0')
	{
		s++;
		if (*s == *c)
			return s;
	}
	return NULL;
}

static void ipconfig_get(uint8_t *cmd, uint8_t len)
{
	uint8_t ip_config[30];
	osel_memset(cmd, 0x00, len);
	osel_memset(ip_config, 0x00, 30);
	tfp_sprintf((char *)ip_config,
	            "\"%d.%d.%d.%d\",%u",
	            gprs_info.dip[0], gprs_info.dip[1], gprs_info.dip[2], gprs_info.dip[3], gprs_info.port);
	if (gprs_info.mode)
	{
		tfp_sprintf((char *)cmd,
		            (char *)GPRS_CIPSTART[0],
		            (char *)tcp_mode, (char *)ip_config);
	}
	else
	{
		tfp_sprintf((char *)cmd,
		            (char *)GPRS_CIPSTART[0],
		            (char *)udp_mode, (char *)ip_config);
	}
}

static bool_t write_fifo( uint8_t *payload,  uint16_t len)
{
	DBG_ASSERT(payload != NULL __DBG_LINE);
	if (len > SEND_SIZE)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}
	osel_memset(recv.buf, 0 , SIZE);
	recv.offset = 0;
	uart_send_string(gprs_info.uart_port, payload, len);
	return TRUE;
}

static bool_t at_deal(void)
{
	if (my_strstr((const char*)recv.buf, (const char*)GPRS_AT[1]) != NULL)
		return TRUE;
	else
		return FALSE;
}

static bool_t ate0_deal(void)
{
	if (my_strstr((const char*)recv.buf, (const char*)GPRS_ATE0[1]) != NULL)
		return TRUE;
	else
		return FALSE;
}

static bool_t csmins_deal(void)
{
	uint8_t* ptr = ustrchr(recv.buf, ",");
	if (ptr != NULL)
	{
		ptr++;
		if (*ptr == 0x31)
		{
			return TRUE;
		}
	}
	gprs_info.gprs_state = SIM_ERROR;
	return FALSE;
}

static bool_t cgatt_deal(void)
{
	uint8_t* ptr = ustrchr(recv.buf, ":");
	if (ptr != NULL)
	{
		ptr += 2;
		if (*ptr == 0x31)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	gprs_info.gprs_state = GPRS_NET_ERROR;
	return FALSE;
}

static bool_t cipstart_deal(void)
{
	if (my_strstr((const char*)recv.buf, (const char*)GPRS_CIPSTART[1]) != NULL)
		return TRUE;
	else
		return FALSE;
}

static void cipsend_ok_cb(void)
{
	_NOP();
	//@note: 发送成功以后需要释放资源
	xSemaphoreGive(gprs_mutex);
}

static void switch_rest(void)
{
	esn_msg_t esn_msg;
	osel_memset(&esn_msg, 0, sizeof(esn_msg_t));
	esn_msg.event = GPRS_EVENT;
	e_state = E_REST;
	xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
}

static void gprs_switch(void)
{
	esn_msg_t esn_msg;
	osel_memset(&esn_msg, 0, sizeof(esn_msg_t));
	esn_msg.event = GPRS_EVENT;
	if (e_state == E_SEND)
	{
		vTaskDelay(300 / portTICK_RATE_MS);
		write_fifo(send.buf, send.len);
	}
	if (e_state == E_SEND_OK)
	{
		cipsend_ok_cb();
	}
	else if (e_state == E_CLOSE)
	{
		gprs_info.gprs_state = WORK_DOWN;
		led_set(LEN_GREEN, FALSE);
		write_fifo(GPRS_CIPCLOSE[0],  sizeof(CIPCLOSE) - 1);
		vTaskDelay(2000 / portTICK_RATE_MS);
		e_state = E_UP;
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if (e_state == E_UP)
	{
		PULL_UP;
		e_state = E_DOWN;
		vTaskDelay(2000 / portTICK_RATE_MS);
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if (e_state == E_DOWN)
	{
		PULL_DOWN;
		e_state = E_IDLE;
		vTaskDelay(5000 / portTICK_RATE_MS);
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if (e_state == E_IDLE)
	{
		static uint8_t idle_num = 0;
		if (idle_num++ >= 20)
			DBG_ASSERT(FALSE __DBG_LINE);

		if ((GPRS_DETECT_STATUS() != FALSE))
		{
			idle_num = 0;
			uint8_t cgatt_num = 0;
			bool_t cgatt_mark = FALSE;
			uint8_t join_num = 0;
			bool_t join_mark = FALSE;

			gprs_info.gprs_state = READY_IDLE;
			write_fifo(GPRS_AT[0],  sizeof(AT) - 1);
			vTaskDelay(500 / portTICK_RATE_MS);

			if (at_deal())
				write_fifo(GPRS_ATE0[0], sizeof(ATE0) - 1);
			else
				switch_rest();
			vTaskDelay(500 / portTICK_RATE_MS);

			if (ate0_deal())
				write_fifo(GPRS_CSMINS[0], sizeof(CSMINS) - 1);
			else
				switch_rest();
			vTaskDelay(500 / portTICK_RATE_MS);

			if (csmins_deal())	//检查SIM卡
				write_fifo(GPRS_CGATT[0], sizeof(CGATT) - 1);
			else
				return;
			vTaskDelay(500 / portTICK_RATE_MS);

			while (cgatt_num++ < 20)
			{
				if (cgatt_deal())	//附着网络
				{
					cgatt_mark = TRUE;
					break;
				}
				else
					write_fifo(GPRS_CGATT[0], sizeof(CGATT) - 1);
				vTaskDelay(2000 / portTICK_RATE_MS);
			}
			if (cgatt_mark == FALSE)
			{
				switch_rest();
			}

			do
			{
				write_fifo(ipconfig, mystrlen((char *)ipconfig));
				vTaskDelay(11000 / portTICK_RATE_MS);
				if (cipstart_deal())
				{
					join_mark = TRUE;
					break;
				}
			}
			while (join_num++ < 2);
			if (join_mark == FALSE)
			{
				switch_rest();
			}

			gprs_info.gprs_state = WORK_ON;
			led_set(LEN_GREEN, TRUE);
		}
		else
		{
			switch_rest();
		}
	}
	else if (e_state == E_REST)
	{
		gprs_info.gprs_state = E_CLOSE;
		e_state = E_CLOSE;
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
}

static void get(gprs_info_t *const info)
{
	osel_memcpy(info, &gprs_info, sizeof(gprs_info_t));
}

static void set(const gprs_info_t *const info)
{
	osel_memcpy(&gprs_info, info, sizeof(gprs_info_t));
}

static void gprs_read_register(void *cb)
{
	if (cb != NULL)
		gprs_read_cb = (gprs_read_cb_t)cb;
}

static bool_t gprs_write_fifo(const uint8_t *const payload, const uint16_t len)
{
	DBG_ASSERT(payload != NULL __DBG_LINE);
	if (gprs_info.gprs_state == WORK_ON && len < SEND_SIZE)
	{
		osel_memset(send_data, 0x00, SIZE);
		tfp_sprintf((char *)send_data, CIPSEND, len);

		osel_memset(send.buf, 0x00, SEND_SIZE);
		osel_memcpy(send.buf, payload, len);
		send.len = len;

		//*< 如果2秒还没有获取到资源，直接释放需要发送的数据
		if (xSemaphoreTake(gprs_mutex, 2 * configTICK_RATE_HZ) == pdTRUE)
		{
			write_fifo(send_data, mystrlen((char *)send_data));
		}
		else
		{
			//TODO: 需要考虑如果发送以后不来TXOK的情况，是否设定多次重传失败机制或超时机制；
		}
	}
	return TRUE;
}

static void port_init(void)
{
	P9DIR |= BIT3;	//上电？
	P9OUT |= BIT3;

	P9DIR |= BIT6;	//关闭睡眠
	P9OUT &= ~BIT6;

	P9SEL &= ~BIT7;
	P9DIR |=  BIT7;

	P10SEL &= ~BIT0;
	P10DIR &= ~BIT0;
}

#define MAINTAIN_TIME	(10*60*1000llu)
static void gprs_maintain(void *p)
{
	while (1)
	{
		vTaskDelay(MAINTAIN_TIME / portTICK_RATE_MS);
		if (gprs_info.gprs_state != WORK_ON)
		{
			DBG_ASSERT(FALSE __DBG_LINE);
		}
	}
}

static void gprs_task(void *p)
{
	esn_msg_t esn_msg;
	osel_memset(&esn_msg, 0, sizeof(esn_msg_t));
	while (1)
	{
		xQueueReceive(gprs_queue,        //*< the handle of received queue
		              &esn_msg,          //*< pointer to data received
		              portMAX_DELAY);   //*< time out

		switch (esn_msg.event)
		{
		case GPRS_EVENT:
		{
			gprs_switch();
		}
		break;
		default:
			break;
		}
		osel_memset(&esn_msg, 0, sizeof(esn_msg_t));
	}
}

static bool_t gprs_init()
{
	port_init();

	uart_init(gprs_info.uart_port, gprs_info.uart_speed);
	uart_int_cb_reg(gprs_info.uart_port, gprs_uart_inter_recv);
	ipconfig_get(ipconfig, 50);
	gprs_info.gprs_state = READY_IDLE;

	gprs_mutex = xSemaphoreCreateMutex();

	portBASE_TYPE res = pdTRUE;
	res = xTaskCreate(gprs_task,                   //*< task body
	                  "gprs_task",                  //*< task name
	                  400,                        //*< task heap
	                  NULL,                       //*< tasK handle param
	                  8,   //*< task prio
	                  NULL);                      //*< task pointer
	if (res != pdTRUE)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}

	res = pdTRUE;
	res = xTaskCreate(gprs_maintain,                   //*< task body
	                  "gprs_maintain",                  //*< task name
	                  50,                        //*< task heap
	                  NULL,                       //*< tasK handle param
	                  4,   //*< task prio
	                  NULL);                      //*< task pointer
	if (res != pdTRUE)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}

	gprs_queue = xQueueCreate(2, sizeof(esn_msg_t));
	if (gprs_queue == NULL)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}

	esn_msg_t esn_msg;
	esn_msg.event = GPRS_EVENT;
	xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	return TRUE;
}

static bool_t gprs_deinit()
{
	e_state = E_CLOSE;
	gprs_info.gprs_state = WORK_DOWN;
	return TRUE;
}

bool_t gprs_uart_inter_recv(uint8_t id, uint8_t ch)
{
	recv.buf[recv.offset++] = ch;
	if (gprs_info.gprs_state == WORK_ON)
	{
		esn_msg_t esn_msg;
		esn_msg.event = GPRS_EVENT;
		if (my_strstr((const char*)recv.buf, (const char*)GPRS_CIPSEND[1]) != NULL)
		{
			memset(recv.buf, 0 , SIZE);
			e_state = E_SEND;
			xQueueSendFromISR(gprs_queue, &esn_msg, NULL);
		}
		else if (my_strstr((const char*)recv.buf, (const char*)"SEND OK\r\n") != NULL)
		{
			memset(recv.buf, 0 , SIZE);
			e_state = E_SEND_OK;
			xQueueSendFromISR(gprs_queue, &esn_msg, NULL);
		}
		else if (my_strstr((const char*)recv.buf, (const char*)"ERROR") != NULL)
		{
			memset(recv.buf, 0 , SIZE);
			gprs_info.gprs_state = WORK_DOWN;
			e_state = E_CLOSE;
			xQueueSendFromISR(gprs_queue, &esn_msg, NULL);
		}
		else if (my_strstr((const char*)recv.buf, (const char*)"SEND FAIL") != NULL)
		{
			memset(recv.buf, 0 , SIZE);
			gprs_info.gprs_state = WORK_DOWN;
			e_state = E_CLOSE;
			xQueueSendFromISR(gprs_queue, &esn_msg, NULL);
		}
	}
	return FALSE;
}

const struct gprs gprs_driver =
{
	get,
	set,
	gprs_init,
	gprs_deinit,
	gprs_read_register,
	gprs_write_fifo,
};
