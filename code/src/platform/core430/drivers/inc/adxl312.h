/**
 * @brief       : 
 *
 * @file        : adxl312.h
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/11/5
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/11/5    v0.0.1      gang.cheng    first version
 */
#ifndef __ADXL312_H__
#define __ADXL312_H__

#define ADXL_DATA_OUT_REG_NUM       (6u)

#define RW_BIT                      (0x80)
#define MB_BIT                      (0x40)

/** ADXL345_ID
*******************************************************************************/
#define ADXL_DEVICE_ID              (0xE5)

/** INSTRUCTION CODES
*******************************************************************************/
#define ADXL_REG_DEVID              (0x00)  // 0   R   11100101  Device ID
#define ADXL_REG_THRESH_TAP         (0x1D)  // 29  R/W 00000000  Tap threshold
#define ADXL_REG_OFSX               (0x1E)  // 30  R/W 00000000  X-axis offset
#define ADXL_REG_OFSY               (0x1F)  // 31  R/W 00000000  Y-axis offset
#define ADXL_REG_OFSZ               (0x20)  // 32  R/W 00000000  Z-axis offset
#define ADXL_REG_DUR                (0x21)  // 33  R/W 00000000  Tap duration
#define ADXL_REG_LATENT             (0x22)  // 34  R/W 00000000  Tap latency
#define ADXL_REG_WINDOW             (0x23)  // 35  R/W 00000000  Tap window
#define ADXL_REG_THRESH_ACT         (0x24)  // 36  R/W 00000000  Activity threshold
#define ADXL_REG_THRESH_INACT       (0x25)  // 37  R/W 00000000  Inactivity threshold
#define ADXL_REG_TIME_INACT         (0x26)  // 38  R/W 00000000  Inactivity time
#define ADXL_REG_ACT_INACT_CTL      (0x27)  // 39  R/W 00000000  Axis enable control for activity //                   and inactivity detection
#define ADXL_REG_THRESH_FF          (0x28)  // 40  R/W 00000000  Free-fall threshold
#define ADXL_REG_TIME_FF            (0x29)  // 41  R/W 00000000  Free-fall time
#define ADXL_REG_TAP_AXES           (0x2A)  // 42  R/W 00000000  Axis control for single tap/double tap
#define ADXL_REG_ACT_TAP_STATUS     (0x2B)  // 43  R   00000000  Source of single tap/double tap
#define ADXL_REG_BW_RATE            (0x2C)  // 44  R/W 00001010  Data rate and power mode control
#define ADXL_REG_POWER_CTL          (0x2D)  // 45  R/W 00000000  Power-saving features control
#define ADXL_REG_INT_ENABLE         (0x2E)  // 46  R/W 00000000  Interrupt enable control
#define ADXL_REG_INT_MAP            (0x2F)  // 47  R/W 00000000  Interrupt mapping control
#define ADXL_REG_INT_SOURCE         (0x30)  // 48  R   00000010  Source of interrupts
#define ADXL_REG_DATA_FORMAT        (0x31)  // 49  R/W 00000000  Data format control
#define ADXL_REG_DATAX0             (0x32)  // 50  R   00000000  X-Axis Data 0
#define ADXL_REG_DATAX1             (0x33)  // 51  R   00000000  X-Axis Data 1
#define ADXL_REG_DATAY0             (0x34)  // 52  R   00000000  Y-Axis Data 0
#define ADXL_REG_DATAY1             (0x35)  // 53  R   00000000  Y-Axis Data 1
#define ADXL_REG_DATAZ0             (0x36)  // 54  R   00000000  Z-Axis Data 0
#define ADXL_REG_DATAZ1             (0x37)  // 55  R   00000000  Z-Axis Data 1
#define ADXL_REG_FIFO_CTL           (0x38)  // 56  R/W 00000000  FIFO control
#define ADXL_REG_FIFO_STATUS        (0x39)  // 57  R   00000000  FIFO status

/** INT_ENABLE/INT_MAP/INT_SOURCE Bits
*******************************************************************************/
#define ADXL_DATA_READY             (0x01 << 7)
#define ADXL_SINGLE_TAP             (0x01 << 6)
#define ADXL_DOUBLE_TAP             (0x01 << 5)
#define ADXL_ACTIVITY               (0x01 << 4)
#define ADXL_INACTIVITY             (0x01 << 3)
#define ADXL_FREE_FALL              (0x01 << 2)
#define ADXL_WATERMARK              (0x01 << 1)
#define ADXL_OVERRUN                (0x01 << 0)

