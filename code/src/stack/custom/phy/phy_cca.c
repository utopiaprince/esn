/**
 * @brief       : Clear Channel Assessment and relative functions
 *
 * @file        : phy_cca.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */

#include <data_type_def.h>
#include <drivers.h>
#include <phy_cca.h>
#include <phy_state.h>

bool_t phy_cca(void)
{
    /* CCA期间不接收数据，接收时间较短，无溢出危险 */

//    hal_rf_cfg_int(HAL_RF_RXSFD_INT, FALSE);
//    hal_rf_cfg_int(HAL_RF_RXOK_INT, FALSE);
//
//    phy_set_state(PHY_RX_STATE);
//    delay_us(600); //等待RSSI寄存器值有效
//
//    int16_t rssi_dbm = hal_rf_get_rssi();
//    delay_us(200);
//    rssi_dbm += hal_rf_get_rssi();
//    rssi_dbm = (rssi_dbm >> 1);
//
//    phy_set_state(PHY_IDLE_STATE);
//
//    hal_rf_cfg_int(HAL_RF_RXSFD_INT, TRUE);
//    hal_rf_cfg_int(HAL_RF_RXOK_INT, TRUE);
//
//    if (rssi_dbm >= CCA_RSSI_THRESHOLD)
//    {
//        return FALSE;
//    }
//    else
//    {
//        return TRUE;
//    }
    return TRUE;
}

bool_t phy_cca_stop(void)
{
	return TRUE;
}

int8_t phy_get_rssi_largest(void)
{
//    return rf_get_rssi();
    return 0;
}

uint8_t phy_get_rssi(void)
{
//    return (rf_get_rssi() >> 1);
    return 0;
}

int8_t phy_rssi_average(uint8_t num)
{
    return 0;
}
