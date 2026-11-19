#include "uwb_diag.h"

uwb_diag_t g_uwb_diag;

void uwb_diag_start(void)
{
    memset(&g_uwb_diag, 0, sizeof(g_uwb_diag));
    g_uwb_diag.start_ms = HAL_GetTick();
    g_uwb_diag.active = 1;
}

void uwb_diag_rx_fault(uwb_diag_rx_t *rx, uint32_t status)
{
    if (!g_uwb_diag.active)
    {
        return;
    }

    rx->fault++;
    rx->preamble_timeout += (status & SYS_STATUS_RXPTO) != 0;
    rx->frame_timeout += (status & SYS_STATUS_RXRFTO) != 0;
    rx->sfd_timeout += (status & SYS_STATUS_RXSFDTO) != 0;
    rx->phr_error += (status & SYS_STATUS_RXPHE) != 0;
    rx->crc_error += (status & SYS_STATUS_RXFCE) != 0;
    rx->sync_loss += (status & SYS_STATUS_RXRFSL) != 0;
    rx->filter_reject += (status & SYS_STATUS_AFFREJ) != 0;
    rx->lde_error += (status & SYS_STATUS_LDEERR) != 0;
    rx->other += (status & (SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)) == 0;
}

static void uwb_diag_print_rx(const char *stage, const uwb_diag_rx_t *rx)
{
    debug_printf("UWB_DIAG %s ok=%lu reject=%lu late=%lu fault=%lu "
                 "pto=%lu fto=%lu sfd=%lu phr=%lu crc=%lu sync=%lu filter=%lu lde=%lu other=%lu\r\n",
                 stage, (unsigned long)rx->ok, (unsigned long)rx->rejected,
                 (unsigned long)rx->late, (unsigned long)rx->fault,
                 (unsigned long)rx->preamble_timeout, (unsigned long)rx->frame_timeout,
                 (unsigned long)rx->sfd_timeout, (unsigned long)rx->phr_error,
                 (unsigned long)rx->crc_error, (unsigned long)rx->sync_loss,
                 (unsigned long)rx->filter_reject, (unsigned long)rx->lde_error,
                 (unsigned long)rx->other);
}

static void uwb_diag_print_tx(const char *stage, const uwb_diag_tx_t *tx)
{
    debug_printf("UWB_DIAG %s sent=%lu start_fail=%lu done_timeout=%lu\r\n",
                 stage, (unsigned long)tx->sent, (unsigned long)tx->start_fail,
                 (unsigned long)tx->completion_timeout);
}

bool uwb_diag_stop(void)
{
    if (!g_uwb_diag.active)
    {
        return false;
    }

    g_uwb_diag.active = 0;
    debug_printf("UWB_DIAG role=anchor id=%u elapsed_ms=%lu\r\n",
                 (unsigned int)anc_id, (unsigned long)(HAL_GetTick() - g_uwb_diag.start_ms));
    uwb_diag_print_rx("poll_rx", &g_uwb_diag.poll);
    uwb_diag_print_tx("resp_tx", &g_uwb_diag.resp);
    uwb_diag_print_rx("final_rx", &g_uwb_diag.final);
    debug_printf("UWB_DIAG peer_rx_late=%lu distance_updates=%lu\r\n",
                 (unsigned long)g_uwb_diag.peer_rx_late, (unsigned long)g_uwb_diag.distance_updates);
    return true;
}