/** ACT_INACT_CONTROL Bits
*******************************************************************************/
#define ADXL_ACT_ACDC               (0x01 << 7)
#define ADXL_ACT_X_EN               (0x01 << 6)
#define ADXL_ACT_Y_EN               (0x01 << 5)
#define ADXL_ACT_Z_EN               (0x01 << 4)
#define ADXL_INACT_ACDC             (0x01 << 3)
#define ADXL_INACT_X_EN             (0x01 << 2)
#define ADXL_INACT_Y_EN             (0x01 << 1)
#define ADXL_INACT_Z_EN             (0x01 << 0)

/** TAP_AXES Bits
*******************************************************************************/
#define ADXL_SUPPRESS               (0x01 << 3)
#define ADXL_TAP_X_EN               (0x01 << 2)
#define ADXL_TAP_Y_EN               (0x01 << 1)
#define ADXL_TAP_Z_EN               (0x01 << 0)

/** ACT_TAP_STATUS Bits
*******************************************************************************/
#define ADXL_ACT_X_SRC               (0x01 << 6)
#define ADXL_ACT_Y_SRC               (0x01 << 5)
#define ADXL_ACT_Z_SRC               (0x01 << 4)
#define ADXL_ASLEEP                  (0x01 << 3)
#define ADXL_TAP_X_SRC               (0x01 << 2)
#define ADXL_TAP_Y_SRC               (0x01 << 1)
#define ADXL_TAP_Z_SRC               (0x01 << 0)

/** BW_RATE Bits
*******************************************************************************/
#define ADXL_LOW_POWER              (0x01 << 4)
#define ADXL_RATE(x)                ((x) & 0xF)

/** POWER_CTL Bits
*******************************************************************************/
#define ADXL_PCTL_LINK              (0x01 << 5)
#define ADXL_PCTL_AUTO_SLEEP        (0x01 << 4)
#define ADXL_PCTL_MEASURE           (0x01 << 3)
#define ADXL_PCTL_SLEEP             (0x01 << 2)
#define ADXL_PCTL_WAKEUP(x)         ((x) & 0x3)

/**  DATA_FORMAT Bits
*******************************************************************************/
#define ADXL_SELF_TEST              (0x01 << 7)
#define ADXL_SPI                    (0x01 << 6)
#define ADXL_INT_INVERT             (0x01 << 5)
#define ADXL_FULL_RES               (0x01 << 3)
#define ADXL_JUSTIFY                (0x01 << 2)
#define ADXL_RANGE(x)               ((x) & 0x3)
#define ADXL_RANGE_PM_2g            (0x00)
#define ADXL_RANGE_PM_4g            (0x01)
#define ADXL_RANGE_PM_8g            (0x02)
#define ADXL_RANGE_PM_16g           (0x03)

/**  Maximum value our axis may get in full res mode for the input device
 *   (signed 13 bits)
*******************************************************************************/
#define ADXL_FULLRES_MAX_VAL        (4096)

/**  Maximum value our axis may get in fixed res mode for the input device
 *  (signed 10 bits)
*******************************************************************************/
#define ADXL_FIXEDRES_MAX_VAL       (512)

/**  FIFO_CTL Bits
*******************************************************************************/
#define ADXL_FIFO_MODE(x)           (((x) & 0x3) << 6)
#define ADXL_FIFO_BYPASS            (0x00)
#define ADXL_FIFO_FIFO              (0x01)
#define ADXL_FIFO_STREAM            (0x02)
#define ADXL_FIFO_TRIGGER           (0x03)
#define ADXL_TRIGGER                (0x01 << 5)
#define ADXL_SAMPLES(x)             ((x) & 0x1F)

/**  FIFO_STATUS Bits
*******************************************************************************/
#define ADXL_FIFO_TRIG              (0x01 << 7)
#define ADXL_ENTRIES(x)             ((x) & 0x3F)

/**  FIFO_CTL Bits
*******************************************************************************/
#define ADXL_X_AXIS                 (0x00)
#define ADXL_Y_AXIS                 (0x01)
#define ADXL_Z_AXIS                 (0x02)

#define GAIN_STOCK          	(7u)
#define GAIN_STOCK_START        ((GAIN_STOCK<<8) | 0)

void adxl_sensor_init(void);

uint8_t adxl_sensor_getid(void);

bool_t adxl_get_xyz(int16_t *x, int16_t *y, int16_t *z);

/**
 * @brief if call this function, it cost 100ms;
 * @param x [description]
 * @param y [description]
 * @param z [description]
 */
void adxl_get_triple_angle(int16_t *x, int16_t *y, int16_t *z);
#endif

