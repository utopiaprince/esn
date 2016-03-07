/**
 * @brief       : this
 * @file        : lora.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-10-01
 * change logs  :
 * Date       Version     Author        Note
 * 2015-10-01  v0.0.1  gang.cheng    first version
 */
/**
 * @note
 * 硬件：单片机为MSP430F5438
 *      单片机的UCA3(P10.5/4)：对应UM402的RS232串口（TTL电平）
 *      单片机的P10.7：对应UM402的SETA
 *      单片机的P10.6：对应UM402的AUX
 *      单片机的P10.3：对应UM402的SETB
 *                MSP430F5438A                    UM402
 *            --------------------          -------------------
 *            |                  |          |                 |
 *            |     P10.0/GPIO   |--------->|SETA             |
 *            |     P9.4/UCA2TXD |--------->|RXD              |
 *            |     P9.5/UCA2RXD |<---------|TXD              |
 *            |     P9.6/GPIO    |<---------|AUX              |
 *            |     P9.7/GPIO    |--------->|SETB             |
 *            |                  |          |                 |
 *            --------------------          -------------------
 */

#include <lib.h>
#include <drivers.h>

#define SETA_H      (P10OUT |= BIT7)
#define SETA_L      (P10OUT &= ~BIT7)

#define SETB_H      (P10OUT |= BIT3)
#define SETB_L      (P10OUT &= ~BIT3)

#define MODE_1()    lora_mode=NORMAL_MODE;SETA_L;SETB_L;    //*< normal mode
#define MODE_2()    lora_mode=WAKEUP_MODE;SETA_L;SETB_H;    //*< wakeup mode
#define MODE_3()    lora_mode=LOWPOW_MODE;SETA_H;SETB_L;    //*< low power mode
#define MODE_4()    lora_mode=SETTING_MODE;SETA_H;SETB_H    //*< set mode

static uint8_t lora_port = 0;

static uint8_t lora_recv_data[150] = {0};
static uint8_t lora_recv_index = 0;
static bool_t lora_recv_rxok_flag = FALSE;


static uint8_t lora_sent_data[150] = {0};

static uint8_t lora_mode = NORMAL_MODE;
//static uint8_t lora_reg = UM_REG_TYPE;

static TimerHandle_t lora_daemon_timer = NULL;

static lora_int_reg_t lora_int_reg[2];  //*< rxok, txok.

static void lora_gpio_init(void)
{
    // MSP430的GPIO初始化
    // P10.0 -- SETA (Output)
    // P9.6 -- AUX (Input)
    // P9.7 -- SETB (Output)
    P10DIR &= ~BIT6;

    P10DIR |= BIT7; P10OUT |= BIT7;
    P10DIR |= BIT3; P10OUT |= BIT3; //*< set SETA,SETB high

    // 使UM402进入配置模式
}

/**
 * @biref the cmd of read the config
 * @param reg  start of read config
 * @param len  how many bytes read
 */
static bool_t lora_reg_read(uint8_t reg, uint8_t len)
{
    uint8_t buf[15];
    uint8_t index = 0;

    lora_mode = SETTING_MODE;
//    lora_reg = reg;

    buf[index++] = 0xFF;
    buf[index++] = 0x56;
    buf[index++] = 0xAE;
    buf[index++] = 0x35;
    buf[index++] = 0xA9;
    buf[index++] = 0x55;
    buf[index++] = 0xf0;
    buf[index++] = reg;
    buf[index++] = len;

    lora_recv_rxok_flag = FALSE;
    uart_send_string(lora_port, buf, index);

    vTaskDelay(30 / portTICK_PERIOD_MS);
    
    if (lora_recv_index!=0)
    {
        if ((lora_recv_index == (len + 1)) &&
                (lora_recv_data[0] == 0x24))
        {
            lora_recv_index = 0;
            osel_memset(lora_recv_data, 0x00, 150);
            return TRUE;
        }
    }

    lora_recv_index = 0;
    osel_memset(lora_recv_data, 0x00, 150);
    return FALSE;
}

