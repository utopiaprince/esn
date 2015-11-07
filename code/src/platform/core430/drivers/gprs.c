#include "gprs.h"
#include <prim.h>
#include <osel_arch.h>
#include <hal_timer.h>
#include <uart.h>

#define GPRS_EVENT		(0x0100)

#define SIZE			(128u)
#define CMD_CB_NUM		(20u)
#define	PULL_UP			(P9OUT &= ~BIT7)
#define	PULL_DOWN		(P9OUT |= BIT7)
#define GPRS_DETECT_STATUS()	(P10IN & BIT0)

#define AT_TEST		("AT+CREG?\r\0")
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
static uint8_t GPRS_AT[][6] = {AT,"OK\r\n"};
static uint8_t GPRS_ATE0[][6] = {ATE0,"OK\r\n"};
static uint8_t GPRS_CSMINS[][20] = {CSMINS,"OK\r\n"};
static uint8_t GPRS_CGATT[][20] = {CGATT,"OK\r\n"};
static uint8_t GPRS_CIPSTART[][20] = {CIPSTART,"CONNECT OK\r\n"};
static uint8_t GPRS_CIPCLOSE[][20] = {CIPCLOSE,"OK\r\n"};
static uint8_t GPRS_CIPSEND[][20] = {CIPSEND,">"};
static uint8_t GPRS_TEST[][20] = {AT_TEST,"OK\r\n"};

typedef void (*gprs_send_cb)(void);
typedef	void (*gprs_read_cb_t)(const uint8_t *const buf, const uint8_t len);
gprs_read_cb_t gprs_read_cb;
static xQueueHandle gprs_queue = NULL;

static void gprs_switch(void);

typedef struct
{
	uint8_t *cmd;
	uint8_t *result;
	gprs_send_cb cb;
}send_cb_t;

typedef enum
{
	E_IDLE,
	E_IDLE_REST,
	E_CNN_REST,
	E_CNN_SEND,
	E_UP,
	E_DOWN,
	E_CLOSE,
}E_SATE_E;

typedef struct
{
	uint8_t buf[SIZE];
	uint8_t len;
	GPRS_STATE_E state;
}send_t;

typedef struct
{
	uint8_t buf[SIZE];
	uint8_t len;
	uint8_t offset;
}recv_t;

static gprs_info_t gprs_info;
static volatile E_SATE_E e_state = E_CLOSE;	
static hal_timer_t *gprs_switch_timer = NULL;
static send_t send;
static recv_t recv;
static send_cb_t send_cb_group[CMD_CB_NUM];
static volatile uint8_t *send_cmd_result;
static volatile uint8_t send_cmd_index = 0;


static bool_t write_fifo(const uint8_t *const payload, const uint8_t len);
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
	
	for(int i = 0; str[i] != '\0'; i++)  
	{  
		int tem = i;    
		int j = 0;  
		
		while(str[i++] == sub_str[j++])  
		{  
			if(sub_str[j] == '\0')  
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
		if(len++ >127)
			DBG_ASSERT(FALSE __DBG_LINE);
	}
	return len;
}

static uint8_t *ustrchr(uint8_t *s, const uint8_t *const c)
{
	DBG_ASSERT(s != NULL __DBG_LINE);
	DBG_ASSERT(c != NULL __DBG_LINE);
	while(*s!='\0')
	{
		s++;
		if (*s==*c)
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
				gprs_info.dip[0],gprs_info.dip[1],gprs_info.dip[2],gprs_info.dip[3], gprs_info.port);
	if(gprs_info.mode)
	{
		tfp_sprintf((char *)cmd, 
					(char *)GPRS_CIPSTART[0], 
					(char *)tcp_mode,(char *)ip_config);
	}
	else
	{
		tfp_sprintf((char *)cmd, 
					(char *)GPRS_CIPSTART[0], 
					(char *)udp_mode,(char *)ip_config);
	}
}

static bool_t write_fifo(const uint8_t *const payload, const uint8_t len)
{
	DBG_ASSERT(payload != NULL __DBG_LINE);
	if(len > SIZE)
	{
		DBG_ASSERT(FALSE __DBG_LINE);
	}
	int  i =0;
	if((*payload)=='A' && (*(payload+1))=='T')
	{
		for( i=0; i<CMD_CB_NUM; i++)
		{
			if(send_cb_group[i].cmd != NULL)
			{
				if(osel_memcmp(send_cb_group[i].cmd, payload, len))
				{
					send_cmd_result = send_cb_group[i].result;
					send_cmd_index = i;
					break;
				}
			}
		}
		if(i > CMD_CB_NUM)
		{
			return FALSE;
		}
		osel_memset(send.buf, 0 , SIZE);
		osel_memcpy(send.buf,payload,len);
		send.len = len;
	}
	osel_memset(recv.buf, 0 , SIZE);
	recv.offset = 0;
	uart_send_string(gprs_info.uart_port, send.buf, send.len);
	return TRUE;
}

