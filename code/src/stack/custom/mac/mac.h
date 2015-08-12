/**
 * @brief       : this
 * @file        : mac.h
 * @version     : v0.0.1
 * @author      : gang.cheng
 * @date        : 2015-08-12
 * change logs  :
 * Date       Version     Author        Note
 * 2015-08-12  v0.0.1  gang.cheng    first version
 */
#ifndef __MAC_H__
#define __MAC_H__











/**
 * @brief send sbuf to mac queue
 *  
 * @param[in] mbuf pointer to store data buf
 *
 * @return has sent data to queue?
 *	- FALSE the queue is full
 *	- TRUE	has sent data to queue 
 */
bool_t mac_queue_send(mbuf_t *mbuf);

/**
 * @brief mac task and queue init
 */
void mac_init(void);

#endif
