/**
 * @brief       : 
 *
 * @file        : uart.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/8/10
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/8/10    v0.0.1      gang.cheng    first version
 */
#include "data_type_def.h"
#include "drivers.h"
#include "osel_arch.h"


static uart_interupt_cb_t uart_interrupt_cb = NULL;

void uart_init(uint8_t uart_id, uint32_t baud_rate)
{
	//*< change SMCLK to ACLK if lpm mode
    fp32_t baud = (fp32_t)SMCLK/baud_rate;
	uint8_t br0 = (uint16_t)baud&0xFF;
	uint8_t br1 = ((uint16_t)baud>>8)&0xFF;		// 37 us
	uint8_t fract = (uint8_t)((baud-(uint16_t)baud) * 8 + 0.5);	// 104us
    
	switch (uart_id)
	{
	case UART_1:
		/* Initialize uart 1 IO */
		P3SEL |= BIT4 + BIT5;
		P3DIR |= BIT4;
		P3DIR &= ~BIT5;
        
		UCA0CTL1 = UCSWRST;
		UCA0CTL0 = UCMODE_0;
		//*< change UCSSEL_2 to UCSSEL__ACLK if lpm mode
        UCA0CTL1 |= UCSSEL_2;				/* SMCLK,8M */

		UCA0BR0 = br0;
		UCA0BR1 = br1;
		UCA0MCTL = UCBRF_0 | (fract<<1);
		UCA0CTL1 &= ~UCSWRST;
		UCA0IE |= UCRXIE;
		break;
        
	case UART_2:
        /* max3221 init */
#if (NODE_TYPE == NODE_TYPE_HOST)
        P10SEL &= ~BIT0;
        P9SEL &= ~BIT7;
        P10DIR |= BIT0;
        P9DIR |= BIT7;
        P10OUT &= ~BIT0;  // Enable EN
        P9OUT |= BIT7;    // Disable FORCEOFF
#endif
        
        P10SEL |= BIT4 + BIT5;
		P10DIR |= BIT4;
		P10DIR &= ~BIT5;
        
		UCA3CTL1 = UCSWRST;
		UCA3CTL0 = UCMODE_0;
		UCA3CTL1 |= UCSSEL_2;				 /* SMCLK,8M */
		UCA3BR0 = br0;
		UCA3BR1 = br1;
		UCA3MCTL = UCBRF_0 | (fract<<1);
		UCA3CTL1 &= ~UCSWRST;
		UCA3IE |= UCRXIE;
		break;
        
    case UART_3:
        /* Initialize uart 1 IO */
        P5SEL |= BIT6 + BIT7;
        P5DIR |= BIT6;      // TXD_TO_485
        P5DIR &= ~BIT7;     // RXD_FROM_485
        
        P10SEL &= ~(BIT6 + BIT7);
        P10DIR |= BIT6 + BIT7;
        P10OUT &= ~(BIT6+BIT7);
        
        UCA1CTL1 = UCSWRST;
        UCA1CTL0 = UCMODE_0;
        UCA1CTL1 |= UCSSEL_2;                /* SMCLK,8M */
        UCA1BR0 = br0;
        UCA1BR1 = br1;
        UCA1MCTL = UCBRF_0 | (fract << 1);
        UCA1CTL1 &= ~UCSWRST;
        UCA1IE |= UCRXIE;
        break;
	default:
		break;
	}
}


void uart_send_char(uint8_t id, uint8_t value)
{
    if (id == UART_1)
	{
        UCA0TXBUF = value;
		while (UCA0STAT & UCBUSY);
	}
	else if (id == UART_2)
	{
		UCA3TXBUF = value;
		while (UCA3STAT & UCBUSY);
	}
    else if(id == UART_3)
    {
        P10OUT |= (BIT6+BIT7);
        UCA1TXBUF = value;
		while (UCA1STAT & UCBUSY);
        P10OUT &= ~(BIT6+BIT7);
    }
}


void uart_send_string(uint8_t id, uint8_t *string, uint16_t length)
{
    uint16_t i = 0;
    for (i = 0; i < length; i++)
    {
        uart_send_char(id, string[i]);
    }
}

void uart_recv_enable(uint8_t uart_id)
{
    if (uart_id == UART_1)
    {
        UCA0IE |= UCRXIE;
    }
    else if (uart_id == UART_2)
    {
        UCA1IE |= UCRXIE;
    }
    else if (uart_id == UART_3)
    {
        UCA1IE |= UCRXIE;
    }
}

void uart_recv_disable(uint8_t uart_id)
{
    if (uart_id == UART_1)
    {
        UCA0IE &= ~UCRXIE;
    }
    else if (uart_id == UART_2)
    {
        UCA1IE &= ~UCRXIE;
    }
    else if (uart_id == UART_3)
    {
        UCA1IE &= ~UCRXIE;
    }
}

void uart_int_cb_reg(uart_interupt_cb_t cb)
{
    if (cb != NULL)
    {
        uart_interrupt_cb = cb;
    }
}

static void uart_int_cb_handle(uint8_t id, uint8_t ch)
{
    if (uart_interrupt_cb != NULL)
    {
        uart_interrupt_cb(id, ch);
    }
}

#pragma vector = USCI_A0_VECTOR
__interrupt void uart0_rx_isr(void)
{
	uart_int_cb_handle(UART_1, UCA0RXBUF);
	LPM3_EXIT;
}

#pragma vector = USCI_A3_VECTOR
__interrupt void uart3_rx_isr(void)
{
	uart_int_cb_handle(UART_2, UCA3RXBUF);
	LPM3_EXIT;
}


// 485
#pragma vector = USCI_A1_VECTOR
__interrupt void uart1_rx_isr(void)
{
    uart_int_cb_handle(UART_3, UCA1RXBUF);
    LPM3_EXIT;
}




