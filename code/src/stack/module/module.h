/**
 * @brief       : this
 * @file        : module.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-16
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-16  v0.0.1  gang.cheng    first version
 */
#ifndef __MODULE_H__
#define __MODULE_H__

#define M_SLOT_EN           0

typedef enum
{
    TRAN_MODULE = 0x0100,
    PRIM_MODULE = 0x0200,
} mac_module_enum_t;

#define MAC_MODUEL_ENUM_MASK            (0xFF00)

/**
 * @brief tran moduel event type
 */
#define M_TRAN_RESEND_TIMEOUT_EVENT     (TRAN_MODULE | 0x01)
#define M_TRAN_CSMA_TIMEOUT_EVENT       (TRAN_MODULE | 0x02)
#define M_TRAN_RXOK_EVENT               (TRAN_MODULE | 0x03)
#define M_TRAN_RXOVR_EVENT              (TRAN_MODULE | 0x04)
#define M_TRAN_TXOK_EVENT               (TRAN_MODULE | 0x05)
#define M_TRAN_TX_ERROR_EVENT           (TRAN_MODULE | 0x06)
#define M_TRAN_TXUND_EVENT              (TRAN_MODULE | 0x07)
#define M_TRAN_ACK_TIMEOUT_EVENT        (TRAN_MODULE | 0x08)

#define M_PRIM_DATA_REQ_EVENT           (PRIM_MODULE | 0x01)
#define M_PRIM_LINK_REQ_EVENT           (PRIM_MODULE | 0X02)

#endif