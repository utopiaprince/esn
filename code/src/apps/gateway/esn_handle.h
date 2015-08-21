/**
 * @brief       : this
 * @file        : esn_handle.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-21
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-21  v0.0.1  gang.cheng    first version
 */
#ifndef __ESN_HANDLE_H__
#define __ESN_HANDLE_H__

typedef struct
{
    uint32_t event;
    void* param;
} esn_msg_t;

void esn_handle_init(void);

#endif