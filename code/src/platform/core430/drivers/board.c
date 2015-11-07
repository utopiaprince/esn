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

void wdt_clear(void)
{
    P6OUT ^= BIT0;
}

void led_set(uint8_t led, bool_t res)
{
    if (led == 0)   //RED
    {
        (res == TRUE) ? (P5OUT |= BIT5) : (P5OUT &= ~BIT5);
    }
    else if (led == 1)
    {
        (res == TRUE) ? (P5OUT |= BIT4) : (P5OUT &= ~BIT4);
    }
}

void board_init(void)
{
    //led
    P5SEL &= ~(BIT4 + BIT5);
    P5DIR |= (BIT4 + BIT5);

    // wdt
    P6SEL &= ~BIT0;
    P6DIR |= BIT0;
}



