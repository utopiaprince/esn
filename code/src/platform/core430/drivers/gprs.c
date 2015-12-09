#include "gprs.h"
#include <prim.h>
#include <osel_arch.h>
#include <hal_timer.h>
#include <uart.h>
#include <drivers.h>
#include <esn_gain.h>
#include <clock.h>
#include "gprs.h"

#define GPRS_EVENT		(0x0100)

#define SEND_SIZE		(200)
#define SIZE			(30)
#define CMD_CB_NUM		(20u)
#define	PULL_UP			(P9OUT |= BIT7)
#define	PULL_DOWN		(P9OUT &= ~BIT7)
#define IS_HIGTH        ((P9OUT & BIT7))
#define GPRS_DETECT_STATUS()	(P10IN & BIT0)

#define AT			("AT\r")                    //*< TEST
#define ATE0		("ATE0\r")                  //*< 关闭回显
#define CSMINS		("AT+CSMINS?\r")            //*< 检查SIM卡
#define CGATT		("AT+CGATT?\r")             //*< 附着网络
#define CIPSTART	("AT+CIPSTART=%s,%s\r")     //*< 连接SOCKET
#define CIPCLOSE	("AT+CIPCLOSE\r")           //*< 关闭SOCKET
#define CIPSEND		("AT+CIPSEND=%d\r")         //*< 发送数据长度
#define CIPSTATUS	("AT+CIPSTATUS\r")			//*< 查询连接状态
#define ATOK		("OK")

static const uint8_t tcp_mode[] = {"\"TCP\""};
static const uint8_t udp_mode[] = {"\"UDP\""};
static uint8_t ipconfig[50];
static uint8_t send_data[SIZE];
static uint8_t GPRS_CIPSEND[][20] = {CIPSEND, "\n>"};

typedef void (*gprs_send_cb)(void);
typedef	void (*gprs_read_cb_t)(const uint8_t *const buf, const uint8_t len);
gprs_read_cb_t gprs_read_cb;
static xQueueHandle gprs_queue = NULL;
static xSemaphoreHandle gprs_mutex = NULL;
static SemaphoreHandle_t guart_Semaphore = NULL; 
static TimerHandle_t gprs_daemon_timer = NULL;

static void gprs_switch(void);
static bool_t gprs_write_fifo(const uint8_t *const payload, const uint16_t len);

typedef struct
{
	uint8_t *cmd;
	uint8_t *result;
	gprs_send_cb cb;
} send_cb_t;