static bool_t lora_reg_write(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t send_buf[15];
    uint8_t index = 0;

    lora_mode = SETTING_MODE;
//    lora_reg = reg;

    send_buf[index++] = 0xFF;
    send_buf[index++] = 0x56;
    send_buf[index++] = 0xAE;
    send_buf[index++] = 0x35;
    send_buf[index++] = 0xA9;
    send_buf[index++] = 0x55;
    send_buf[index++] = 0x90;
    send_buf[index++] = reg;
    send_buf[index++] = len;
    osel_memcpy(&send_buf[index], buf, len);
    index += len;

    lora_recv_rxok_flag = FALSE;
    uart_send_string(lora_port, send_buf, index);

    vTaskDelay(2);

    if (lora_recv_index!=0)
    {
        if ((lora_recv_index == (len + 1)) &&
                (lora_recv_data[0] == 0x24))
        {
            lora_recv_index = 0;
            osel_memset(lora_recv_data, 0x00, 150);
            return TRUE;
        }
    }

    lora_recv_index = 0;
    osel_memset(lora_recv_data, 0x00, 150);
    return FALSE;
}

void lora_data_write(uint8_t *buf, uint8_t len)
{
    portENTER_CRITICAL();
    uart_send_string(lora_port, buf, len);
    portEXIT_CRITICAL();
}

uint8_t lora_data_read(uint8_t *buf, uint8_t len)
{
    uint8_t read_len = 0;
    if(len > lora_recv_index)
    {
        read_len = lora_recv_index;
    }
    else
    {
        read_len = len;
    }
    
    osel_memcpy(buf, lora_recv_data, read_len);
    
    osel_memset(lora_recv_data, 0x00, read_len);
    lora_recv_index = 0;
    return read_len;
}

void lora_data_sent(void)
{
    if (lora_int_reg[0] != NULL)
    {
        (*(lora_int_reg[0]))(0x0000);
    }
}

uint8_t lora_data_len_get(void)
{
    return lora_recv_index;
}


static void lora_recv_timeout_cb(TimerHandle_t timer)
{
    xTimerStop(lora_daemon_timer, 0);
    //*< rxok event
    lora_recv_rxok_flag = TRUE;

    if (lora_mode != SETTING_MODE)
    {
        if (lora_int_reg[1] != NULL)
        {
            (*(lora_int_reg[1]))(0x0000);    //*< rxok
        }
    }
}

static bool_t lora_recv_ch_cb(uint8_t id, uint8_t ch)
{
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    lora_recv_data[lora_recv_index++] = ch;
//    xTimerResetFromISR(lora_daemon_timer, &xHigherPriorityTaskWoken);

    return FALSE;
}

/**
 * @brief
 */
void lora_init(uint8_t uart_id, uint32_t baud)
{
    lora_port = uart_id;

    lora_gpio_init();
    uart_init(lora_port, baud);
    uart_int_cb_reg(lora_port, lora_recv_ch_cb);

    lora_daemon_timer = xTimerCreate("LoraTimer",
                                     2,
                                     pdTRUE,
                                     NULL,
                                     lora_recv_timeout_cb);
}

static uint8_t char_buf[];

/**
 * @note call this func after os is running
 */
void lora_setting(lora_int_reg_t txok_cb,
                  lora_int_reg_t rxok_cb)
{
    MODE_4();

    vTaskDelay(20 / portTICK_PERIOD_MS);
    lora_reg_read(UM_REG_TYPE, 10);
    
    uint8_t rf_bps = 0x0e;
    lora_reg_write(UM_REG_RF_BPS, &rf_bps, 1);

    uint8_t rf_req[] = {0x07, 0x2B, 0xF0};
    lora_reg_write(UM_REG_FREQ_3, rf_req, 3);

    uint8_t rf_pow = 0x16;
    lora_reg_write(UM_REG_POWER, &rf_pow, 1);

    uint8_t rf_laddr[] = {0xFF, 0xFF};
    lora_reg_write(UM_REG_HEAD_H, rf_laddr, 2);

    uint8_t rf_feat = 0x0A;
    lora_reg_write(UM_REG_FEATURE, &rf_feat, 1);

//    uint8_t rf_baud = 0x07; //*< 115200
//    lora_reg_write(UM_REG_UART_BPS, &rf_baud, 1);
//    uart_init(lora_port, 115200);

    lora_int_reg[0] = txok_cb;
    lora_int_reg[1] = rxok_cb;

    MODE_1();
}












