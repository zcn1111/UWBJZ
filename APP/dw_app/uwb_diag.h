#ifndef UWB_DIAG_H
#define UWB_DIAG_H

#include "instance.h"

typedef struct
{
    uint32_t ok;
    uint32_t rejected;
    uint32_t late;
    uint32_t fault;
    uint32_t preamble_timeout;
    uint32_t frame_timeout;
    uint32_t sfd_timeout;
    uint32_t phr_error;
    uint32_t crc_error;
    uint32_t sync_loss;
    uint32_t filter_reject;
    uint32_t lde_error;
    uint32_t other;
} uwb_diag_rx_t;

typedef struct
{
    uint32_t sent;
    uint32_t start_fail;
    uint32_t completion_timeout;
} uwb_diag_tx_t;

typedef struct
{
    uint8_t active;
    uint32_t start_ms;
    uwb_diag_rx_t poll;
    uwb_diag_tx_t resp;
    uwb_diag_rx_t final;
    uint32_t peer_rx_late;
    uint32_t distance_updates;
} uwb_diag_t;

extern uwb_diag_t g_uwb_diag;

/* Main-loop counters. RX callbacks only latch the hardware event. */
#define UWB_DIAG_INC(counter) do { if (g_uwb_diag.active) { (counter)++; } } while (0)

void uwb_diag_start(void);
bool uwb_diag_stop(void);
void uwb_diag_rx_fault(uwb_diag_rx_t *rx, uint32_t status);

#endif
