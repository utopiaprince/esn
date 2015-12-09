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

#define SETA_H      (P10OUT |= BIT0)
#define SETA_L      (P10OUT &= ~BIT0)

#define SETB_H      (P9OUT |= BIT7)
#define SETB_L      (P9OUT &= ~BIT7)

#define MODE_1()    (lora_mode=NORMAL_MODE;SETA_L;SETB_L;)    //*< normal mode
#define MODE_2()    (lora_mode=WAKEUP_MODE;SETA_L;SETB_H;)    //*< wakeup mode
#define MODE_3()    (lora_mode=LOWPOW_MODE;SETA_H;SETB_L;)    //*< low power mode
#define MODE_4()    (lora_mode=SETTING_MODE;SETA_H;SETB_H;)    //*< set mode

static uint8_t lora_port = 0;

static uint8_t lora_recv_data[150] = {0};
static uint8_t lora_recv_index = 0;

static uint8_t lora_sent_data[150] = {0};

static uint8_t lora_mode = NORMAL_MODE;
static uint8_t lora_reg = UM_REG_TYPE;


static void lora_gpio_init(void)
{
    // MSP430的GPIO初始化
    // P10.0 -- SETA (Output)
    // P9.6 -- AUX (Input)
    // P9.7 -- SETB (Output)
    P9DIR &= ~BIT6;

    P10DIR |= BIT0; P10OUT |= BIT0;
    P9DIR  |= BIT7; P9OUT  |= BIT7; //*< set SETA,SETB high

    // 使UM402进入配置模式
}

/**
 * @biref the cmd of read the config
 * @param reg  start of read config
 * @param len  how many bytes read
 */
static void lora_reg_read(uint8_t reg, uint8_t len)
{
    uint8_t buf[12];
    uint8_t index = 0;

    lora_mode = SETTING_MODE;
    lora_reg = reg;

    buf[index++] = 0xFF;
    buf[index++] = 0x56;
    buf[index++] = 0xAE;
    buf[index++] = 0x35;
    buf[index++] = 0xA9;
    buf[index++] = 0x55;
    buf[index++] = 0xf0;
    buf[index++] = reg;
    buf[index++] = len;

    uart_send_string(lora_port, buf, index);
}

static void lora_reg_write(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t buf[12];
    uint8_t index = 0;

    lora_mode = SETTING_MODE;
    lora_reg = reg;

    buf[index++] = 0xFF;
    buf[index++] = 0x56;
    buf[index++] = 0xAE;
    buf[index++] = 0x35;
    buf[index++] = 0xA9;
    buf[index++] = 0x55;
    buf[index++] = 0x90;
    buf[index++] = reg;
    buf[index++] = len;
    osel_memcpy(&buf[index], buf, len);
    index += len;

    uart_send_string(lora_port, buf, index);
}



static bool_t lora_recv_ch_cb(uint8_t id, uint8_t ch)
{
    if (lora_mode == SETTING_MODE)
    {
        lora_recv_data[]
    }
    else
    {

    }
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
}

void lora_setting(void)
{
    MODE_4();


}












