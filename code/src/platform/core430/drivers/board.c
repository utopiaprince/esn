/**
 * @brief       : this
 * @file        : board.c
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-05-05
 * change logs  :
 * Date       Version     Author        Note
 * 2015-05-05  v0.0.1  gang.cheng    first version
 */
#include "osel_arch.h"
#include "lib.h"
#include "drivers.h"

void hard_wdt_clear(void)
{
    P6OUT ^= BIT0;
}

void sys_wdt_clear(void)
{
    WDTCTL = WDTPW + WDTCNTCL + WDTSSEL0 + WDTIS0 + WDTIS1;
}

void led_set(uint8_t led, bool_t res)
{
    if (led == LED_RED)   //RED
    {
        (res == TRUE) ? (P5OUT |= BIT5) : (P5OUT &= ~BIT5);
    }
    else if (led == LEN_GREEN)
    {
        (res == TRUE) ? (P5OUT |= BIT4) : (P5OUT &= ~BIT4);
    }
}

void mac_addr_get(uint8_t *mac_addr)
{
    #define ID_MAX  (4u)
#ifdef NODE_TYPE_GATEWAY
	uint8_t id[ID_MAX] = {0xaa,0xbb,0xcc,0xdd};
#else
	uint8_t id[ID_MAX] = {0x11,0x22,0x33,0x44};
#endif
	
	osel_memcpy(mac_addr, id, ID_MAX);
}

void board_init(void)
{
    //led
    P5SEL &= ~(BIT4 + BIT5);
    P5DIR |= (BIT4 + BIT5);

    // wdt
    P6SEL &= ~BIT0;
    P6DIR |= BIT0;
	
	led_set(LED_RED, FALSE);
}



