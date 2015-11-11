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

void lora_init(void)
{
    lora_gpio_init();
    // uart_init(UART_3, 9600);
}












