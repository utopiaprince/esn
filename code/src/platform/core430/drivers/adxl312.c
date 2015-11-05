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
#include <drivers.h>

#define ADXL_CS_EN()            (P3OUT |= BIT0)
#define ADXL_CS_DIS()           

bool_t adxl312_reg_read(uint8_t addr, uint8_t *pvalue)
{
#if 0
    uint8_t value;	
	ADXL_CS_EN();        
	addr |= RW_BIT;        
	while(!(UCB0IFG & UCTXIFG));			// Wait for USCI_B0 TX buffer ready
	UCB0TXBUF = addr;						// Send the header byte
	while(!(UCB0IFG & UCTXIFG));			// Wait for USCI_B0 TX complete
	while(!(UCB0IFG & UCRXIFG));			// Wait for USCI_B0 RX buffer ready
    UCB0IFG &= ~UCRXIFG;
	while(!(UCB0IFG & UCTXIFG));			// Wait for USCI_B0 TX buffer ready
	UCB0TXBUF = 0xFF;						// Send the header byte
	while(!(UCB0IFG & UCTXIFG));			// Wait for USCI_B0 TX complete
	while(!(UCB0IFG & UCRXIFG));			// Wait for USCI_B0 RX buffer ready
	*pvalue = UCB0RXBUF;
	ADXL_CS_DIS();
    
#else
    ADXL_CS_EN();
    UCB0I2CSA = ADXL_ADDRESS;
    UCB0CTL1 |= UCTR;                 // 写模式
    UCB0CTL1 |= UCTXSTT;              // 发送启动位和写控制字节

    UCB0TXBUF = addr;                   // 发送数据，必须要先填充TXBUF
    // 等待UCTXIFG=1 与UCTXSTT=0 同时变化 等待一个标志位即可
    while(!(UCB0IFG & UCTXIFG))
    {
        if( UCB0IFG & UCNACKIFG )       // 若无应答 UCNACKIFG=1
        {
            ADXL_CS_DIS();
            return FALSE;
        }
    }

    UCB0CTL1 &= ~UCTR;                // 读模式
    UCB0CTL1 |= UCTXSTT;              // 发送启动位和读控制字节

    while(UCB0CTL1 & UCTXSTT);        // 等待UCTXSTT=0
    // 若无应答 UCNACKIFG = 1

    UCB0CTL1 |= UCTXSTP;              // 先发送停止位

    while(!(UCB0IFG & UCRXIFG));      // 读取数据，读取数据在发送停止位之后
    *pvalue = UCB0RXBUF;

    while( UCB0CTL1 & UCTXSTP );
    ADXL_CS_DIS();
#endif
    
    return TRUE;
}

bool_t adxl312_reg_write(uint8_t addr, uint8_t pvalue)
{
    ADXL_CS_EN();
    UCB0I2CSA = ADXL_ADDRESS;
    while( UCB0CTL1 & UCTXSTP );
    UCB0CTL1 |= UCTR;                 // 写模式
    UCB0CTL1 |= UCTXSTT;              // 发送启动位

    UCB0TXBUF = addr;             // 发送寄存器地址
    // 等待UCTXIFG=1 与UCTXSTT=0 同时变化 等待一个标志位即可
    while(!(UCB0IFG & UCTXIFG))
    {
        if( UCB0IFG & UCNACKIFG )     // 若无应答 UCNACKIFG=1
        {
            ADXL_CS_DIS();
            return FALSE;
        }
    }

    UCB0TXBUF = pvalue;            // 发送寄存器内容
    while(!(UCB0IFG & UCTXIFG));      // 等待UCTXIFG=1

    UCB0CTL1 |= UCTXSTP;
    while(UCB0CTL1 & UCTXSTP);        // 等待发送完成
    
    ADXL_CS_DIS();
    return TRUE;
}

bool_t adxl345_read_buff(uint8_t reg_add , uint8_t *pregbuf , uint8_t  len)
{
    UCB1I2CSA = ADXL345_ADDRESS;
    while( UCB1CTL1 & UCTXSTP );
    UCB1CTL1 |= UCTR;                 // 写模式
    UCB1CTL1 |= UCTXSTT;              // 发送启动位和写控制字节

    UCB1TXBUF = reg_add;             // 发送数据
    // 等待UCTXIFG=1 与UCTXSTT=0 同时变化 等待一个标志位即可
    while(!(UCB1IFG & UCTXIFG))
    {
        if( UCB1IFG & UCNACKIFG )       // 若无应答 UCNACKIFG=1
        {
            return I2C_FAIL;
        }
    }

    UCB1CTL1 &= ~UCTR;                // 读模式
    UCB1CTL1 |= UCTXSTT;              // 发送启动位和读控制字节

    while(UCB1CTL1 & UCTXSTT);        // 等待UCTXSTT=0
    // 若无应答 UCNACKIFG = 1

    for(uint8_t i = 0; i < (len - 1); i++)
    {
        while(!(UCB1IFG & UCRXIFG));    // 读取数据
        *pregbuf++ = UCB1RXBUF;
    }

    UCB1CTL1 |= UCTXSTP;              // 在接收最后一个字节之前发送停止位

    while(!(UCB1IFG & UCRXIFG));      // 读取数据
    *pregbuf = UCB1RXBUF;

    while( UCB1CTL1 & UCTXSTP );

    return I2C_OK;
}

