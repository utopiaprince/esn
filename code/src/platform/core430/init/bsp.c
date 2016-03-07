#include "lib.h"
#include "drivers.h"
#include "bsp.h"
#include "FreeRTOS.h"
#include "task.h"


void bsp_init(void)
{
    clk_init(SYSCLK_8MHZ);
    board_init();
}

/* The MSP430X port uses this callback function to configure its tick interrupt.
This allows the application to choose the tick interrupt source.
configTICK_VECTOR must also be set in FreeRTOSConfig.h to the correct
interrupt vector for the chosen tick interrupt source.  This implementation of
vApplicationSetupTimerInterrupt() generates the tick from timer A0, so in this
case configTICK_VECTOR is set to TIMER1_A0_VECTOR. */
void vApplicationSetupTimerInterrupt( void )
{
    const unsigned short usACLK_Frequency_Hz = 32000;
    //@todo: start timer ao vector
    /* Ensure the timer is stopped. */
	TA1CTL = 0;

	/* Run the timer from the ACLK. */
	TA1CTL = TASSEL_1;

	/* Clear everything to start with. */
	TA1CTL |= TACLR;

	/* Set the compare match value according to the tick rate we want. */
	TA1CCR0 = usACLK_Frequency_Hz / configTICK_RATE_HZ;

	/* Enable the interrupts. */
	TA1CCTL0 = CCIE;

	/* Start up clean. */
	TA1CTL |= TACLR;

	/* Up mode. */
	TA1CTL |= MC_1;
}



void vApplicationIdleHook( void )
{
	/* Called on each iteration of the idle task.  In this case the idle task
	just enters a low power mode. */
//	__bis_SR_register( LPM3_bits + GIE );
    
    static uint32_t idle_cnt = 0;
    if(idle_cnt ++ >= 10000)
    {
        idle_cnt = 0;
        extern void hard_wdt_clear();
        hard_wdt_clear();
    }
    
    __bis_SR_register( GIE );
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues or
	semaphores. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}