static void at_cb(void)
{
	write_fifo(GPRS_ATE0[0], sizeof(ATE0) - 1);
}

static void ate0_cb(void)
{
	write_fifo(GPRS_CSMINS[0], sizeof(CSMINS) - 1);
}

static void csmins_cb(void)
{
	uint8_t* ptr = ustrchr(recv.buf, ",");  
	if(ptr != NULL)
	{
		ptr++;
		if(*ptr == 0x31)
		{
			write_fifo(GPRS_CGATT[0], sizeof(CGATT) - 1);
			return;
		}
	}
	gprs_info.gprs_state = SIM_ERROR;
}

static void cgatt_cb(void)
{
	uint8_t* ptr = ustrchr(recv.buf, ":");  
	static uint8_t rest_num = 0;
	if(ptr != NULL)
	{
		ptr+=2;
		if(*ptr == 0x31)
		{
			rest_num = 0;
			e_state = E_CNN_REST;
			gprs_switch();
			write_fifo(ipconfig, mystrlen((char *)ipconfig));
			return;
		}
		else
		{
			if(rest_num++ < 20)
			{
				e_state = E_IDLE_REST;
				gprs_init();
			}
			return;
		}
	}
	gprs_info.gprs_state = GPRS_NET_ERROR;
}

static void cipstart_cb(void)
{
	gprs_info.gprs_state = WORK_ON;
}

static void cipclose_cb(void)
{
	gprs_info.gprs_state = WORK_DOWN;
}

static void cipsend_ok_cb(void)
{
	_NOP();
}

static void cipsend_cb(void)
{
	send_cmd_result = "SEND OK\r\n";
	send_cmd_index = CMD_CB_NUM-1;
	send_cb_group[CMD_CB_NUM-1].cb = cipsend_ok_cb;
	write_fifo(send.buf,send.len);
}
/**
*实现cmd回调函数注册
*/
static void cmd_cb_register(void)
{
	uint8_t index = 0;
	send_cb_group[index].cmd = GPRS_AT[0];
	send_cb_group[index].result = GPRS_AT[1];
	send_cb_group[index++].cb = at_cb;
	
	send_cb_group[index].cmd = GPRS_ATE0[0];
	send_cb_group[index].result = GPRS_ATE0[1];
	send_cb_group[index++].cb = ate0_cb;
	
	send_cb_group[index].cmd = GPRS_CSMINS[0];
	send_cb_group[index].result = GPRS_CSMINS[1];
	send_cb_group[index++].cb = csmins_cb;
	
	send_cb_group[index].cmd = GPRS_CGATT[0];
	send_cb_group[index].result = GPRS_CGATT[1];
	send_cb_group[index++].cb = cgatt_cb;
	
	
	ipconfig_get(ipconfig,50);
	send_cb_group[index].cmd = ipconfig;
	send_cb_group[index].result = GPRS_CIPSTART[1];
	send_cb_group[index++].cb = cipstart_cb;
	
	send_cb_group[index].cmd = GPRS_CIPCLOSE[0];
	send_cb_group[index].result = GPRS_CIPCLOSE[1];
	send_cb_group[index++].cb = cipclose_cb;
	
	send_cb_group[index].cmd = GPRS_TEST[0];
	send_cb_group[index].result = GPRS_TEST[1];
	send_cb_group[index++].cb = at_cb;
}

