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


static uart_interupt_cb_t uart_interrupt_cb[UART_MAX] = {NULL};

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
        /* Initialize uart 1 IO */
        P5SEL |= BIT6 + BIT7;
        P5DIR |= BIT6;      
        P5DIR &= ~BIT7;     
        
        UCA1CTL1 = UCSWRST;
        UCA1CTL0 = UCMODE_0;
        UCA1CTL1 |= UCSSEL_2;                /* SMCLK,8M */
        UCA1BR0 = br0;
        UCA1BR1 = br1;
        UCA1MCTL = UCBRF_0 | (fract << 1);
        UCA1CTL1 &= ~UCSWRST;
        UCA1IE |= UCRXIE;
        break;
        
    case UART_3:        
        P9SEL |= BIT4 + BIT5;
		P9DIR |= BIT4;
		P9DIR &= ~BIT5;
        
		UCA2CTL1 = UCSWRST;
		UCA2CTL0 = UCMODE_0;
		UCA2CTL1 |= UCSSEL_2;				 /* SMCLK,8M */
		UCA2BR0 = br0;
		UCA2BR1 = br1;
		UCA2MCTL = UCBRF_0 | (fract<<1);
		UCA2CTL1 &= ~UCSWRST;
		UCA2IE |= UCRXIE;
		break;
        
    case UART_4:
        /* Initialize uart 1 IO */
        P10SEL |= BIT4 + BIT5;
        P10DIR |= BIT4;      // TXD_TO_485
        P10DIR &= ~BIT5;     // RXD_FROM_485
        
        UCA3CTL1 = UCSWRST;
        UCA3CTL0 = UCMODE_0;
        UCA3CTL1 |= UCSSEL_2;                /* SMCLK,8M */
        UCA3BR0 = br0;
        UCA3BR1 = br1;
        UCA3MCTL = UCBRF_0 | (fract << 1);
        UCA3CTL1 &= ~UCSWRST;
        UCA3IE |= UCRXIE;
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
		UCA1TXBUF = value;
		while (UCA1STAT & UCBUSY);
	}
    else if(id == UART_3)
    {
        UCA2TXBUF = value;
		while (UCA2STAT & UCBUSY);
    }
    else if(id == UART_4)
    {
        UCA3TXBUF = value;
		while (UCA3STAT & UCBUSY);
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
        UCA2IE |= UCRXIE;
    }
    else if (uart_id == UART_4)
    {
        UCA3IE |= UCRXIE;
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
        UCA2IE &= ~UCRXIE;
    }
    else if (uart_id == UART_3)
    {
        UCA3IE &= ~UCRXIE;
    }
}

void uart_int_cb_reg(uint8_t id, uart_interupt_cb_t cb)
{
    if (cb != NULL)
    {
        uart_interrupt_cb[id] = cb;
    }
}

static void uart_int_cb_handle(uint8_t id, uint8_t ch)
{
    BaseType_t xTaskWoken = pdFALSE;

    if (uart_interrupt_cb[id] != NULL)
    {
        xTaskWoken = uart_interrupt_cb[id](id, ch);
    }

    if(xTaskWoken)
    {
        taskYIELD();
    }
}

#pragma vector = USCI_A0_VECTOR
__interrupt void uart0_rx_isr(void)
{
	uart_int_cb_handle(UART_1, UCA0RXBUF);
	LPM3_EXIT;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void uart1_rx_isr(void)
{
	uart_int_cb_handle(UART_2, UCA1RXBUF);
	LPM3_EXIT;
}

#pragma vector = USCI_A2_VECTOR
__interrupt void uart2_rx_isr(void)
{
#if 0
    uart_int_cb_handle(UART_3, UCA2RXBUF);
#endif
	gprs_uart_inter_recv(UCA2RXBUF);
    LPM3_EXIT;
}

#pragma vector = USCI_A3_VECTOR
__interrupt void uart3_rx_isr(void)
{
    uart_int_cb_handle(UART_4, UCA3RXBUF);
    LPM3_EXIT;
}


