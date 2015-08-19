/**
 * @brief       : PHY state control
 *
 * @file        : phy_state.c
 * @author      : gang.cheng
 * @version     : v0.0.1
 * @date        : 2015/5/7
 *
 * Change Logs  :
 *
 * Date        Version      Author      Notes
 * 2015/5/7    v0.0.1      gang.cheng    first version
 */
#include "lib.h"
#include <drivers.h>
#include <phy_state.h>

uint8_t phy_get_state(void)
{
//    uint8_t state = hal_rf_get_state();
//    uint8_t phy_rf_state = PHY_INVALID_STATE;
//
//    switch(state)
//    {
//    case HAL_RF_IDLE_STATE:
//        phy_rf_state = PHY_IDLE_STATE;
//        break;
//
//    case HAL_RF_RX_STATE:
//        phy_rf_state = PHY_RX_STATE;
//        break;
//
//    case HAL_RF_TX_STATE:
//        phy_rf_state = PHY_TX_STATE;
//        break;
//
//    case HAL_RF_SLEEP_STATE:
//        phy_rf_state = PHY_SLEEP_STATE;
//        break;
//
//    case HAL_RF_INVALID_STATE:
//        phy_rf_state = PHY_INVALID_STATE;
//        break;
//
//    default:
//        break;
//    }

//    return phy_rf_state;
    return 0;
}


bool_t phy_set_state(uint8_t phy_state)
{
    return TRUE;
//    uint8_t hal_rf_state_wanted = HAL_RF_INVALID_STATE;
//
//    switch(phy_state)
//    {
//    case PHY_RX_STATE:
//        hal_rf_state_wanted = HAL_RF_RX_STATE;
//        break;
//
//    case PHY_TX_STATE:
//        hal_rf_state_wanted = HAL_RF_TX_STATE;
//        break;
//
//    case PHY_IDLE_STATE:
//        hal_rf_state_wanted = HAL_RF_IDLE_STATE;
//        break;
//
//    case PHY_SLEEP_STATE:
//        hal_rf_state_wanted = HAL_RF_SLEEP_STATE;
//        break;
//
//    default:
//      break;
//    }
//
//    return (hal_rf_set_state(hal_rf_state_wanted)) ;
}


bool_t phy_set_channel(uint8_t channel_index)
{
//    return hal_rf_set_channel(channel_index);
    return TRUE;
}


uint8_t phy_get_power(void)
{
//    return hal_rf_get_power();
    return 0;
}


bool_t phy_set_power(uint8_t power_index)
{
//    return hal_rf_set_power(power_index);
    return TRUE;
}