static void gprs_switch(void)
{
	esn_msg_t esn_msg;
	esn_msg.event = GPRS_EVENT;
	if(e_state == E_IDLE)
	{
		if((GPRS_DETECT_STATUS() != FALSE))
		{
			gprs_info.gprs_state = READY_IDLE;
			//write_fifo(GPRS_AT[0],  sizeof(AT) - 1);
			write_fifo(GPRS_TEST[0], sizeof(AT_TEST) - 1);
		}
		else
		{
			gprs_info.gprs_state = E_CLOSE;
			e_state = E_CLOSE;
			xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
		}
	}
	else if(e_state == E_CNN_SEND)
	{
		if(gprs_info.gprs_state == WORK_ON)
		{
			e_state = E_CLOSE;
		}
		else
		{
			e_state = E_CNN_REST;
			write_fifo(ipconfig, mystrlen((char *)ipconfig));
			xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
		}
	}
	else if(e_state == E_CNN_REST)
	{
		e_state = E_CNN_SEND;
		gprs_info.gprs_state = WORK_DOWN;
		vTaskDelay(11000 / portTICK_RATE_MS);
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if(e_state == E_IDLE_REST)
	{
		e_state = E_IDLE;
		vTaskDelay(2000 / portTICK_RATE_MS);
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if(e_state == E_UP)
	{
		PULL_UP;
		e_state = E_DOWN;
		vTaskDelay(2000 / portTICK_RATE_MS);
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if(e_state == E_DOWN)
	{
		PULL_DOWN;
		e_state = E_IDLE;
		vTaskDelay(5000 / portTICK_RATE_MS);
		xQueueSend(gprs_queue, &esn_msg, portMAX_DELAY);
	}
	else if(e_state == E_CLOSE)
	{
		send_cmd_result = NULL;
		e_state = E_UP;
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
	if(cb != NULL)
		gprs_read_cb = (gprs_read_cb_t)cb;
}

static bool_t gprs_write_fifo(const uint8_t *const payload, const uint8_t len)
{
	DBG_ASSERT(payload != NULL __DBG_LINE);
	if(gprs_info.gprs_state == WORK_ON)
	{
		osel_memset(send_data, 0x00,SIZE);
		tfp_sprintf((char *)send_data, CIPSEND, len);
		
		send_cb_group[CMD_CB_NUM-1].cmd = send_data;
		send_cb_group[CMD_CB_NUM-1].result = GPRS_CIPSEND[1];
		send_cb_group[CMD_CB_NUM-1].cb = cipsend_cb;
		
		write_fifo(send_data,mystrlen((char *)send_data));
		
		osel_memset(send.buf, 0x00,SIZE);
		osel_memcpy(send.buf, payload, len);
		send.len = len;
	}
	return TRUE;
}

static void gprs_task(void *p)
{
	esn_msg_t esn_msg;
	while (1)
	{
		xQueueReceive(gprs_queue,        //*< the handle of received queue
					  &esn_msg,          //*< pointer to data received
					  portMAX_DELAY);   //*< time out
		
		switch (esn_msg.event)
		{
		case GPRS_EVENT:
			gprs_switch();
			break;
		default:
			break;
		}
	}
}

static void port_init(void)
{
	P9SEL &= ~BIT7;
	P9DIR |=  BIT7;
	
	P10SEL &= ~BIT0;
	P10DIR &= ~BIT0;
//	P10DIR |= BIT0;
//	P10OUT |= BIT0;
//	P6SEL &=~BIT7;//Close power
//	P6DIR |= BIT7;
//	P6OUT &= ~BIT7;
//	
//	P6SEL &=~BIT7;//EN_6130 power
//	P6DIR |= BIT7;
//	P6OUT |= BIT7;
//	P10DIR |= BIT0;
//	P10OUT |= BIT0; 
}

static bool_t gprs_init()
{
	port_init();
	uart_init(gprs_info.uart_port, gprs_info.uart_speed);
	cmd_cb_register();
	gprs_info.gprs_state = READY_IDLE;
	
	portBASE_TYPE res = pdTRUE;
	res = xTaskCreate(gprs_task,                   //*< task body
					  "gprs_task",                  //*< task name
					  200,                        //*< task heap
					  NULL,                       //*< tasK handle param
					  configMAX_PRIORITIES - 3,   //*< task prio
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

void gprs_uart_inter_recv(uint8_t ch)
{
	recv.buf[recv.offset++] = ch;
	if(send_cmd_result == NULL)
	{
		return;
	}
	if(my_strstr((const char*)recv.buf, (const char*)send_cmd_result) != NULL)
	{
		send_cb_group[send_cmd_index].cb();
	}
	if(gprs_info.gprs_state == WORK_ON)
	{
		if(my_strstr((const char*)recv.buf, (const char*)"ERROR\r\n") != NULL)
		{
			e_state = E_CNN_REST;
			gprs_switch();
			write_fifo(ipconfig, mystrlen((char *)ipconfig));
		}
		else if(my_strstr((const char*)recv.buf, (const char*)"SEND FAIL\r\n") != NULL)
		{
			e_state = E_CNN_REST;
			gprs_switch();
			write_fifo(ipconfig, mystrlen((char *)ipconfig));
		}
	}
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
