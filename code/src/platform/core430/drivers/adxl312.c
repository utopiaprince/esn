/**
 * @brief       :
 *
 * @file        : adxl345.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/11/5
 *
 *                MSP430F5438A                    UM402
 *            --------------------          -------------------
 *            |                  |          |                 |
 *            |     P3.0/GPIO    |--------->|ADCS             |
 *            |     P3.1/UCB0SDA |--------->|ADSDI            |
 *            |     P3.2/UCB0SCL |--------->|ADSDO            |
 *            |     P3.3/UCB0CLK |--------->|ADSCL            |
 *            |     P2.7/UCA2RXD |<---------|INT2             |
 *            |     P2.6/GPIO    |<---------|INT1             |
 *            |                  |          |                 |
 *            --------------------          -------------------
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/11/5    v0.0.1      gang.cheng    first version
 */

#include <osel_arch.h>
#include <lib.h>
#include <math.h>
#include <drivers.h>
#include <esn_gain.h>

#define ADXL_CS_EN()            (P3OUT &= ~BIT0)
#define ADXL_CS_DIS()           (P3OUT |= BIT0)
static TimerHandle_t adxl312_daemon_timer = NULL;

bool_t adxl312_reg_read(uint8_t addr, uint8_t *pvalue)
{
    DBG_ASSERT(pvalue != NULL __DBG_LINE);
    ADXL_CS_EN();
    addr |= RW_BIT;
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX buffer ready
    UCB0TXBUF = addr;                       // Send the header byte
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX complete
    while (!(UCB0IFG & UCRXIFG));           // Wait for USCI_B0 RX buffer ready
    UCB0IFG &= ~UCRXIFG;
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX buffer ready
    UCB0TXBUF = 0xFF;                       // Send the header byte
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX complete
    while (!(UCB0IFG & UCRXIFG));           // Wait for USCI_B0 RX buffer ready
    *pvalue = UCB0RXBUF;
    ADXL_CS_DIS();

    return TRUE;
}

bool_t adxl312_reg_write(uint8_t addr, uint8_t value)
{
    ADXL_CS_EN();

    addr &= ~RW_BIT;
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX buffer ready
    UCB0TXBUF = addr;                       // Send the header byte
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX complete
    while (!(UCB0IFG & UCRXIFG));           // Wait for USCI_B0 RX buffer ready
    UCB0IFG &= ~UCRXIFG;
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX buffer ready
    UCB0TXBUF = value;                      // Send the header byte
    while (!(UCB0IFG & UCTXIFG));           // Wait for USCI_B0 TX complete
    while (!(UCB0IFG & UCRXIFG));           // Wait for USCI_B0 RX buffer ready
    UCB0IFG &= ~UCRXIFG;

    ADXL_CS_DIS();
    return TRUE;
}

bool_t adxl312_read_fifo(uint8_t addr, uint8_t *datap, uint8_t len)
{
    ADXL_CS_EN();

    addr |= (RW_BIT + MB_BIT);
    while (!(UCB0IFG & UCTXIFG));     // Wait for USCI_B0 TX buffer ready
    UCB0TXBUF = addr;           // Send the header byte
    while (!(UCB0IFG & UCTXIFG));     // Wait for USCI_B0 TX complete
    while (!(UCB0IFG & UCRXIFG));     // Wait for USCI_B0 RX buffer ready
    UCB0IFG &= ~UCRXIFG;

    for (uint8_t i = 0; i < len; i++)
    {
        while (!(UCB0IFG & UCTXIFG));     // Wait for USCI_B0 TX buffer ready
        UCB0TXBUF = 0xFF;                 // Send the header byte
        while (!(UCB0IFG & UCTXIFG));     // Wait for USCI_B0 TX complete
        while (!(UCB0IFG & UCRXIFG));     // Wait for USCI_B0 RX buffer ready
        datap[i] = UCB0RXBUF;
    }

    ADXL_CS_DIS();

    return TRUE;
}

bool_t adxl312_write_fifo(uint8_t addr, uint8_t *datap, uint8_t len)
{
    ADXL_CS_EN();

    addr &= ~RW_BIT;
    addr |= MB_BIT;
    while (!(UCB0IFG & UCTXIFG));       // Wait for USCI_B0 TX buffer ready
    UCB0TXBUF = addr;                   // Send the header byte
    while (!(UCB0IFG & UCTXIFG));       // Wait for USCI_B0 TX complete
    while (!(UCB0IFG & UCRXIFG));       // Wait for USCI_B0 RX buffer ready
    UCB0IFG &= ~UCRXIFG;
    for (uint8_t i = 0; i < len; i++)
    {
        while (!(UCB0IFG & UCTXIFG));   // Wait for USCI_B0 TX buffer ready
        UCB0TXBUF = datap[i];           // Send the header byte
        while (!(UCB0IFG & UCTXIFG));   // Wait for USCI_B0 TX complete
        while (!(UCB0IFG & UCRXIFG));   // Wait for USCI_B0 RX buffer ready
        UCB0IFG &= ~UCRXIFG;
    }

    ADXL_CS_DIS();

    return TRUE;
}