typedef enum
{
	E_IDLE,
    E_OPEN,
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
					(char *)CIPSTART,
					(char *)tcp_mode, (char *)ip_config);
	}
	else
	{
		tfp_sprintf((char *)cmd,
					(char *)CIPSTART,
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
	if (my_strstr((const char*)recv.buf, (const char*)ATOK) != NULL)
		return TRUE;
	else
		return FALSE;
}

static bool_t ate0_deal(void)
{
	if (my_strstr((const char*)recv.buf, (const char*)ATOK) != NULL)
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
	if (my_strstr((const char*)recv.buf, (const char*)"STATE") != NULL)
	    return FALSE;
	else if (my_strstr((const char*)recv.buf, (const char*)"ALREADY CONNECT") != NULL)
		return TRUE;
	else if (my_strstr((const char*)recv.buf, (const char*)ATOK) != NULL)
		return TRUE;
	else
		return FALSE;
}

static bool_t cipstatus(void)
{
	if (my_strstr((const char*)recv.buf, (const char*)"CONNECT OK") != NULL)
		return TRUE;
	else
		return FALSE;
}

#define SOFT_WDT    (5*60*1000llu)
static void cipsend_ok_cb(void)         //3分钟没有喂狗说明GPRS流程出现了问题
{
	xTimerStop(gprs_daemon_timer, 0);
	xSemaphoreGive(gprs_mutex);
    
    wdt_clear(SOFT_WDT);
}

static void switch_rest(void)
{
	esn_msg_t esn_msg;
	esn_msg.event = GPRS_EVENT;
	e_state = E_REST;
	xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
}

static void switch_join(void)
{
	uint8_t join_num = 0;
	bool_t join_mark = FALSE;
	do
	{
		write_fifo(ipconfig, mystrlen((char *)ipconfig));
		vTaskDelay(8000 / portTICK_RATE_MS);
		if (cipstart_deal())
		{
			write_fifo(CIPSTATUS, sizeof(CIPSTATUS)-1);
			vTaskDelay(500 / portTICK_RATE_MS);
			if(cipstatus())	//防止误报连接
				join_mark = TRUE;
			else
				join_mark = FALSE;
			break;
		}
	}
	while (join_num++ < 2);
	if (join_mark == FALSE)
	{
		switch_rest();
		return;
	}
	
	gprs_info.gprs_state = WORK_ON;
	led_set(LEN_GREEN, TRUE);
	
	esn_msg_t esn_msg;
	esn_msg.event = GPRS_CNN_START;
	xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
}

static void gprs_switch(void)
{
	esn_msg_t esn_msg;
	esn_msg.event = GPRS_EVENT;
	if (e_state == E_SEND)
	{
        led_set(LED_RED, FALSE);
		write_fifo(send.buf, send.len);
	}
	else if (e_state == E_SEND_OK)
	{
		cipsend_ok_cb();
	}
    else if(e_state == E_OPEN)
    {
        e_state = E_IDLE;
        if(IS_HIGTH)
        {
            PULL_DOWN;
            vTaskDelay(2000 / portTICK_RATE_MS);
            PULL_UP;
            vTaskDelay(3000 / portTICK_RATE_MS);
        }
        else
        {
           DBG_ASSERT(FALSE __DBG_LINE);    //不可能进入这里
        }
        xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
    }
	else if (e_state == E_CLOSE)
	{
		gprs_info.gprs_state = WORK_DOWN;
		led_set(LEN_GREEN, FALSE);
        led_set(LED_RED, FALSE);
        if(GPRS_DETECT_STATUS())            //判断是否是GPRS开机状态
        {
            write_fifo(CIPCLOSE,  sizeof(CIPCLOSE) - 1);
            vTaskDelay(500 / portTICK_RATE_MS);
            PULL_DOWN;
            vTaskDelay(2000 / portTICK_RATE_MS);
            PULL_UP;
            vTaskDelay(2000 / portTICK_RATE_MS);
        }
        else
        {
            PULL_UP;
            vTaskDelay(5000 / portTICK_RATE_MS);
        }
		e_state = E_OPEN;
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if (e_state == E_IDLE)
	{
        gprs_info.gprs_state = WORK_DOWN;
		static uint8_t idle_num = 0;
		led_set(LEN_GREEN, FALSE);
		if (idle_num++ >= 5)
			DBG_ASSERT(FALSE __DBG_LINE);
		
		if ((GPRS_DETECT_STATUS() != FALSE))
		{
			idle_num = 0;
			uint8_t cgatt_num = 0;
			bool_t cgatt_mark = FALSE;
			
			gprs_info.gprs_state = READY_IDLE;
			write_fifo(AT,  sizeof(AT) - 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			
			if (at_deal())
				write_fifo(ATE0, sizeof(ATE0) - 1);
			else
			{
				switch_rest();
				return;
			}
			vTaskDelay(500 / portTICK_RATE_MS);
			
			if (ate0_deal())
				write_fifo(CSMINS, sizeof(CSMINS) - 1);
			else
			{
				switch_rest();
				return;
			}
			vTaskDelay(500 / portTICK_RATE_MS);
			
			if (csmins_deal())	//检查SIM卡
				write_fifo(CGATT, sizeof(CGATT) - 1);
			else
			{
				gprs_info.gprs_state = SIM_ERROR;
				return;
			}
			vTaskDelay(500 / portTICK_RATE_MS);
			
			while (cgatt_num++ < 20)
			{
				if (cgatt_deal())	//附着网络
				{
					cgatt_mark = TRUE;
					break;
				}
				else
					write_fifo(CGATT, sizeof(CGATT) - 1);
				vTaskDelay(2000 / portTICK_RATE_MS);
			}
			if (cgatt_mark == FALSE)
			{
				switch_rest();
				return;
			}
			switch_join();
		}
		else
		{
			switch_rest();
		}
	}
	else if (e_state == E_REST)
	{
		gprs_info.gprs_state = WORK_DOWN;
		e_state = E_CLOSE;
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
}

static gprs_info_t* get(void)
{
	return &gprs_info;
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
		if(xSemaphoreTake(gprs_mutex, 600) == pdTRUE)
		{
            led_set(LED_RED, TRUE);
			//等待数据发送完成
			xTimerReset(gprs_daemon_timer, 400);
			osel_memset(send_data, 0x00, SIZE);
			tfp_sprintf((char *)send_data, CIPSEND, len);
			
			osel_memset(send.buf, 0x00, SEND_SIZE);
			osel_memcpy(send.buf, payload, len);
			send.len = len;
			write_fifo(send_data, mystrlen((char *)send_data));
			return TRUE;
		}
		
	}
	
	return FALSE;
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

#define HEART_TIME	(1*60*1000llu)
static void gprs_heart(void *p) //定时发送心跳维护TCP连接
{
	esn_msg_t esn_msg;
	esn_msg.event = GPRS_EVENT;
	while (1)
	{
		vTaskDelay(HEART_TIME / portTICK_RATE_MS);
		esn_msg.event = GPRS_HEART_START;
        if(gprs_info.mode == TRUE)
        {
            xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
        }
	}
}

void uart_deal_task(void *p)  
{  
	while(TRUE)
	{
		if( xSemaphoreTake( guart_Semaphore, portMAX_DELAY ) == pdTRUE )  
		{  
			vTaskDelay(300 / portTICK_RATE_MS);
			esn_msg_t esn_msg;
			esn_msg.event = GPRS_EVENT;
			if(gprs_info.gprs_state == WORK_ON)
			{
				if (my_strstr((const char*)recv.buf, (const char*)GPRS_CIPSEND[1]) != NULL)
				{
					memset(recv.buf, 0 , SIZE);
					e_state = E_SEND;
					xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
				}
				else if (my_strstr((const char*)recv.buf, (const char*)"SEND OK\r\n") != NULL)
				{
					memset(recv.buf, 0 , SIZE);
					e_state = E_SEND_OK;
					xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
				}
				else if (my_strstr((const char*)recv.buf, (const char*)"ERROR") != NULL)
				{
					memset(recv.buf, 0 , SIZE);
					gprs_info.gprs_state = GPRS_NET_ERROR;
					e_state = E_CLOSE;
					xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
				}
				else if (my_strstr((const char*)recv.buf, (const char*)"SEND FAIL") != NULL)
				{
					memset(recv.buf, 0 , SIZE);
					gprs_info.gprs_state = GPRS_NET_ERROR;
					e_state = E_CLOSE;
					xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
				}
			}
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
	}
}

static void gprs_sent_timeout_cb(TimerHandle_t timer)
{
	xTimerStop(timer, 0);
	osel_memset(recv.buf, 0 , SIZE);
	recv.offset = 0;
	xSemaphoreGive(gprs_mutex);
}

static bool_t gprs_init()
{
    wdt_clear(SOFT_WDT);
	gprs_info.heart = FALSE;
	port_init();
	
	uart_init(gprs_info.uart_port, gprs_info.uart_speed);
	uart_int_cb_reg(gprs_info.uart_port, gprs_uart_inter_recv);
	ipconfig_get(ipconfig, 50);
	gprs_info.gprs_state = READY_IDLE;
	
	gprs_mutex = xSemaphoreCreateMutex();
	guart_Semaphore = xSemaphoreCreateBinary();  
	
	xTaskCreate(gprs_task, "gprs_task", 300, NULL, tskIDLE_PRIORITY+6, NULL);  
	xTaskCreate(uart_deal_task, "uart_deal_task", 50, NULL, tskIDLE_PRIORITY+7, NULL);  
    if(gprs_info.mode == TRUE)
    {
        xTaskCreate(gprs_heart, "gprs_heart", 50, NULL, tskIDLE_PRIORITY+1, NULL);  
    }
    
	gprs_daemon_timer = xTimerCreate("GprsTimer",
									 (6 * configTICK_RATE_HZ),
									 pdTRUE,
									 NULL,
									 gprs_sent_timeout_cb);
		
	if (gprs_daemon_timer == NULL) {
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
	static BaseType_t xHigherPriorityTaskWoken;  
	recv.buf[recv.offset++] = ch;
	if(gprs_info.gprs_state == WORK_ON)
	{
		if (recv.buf[recv.offset-2] == '\r' && recv.buf[recv.offset-1] == '\n') 
		{  
			xSemaphoreGiveFromISR( guart_Semaphore, &xHigherPriorityTaskWoken );  
		}  
	}
	//	portYIELD_FROM_ISR( xHigherPriorityTaskWoken ); 
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
