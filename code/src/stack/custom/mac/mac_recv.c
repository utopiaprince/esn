
#include "oserl_arch.h"
#include "lib.h"

#include "pbuf.h"
#include "sbuf.h"

#include "module.h"
#include "m_tran.h"

typedef struct _tran_cfg_t
{
    tran_frm_parse_cb_t     frm_head_parse;
    tran_frm_parse_cb_t     frm_payload_parse;
    tran_tx_finish_cb_t     tx_finish;
    tran_frm_get_cb_t       frm_get;
	tran_send_ack_cb_t      send_ack;
} tran_cfg_t;

void m_recv_init(void)
{
	tran_cfg_t tran_cfg;
	tran_cfg.frm_get  = m_recv_frm_get();
	tran_cfg.frm_head_parse = m_recv_frm_hd_parse();
}