static void adxl312_iic_lock_init(void)
{
    //P3.3 作为模拟scl，输出9个信号
    P3SEL &= ~BIT3;// P3.3置成IO口模式
	P3DIR |= BIT3; //P3.3做为输出
    P3OUT |= BIT3;
    // 主设备模拟SCL，从高到低，输出9次，使得从设备释放SDA
    for(uint8_t i=0;i<9;i++)
    {
        P3OUT |= BIT3;
        osel_delay(1);
        P3OUT &= ~BIT3;
        osel_delay(1);
    }
}

static void adxl312_iic_init(void)
{
    adxl312_iic_lock_init();
    
    P3SEL |= BIT1 + BIT2 + BIT3;
    P3DIR |= BIT0 + BIT1 + BIT3;
    ADXL_CS_EN();

    UCB0CTL1 |= UCSWRST;
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
    UCB0CTL1 = UCSSEL__SMCLK;           // Select SMCLK as clock source
    UCB0BR0 = 80;
    UCB0BR1 = 0x00;                     // fBitClock = fBRCLK/UCBRx = SMCLK = DCO/BR  SCLK = 8M
    UCB0CTL0 &= ~UCSLA10;               // 7位地址模式
    UCB0CTL1 &= ~UCSWRST;               // Initialize USCI state machine
    UCB0IE &= ~UCRXIE;
}

static void adxl312_port_init(void)
{
    // 配置传感器INT2引脚：TA0 as capture interrupt
    P2SEL &= ~BIT7;   //config P8.2 as input capture io
    P2DIR &= ~BIT7;  //config P8.2 as input
    
    P2IES |= BIT7;       // P2IFG flag is set with a high-to-low transition.下降沿触发, ;
    P2IFG &= ~BIT7;     //P2IES的切换可能使P2IFG置位，需清除
    
    P2IE |= BIT7;        // Corresponding port interrupt enabled
}