static void adxl312_spi_init(void)
{
    P3SEL |= BIT1 + BIT2 + BIT3;
    P3DIR |= BIT0 + BIT1 + BIT3;
    ADXL_CS_DIS();

    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCCKPL + UCMSB + UCMST + UCSYNC;
    UCB0CTL1 = UCSSEL__SMCLK;           // Select SMCLK as clock source
    UCB0BR0 = 16;
    UCB0BR1 = 0x00;                     // fBitClock = fBRCLK/UCBRx = SMCLK = DCO/BR  SCLK = 8M

    UCB0CTL1 &= ~UCSWRST;               // Initialize USCI state machine
    UCB0IE &= ~UCRXIE;
}

static void adxl312_port_init(void)
{
    // 配置传感器INT2引脚：TA0 as capture interrupt
    P2SEL &= ~BIT7;     //config P8.2 as input capture io
    P2DIR &= ~BIT7;     //config P8.2 as input

    // P2IES |= BIT7;
    P2IES &= ~BIT7;       // P2IFG flag is set with a low-to-high transition.
    P2IFG &= ~BIT7;     //P2IES的切换可能使P2IFG置位，需清除

    P2IE |= BIT7;        // Corresponding port interrupt enabled
}

static void adxl312_settings(void)
{
    uint8_t device_id = 0;
    adxl312_reg_read(ADXL_REG_DEVID, &device_id);
	if(device_id == 0)
	{
		return;
	}
    DBG_ASSERT(ADXL_DEVICE_ID == device_id __DBG_LINE);
#if 1
    adxl312_reg_write(ADXL_REG_BW_RATE, 0x0C);
    adxl312_reg_write(ADXL_REG_FIFO_CTL, 0x8a);
    adxl312_reg_write(ADXL_REG_POWER_CTL, 0x00);
    adxl312_reg_write(ADXL_REG_DATA_FORMAT, 0x08);
    adxl312_reg_write(ADXL_REG_OFSX, 0x00);
    adxl312_reg_write(ADXL_REG_OFSY, 0x00);
    adxl312_reg_write(ADXL_REG_OFSZ, 0x00);

    adxl312_reg_write(ADXL_REG_THRESH_TAP, 0x10);   //*< 1个单位是62.5mg
    adxl312_reg_write(ADXL_REG_DUR, 0x10);           //*< 1个单位是625us
    adxl312_reg_write(ADXL_REG_TAP_AXES, 0x07);

    adxl312_reg_write(ADXL_REG_INT_ENABLE, ADXL_SINGLE_TAP);
    adxl312_reg_write(ADXL_REG_INT_MAP, ADXL_SINGLE_TAP);
    adxl312_reg_write(ADXL_REG_POWER_CTL, 0x28);

    uint8_t int_source;
    adxl312_reg_read(ADXL_REG_INT_SOURCE, &int_source);
#else

    // ADXL345_REG_POWER_CTL[3]=0设置成待机模式,即清除测试位
    adxl312_reg_write(ADXL_REG_POWER_CTL, 0x00);

    //BW_RATE[4]=1；即设置LOW_POWER位低功耗
    //BW_RATE[3][2][1][0]=0x07，即设置输出速率12.5HZ，Idd=34uA
    //普通，100hz
    adxl312_reg_write(ADXL_REG_BW_RATE, 0x0A);

    adxl312_reg_write(ADXL_REG_OFSX, 0x00);      // Z轴偏移量
    adxl312_reg_write(ADXL_REG_OFSY, 0x00);      // Y轴偏移量
    adxl312_reg_write(ADXL_REG_OFSZ, 0x00);      // Z轴偏移量


    //THRESH_ACT:比例因子为62.5mg/LSB，
    //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff,,,//1.5g=0x18
    adxl312_reg_write(ADXL_REG_THRESH_ACT, 0x18);

    //THRESH_INACT:比例因子为62.5mg/LSB
    //1g=0x10  //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff
    adxl312_reg_write(ADXL_REG_THRESH_INACT, 0x10);

    //TIME_INACT:比例因子为1sec/LSB      //1s=0x01
    adxl312_reg_write(ADXL_REG_TIME_INACT, 0x05);

    //设置为直流耦合：当前加速度值直接与门限值比较，以确定是否运动或静止
    //x,y,z参与检测活动或静止
    adxl312_reg_write(ADXL_REG_ACT_INACT_CTL, 0x77);

    // 中断使能
    //1）DATA_READY[7]   2)SINGLE_TAP[6]  3)DOUBLE_TAP[5]  4)Activity[4]
    //5)inactivity[3]    6)FREE_FALL[2]   7)watermark[1]   8)overrun[0]
    adxl312_reg_write(ADXL_REG_INT_ENABLE, 0x18); //0xfc

    //INT_MAC中断映射：任意位设为0发送到INT1位，，设为1发送到INT2位
    //1）DATA_READY[7]   2)SINGLE_TAP[6]  3)DOUBLE_TAP[5]  4)Activity[4]
    //5)inactivity[3]    6)FREE_FALL[2]   7)watermark[1]   8)overrun[0]
    adxl312_reg_write(ADXL_REG_INT_MAP, 0x18);

    //1）SELF_TEST[7];2)SPI[6]; 3)INT_INVERT[5]：设置为0中断高电平有效，
    // 数据输出格式  高电平触发
    adxl312_reg_write(ADXL_REG_DATA_FORMAT, 0x0B);

    //设置 FIFO模式
    adxl312_reg_write(ADXL_REG_FIFO_CTL, 0xaa);
    // 进入测量模式
    //1)链接位[5]    2)AUTO_SLEEP[4]   3)测量位[3]  4)休眠位[2]  5)唤醒位[1][0]
    adxl312_reg_write(ADXL_REG_POWER_CTL, 0x28);
    uint8_t int_source;
    adxl312_reg_read(ADXL_REG_INT_SOURCE, &int_source);
#endif
}