static void adxl312_settings(void)
{
    uint8_t device_id = 0;
    adxl312_reg_read(ADXL_REG_DEVID, &device_id);
    DBG_ASSERT(ADXL_DEVICE_ID == device_id __DBG_LINE);
    // ADXL345_REG_POWER_CTL[3]=0设置成待机模式,即清除测试位
    adxl312_reg_write(ADXL_REG_POWER_CTL, 0x00); 
    
    //BW_RATE[4]=1；即设置LOW_POWER位低功耗
    //BW_RATE[3][2][1][0]=0x07，即设置输出速率12.5HZ，Idd=34uA
    //普通，100hz
    adxl312_reg_write(ADXL_REG_BW_RATE, 0x07);
    
    //adxl345_write_register(ADXL345_BW_RATE,0x0A);  
   //THRESH_TAP: 比例因子为62.5mg/lsb  建议大于3g 
    //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff   3.5g=0x38
    adxl312_reg_write(ADXL_REG_THRESH_TAP,0x38); 
                                                     
    adxl312_reg_write(ADXL_REG_OFSX,0x00);       // Z轴偏移量
    adxl312_reg_write(ADXL_REG_OFSY,0x00);       // Y轴偏移量
    adxl312_reg_write(ADXL_REG_OFSZ,0x00);       // Z轴偏移量
    
    //DUR:比例因子为625us/LSB，建议大于10ms
    //6.25ms=0x0A //12.5ms=0x14。
    adxl312_reg_write(ADXL_REG_DUR,0x14); 
    
    //Latent:比例因子为1.25ms/LSB，建议大于20ms
    //2.5ms=0x02，，20ms=0x10，，，25ms=0x14
    adxl312_reg_write(ADXL_REG_LATENT,0x14);    
    
    //window:比例因子为1.25ms/LSB，建议大于80ms 
    //10ms=0x08，，80ms=0x40
    adxl312_reg_write(ADXL_REG_WINDOW,0x41);  
    
    //THRESH_ACT:比例因子为62.5mg/LSB，
    //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff,,,//1.5g=0x18
    adxl312_reg_write(ADXL_REG_THRESH_ACT,0x18);
    
    //THRESH_INACT:比例因子为62.5mg/LSB   
    //1g=0x10  //2g=0X20,,,4g=0x40,,,8g=0x80,,,16g=0xff
    adxl312_reg_write(ADXL_REG_THRESH_INACT,0x10);
                                                    
    //TIME_INACT:比例因子为1sec/LSB      //1s=0x01                                           
    adxl312_reg_write(ADXL_REG_TIME_INACT,0x05);
    
    //设置为直流耦合：当前加速度值直接与门限值比较，以确定是否运动或静止  
    //x,y,z参与检测活动或静止
    adxl312_reg_write(ADXL_REG_ACT_INACT_CTL,0x77);
    
    //用于自由落地检测，比例因子为62.5mg/LSB   
    //建议设置成300mg~600mg（0x05~0x09）
    adxl312_reg_write(ADXL_REG_THRESH_FF,0x06);     
    
    //所有轴的值必须小于此设置值，才会触发中断;比例因子5ms/LSB   
    //建议设成100ms到350ms(0x14~~0x46),200ms=0x28
    adxl312_reg_write(ADXL_REG_TIME_FF,0x28);    
    
    //TAP_AXES:单击/双击轴控制寄存器； 
    //1）不抑制双击  2）使能x.y,z进行敲击检查
    adxl312_reg_write(ADXL_REG_TAP_AXES,0x07);  
    // 中断使能   
    //1）DATA_READY[7]   2)SINGLE_TAP[6]  3)DOUBLE_TAP[5]  4)Activity[4]
    //5)inactivity[3]    6)FREE_FALL[2]   7)watermark[1]   8)overrun[0]
    adxl312_reg_write(ADXL_REG_INT_ENABLE,0xfc); 
                                                    
    //INT_MAC中断映射：任意位设为0发送到INT1位，，设为1发送到INT2位
    //1）DATA_READY[7]   2)SINGLE_TAP[6]  3)DOUBLE_TAP[5]  4)Activity[4]
    //5)inactivity[3]    6)FREE_FALL[2]   7)watermark[1]   8)overrun[0] 
    adxl312_reg_write(ADXL_REG_INT_MAP,0x40);   
    
    //1）SELF_TEST[7];2)SPI[6]; 3)INT_INVERT[5]：设置为0中断高电平有效，
    // 数据输出格式  高电平触发
    adxl312_reg_write(ADXL_REG_DATA_FORMAT, 0x0B);
    //adxl345_write_register(ADXL345_DATA_FORMAT,0x2B);// 数据输出格式  低电平触发         
                                                    //反之设为1低电平有效    rang[1][0]
    //设置 FIFO模式
    adxl312_reg_write(ADXL_REG_FIFO_CTL,0xBF);
    // 进入测量模式
    //1)链接位[5]    2)AUTO_SLEEP[4]   3)测量位[3]  4)休眠位[2]  5)唤醒位[1][0]
    adxl312_reg_write(ADXL_REG_POWER_CTL,0x28);
    uint8_t int_source; 
    adxl312_reg_read(ADXL_REG_INT_SOURCE, &int_source);
}


void adxl_sensor_init(void)
{
    adxl312_iic_init();
    adxl312_port_init();
    adxl312_settings();
}

bool_t adxl345_get_xyz( int16_t *pacc_x , int16_t *pacc_y , int16_t *pacc_z)
{
    uint8_t accbuf[ADXL345_DATA_OUT_REG_NUM] = {0};
    uint8_t ret = I2C_FAIL;               // 读写返回值
    
    ret = adxl345_read_buff( 0x32 , accbuf , ADXL345_DATA_OUT_REG_NUM );
    DBG_ASSERT(I2C_OK == ret __DBG_LINE);

    *pacc_x = (accbuf[1] << 8 ) | accbuf[0];
    *pacc_y = (accbuf[3] << 8 ) | accbuf[2];
    *pacc_z = (accbuf[5] << 8 ) | accbuf[4];
/*
    // 转换结果调整为mg
    *pacc_x = (fp32_t)( *pacc_x * 3.9);
    *pacc_y = (fp32_t)( *pacc_y * 3.9);
    *pacc_z = (fp32_t)( *pacc_z * 3.9);
*/
    return I2C_OK;
}


static void adxl_sensor_int_handler(void)
{
    //*< do something
    //...
}

#pragma vector = PORT2_VECTOR
__interrupt void port2_isr(void)
{
    if((P2IFG & BIT7) == BIT7)
    {
        P2IFG &= ~BIT7;
        adxl_sensor_int_handler();
    }
    
    LPM3_EXIT;
}