static void adxl312_daemon_timer_sent_timeout_cb(TimerHandle_t timer)
{
	xTimerStop(timer, 0);
	uint8_t int_source;
	adxl312_reg_read(ADXL_REG_INT_SOURCE, &int_source);
		
	if (int_source & ADXL_SINGLE_TAP)
	{
		esn_msg_t esn_msg;
		esn_msg.event = GAIN_STOCK_START;
		xQueueSend(esn_gain_queue, &esn_msg, portMAX_DELAY);
	}
	else if (int_source & ADXL_INACTIVITY)
	{
		;
	}
	P2IFG &= ~BIT7;
	//adxl312_reg_read(ADXL_REG_INT_SOURCE, &int_source);
	P2IE |= BIT7;
}

void adxl_sensor_init(void)
{
    adxl312_spi_init();

    adxl312_settings();

    adxl312_port_init();
	
	adxl312_daemon_timer = xTimerCreate("adxl312_daemon_timer",
									 (6 * configTICK_RATE_HZ),
									 pdTRUE,
									 NULL,
									 adxl312_daemon_timer_sent_timeout_cb);
}

bool_t adxl_get_xyz( int16_t *pacc_x , int16_t *pacc_y , int16_t *pacc_z)
{
    uint8_t accbuf[6] = {0};
    bool_t ret = FALSE;               // 读写返回值

//    portENTER_CRITICAL();
    ret = adxl312_read_fifo( 0x32 , accbuf , ADXL_DATA_OUT_REG_NUM );
//    portEXIT_CRITICAL();
    
    DBG_ASSERT(TRUE == ret __DBG_LINE);

    *pacc_x = (accbuf[1] << 8 ) | accbuf[0];
    *pacc_y = (accbuf[3] << 8 ) | accbuf[2];
    *pacc_z = (accbuf[5] << 8 ) | accbuf[4];
    /*
        // 转换结果调整为mg
        *pacc_x = (fp32_t)( *pacc_x * 3.9);
        *pacc_y = (fp32_t)( *pacc_y * 3.9);
        *pacc_z = (fp32_t)( *pacc_z * 3.9);
    */
    return TRUE;
}

void adxl_get_triple_angle(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t read_buf[6];
    uint8_t cnt = 0;
    fp32_t ax = 0, ay = 0, az = 0;
    fp32_t ax2 = 0, ay2 = 0, az2 = 0;
    fp32_t angx, angy, angz;
    uint8_t int_source;


#define AVERAGE_CNT     (10)
    for (uint8_t i = 0; i < AVERAGE_CNT; i++)
    {
        adxl312_reg_read(ADXL_REG_INT_SOURCE, &int_source);

        if ((int_source & ADXL_DATA_READY) == ADXL_DATA_READY)
        {
            cnt++;
            adxl312_read_fifo(ADXL_REG_DATAX0, read_buf, ADXL_DATA_OUT_REG_NUM);
            ax += ((int16_t)(read_buf[1] << 8)) | read_buf[0];
            ay += ((int16_t)(read_buf[3] << 8)) | read_buf[2];
            az += ((int16_t)(read_buf[5] << 8)) | read_buf[4];
        }
    }

    ax = ax / cnt;
    ay = ay / cnt;
    az = az / cnt;

    ax2 = ax * ax;
    ay2 = ay * ay;
    az2 = az * az;

    angx = atan2(ax, sqrt(ay2 + az2));
    angy = atan2(ay, sqrt(ax2 + az2));
    angz = atan2(sqrt(ax2 + ay2), az);

    *x = (int16_t)(10 * angx * 180.0 / 3.1415926);
    *y = (int16_t)(10 * angy * 180.0 / 3.1415926);
    *z = (int16_t)(10 * angz * 180.0 / 3.1415926);

    *x = *x + 900;
    *y = *y + 900;
#undef AVERAGE_CNT
}

#pragma vector = PORT2_VECTOR
__interrupt void port2_isr(void)	//这个中断会频繁进入，影响GPRS串口接收
{
    if ((P2IFG & BIT7) == BIT7)
    {
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		P2IE &= ~BIT7;
        P2IFG &= ~BIT7;
		xTimerResetFromISR(adxl312_daemon_timer, &xHigherPriorityTaskWoken);
    }
    LPM3_EXIT;
}



