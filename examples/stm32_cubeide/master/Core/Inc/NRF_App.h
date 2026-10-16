/*
 * NRF_App.h
 *
 *  Created on: Sep 25, 2026
 *      Author: amire
 */

#ifndef INC_NRF_APP_H_
#define INC_NRF_APP_H_


#include "nrf.h"

/* ============================================================
 * TX Result Enum
 * ============================================================ */
typedef enum {
    NRF_TX_SUCCESS = 0,
    NRF_TX_MAX_RT  = 1,
    NRF_TX_TIMEOUT = 2,
    NRF_TX_FAILED  = 3
} NRF_TxResult_t;

/* ============================================================
 * Forward Declarations & Event Types
 * ============================================================ */

typedef struct NRF_App_s NRF_App_t;


typedef struct {
    uint8_t rx_data_ready;
    uint8_t tx_data_sent;
    uint8_t tx_max_retry;
    uint8_t rx_pipe;
} NRF_App_Event_t;

typedef void (*NRF_App_EventCallback_t)(NRF_App_t *app, const NRF_App_Event_t *event, void *user_arg);

/* ============================================================
 * Application Configuration Structure
 * ============================================================ */
typedef struct {
    NRF_Role_t            role;
    NRF_CrcState_t        crc_state;
    NRF_CrcLength_t       crc_length;
    NRF_PipeMask_t        auto_ack_mask;
    NRF_PipeMask_t        rx_addr_mask;
    NRF_AddrWidth_t       addr_width;
    NRF_RetransmitDelay_t retr_delay;
    uint8_t               retr_count;
    uint8_t               rf_channel;     /* 0 to 125 */
    NRF_DataRate_t        data_rate;
    NRF_OutputPower_t     rf_pwr;
    NRF_PipeMask_t        dynpd_mask;
    NRF_FeatureMask_t     feature_mask;
} NRF_App_Config_t;

/* ============================================================
 * Application Handle
 * ============================================================ */
struct NRF_App_s {
    NRF_Device_t            dev;
    NRF_App_EventCallback_t event_callback;
    void                   *callback_user_arg;
};
/* ============================================================
 * Lifecycle & Control
 * ============================================================ */
NRF_Status_t NRF_App_Init(NRF_App_t *app, const NRF_App_Config_t *config);
NRF_Status_t NRF_App_DeInit(NRF_App_t *app);
NRF_Status_t NRF_App_Start(NRF_App_t *app);
NRF_Status_t NRF_App_Stop(NRF_App_t *app);
NRF_Status_t NRF_App_Restart(NRF_App_t *app);
NRF_Status_t NRF_App_Recover(NRF_App_t *app);
NRF_Status_t NRF_App_SelfTest(NRF_App_t *app);

/* ============================================================
 * Mode Switching
 * ============================================================ */
NRF_Status_t NRF_App_SetTxMode(NRF_App_t *app);
NRF_Status_t NRF_App_SetRxMode(NRF_App_t *app);
NRF_Status_t NRF_App_SetStandby(NRF_App_t *app);
NRF_Status_t NRF_App_PowerDown(NRF_App_t *app);

/* ============================================================
 * Data Transfer
 * ============================================================ */
NRF_Status_t NRF_App_Send(NRF_App_t *app, const uint8_t *data, uint8_t size);
NRF_Status_t NRF_App_Transmit(NRF_App_t *app, const uint8_t *data, uint8_t size);
NRF_Status_t NRF_App_TransmitWait(NRF_App_t *app, const uint8_t *data, uint8_t size, uint32_t timeout_ms, NRF_TxResult_t *tx_result);

NRF_Status_t NRF_App_DataAvailable(NRF_App_t *app, uint8_t *available);
NRF_Status_t NRF_App_Receive(NRF_App_t *app, uint8_t *data, uint8_t size);
NRF_Status_t NRF_App_ReceivePipe(NRF_App_t *app, uint8_t *data, uint8_t size, NRF_RxPipeStatus_t *pipe);

NRF_Status_t NRF_App_ReadPacket(NRF_App_t *app, uint8_t *data, uint8_t *actual_size);

/* ============================================================
 * Pipe & Address Management
 * ============================================================ */
NRF_Status_t NRF_App_EnablePipe(NRF_App_t *app, uint8_t pipe);
NRF_Status_t NRF_App_DisablePipe(NRF_App_t *app, uint8_t pipe);
NRF_Status_t NRF_App_EnableAutoAck(NRF_App_t *app, uint8_t pipe);
NRF_Status_t NRF_App_DisableAutoAck(NRF_App_t *app, uint8_t pipe);

NRF_Status_t NRF_App_SetPipePayloadSize(NRF_App_t *app, uint8_t pipe, uint8_t size);
NRF_Status_t NRF_App_GetPipePayloadSize(NRF_App_t *app, uint8_t pipe, uint8_t *size);

NRF_Status_t NRF_App_SetRxAddress(NRF_App_t *app, uint8_t pipe, const uint8_t *address, uint8_t size);
NRF_Status_t NRF_App_GetRxAddress(NRF_App_t *app, uint8_t pipe, uint8_t *address, uint8_t size);
NRF_Status_t NRF_App_SetTxAddress(NRF_App_t *app, const uint8_t *address, uint8_t size);
NRF_Status_t NRF_App_GetTxAddress(NRF_App_t *app, uint8_t *address, uint8_t size);

/* ============================================================
 * Dynamic Payload & Features
 * ============================================================ */
NRF_Status_t NRF_App_EnableDynamicPayload(NRF_App_t *app, uint8_t pipe);
NRF_Status_t NRF_App_DisableDynamicPayload(NRF_App_t *app, uint8_t pipe);
NRF_Status_t NRF_App_IsDynamicPayloadEnabled(NRF_App_t *app, uint8_t pipe, uint8_t *enabled);
NRF_Status_t NRF_App_EnableFeature(NRF_App_t *app, NRF_FeatureMask_t feature);
NRF_Status_t NRF_App_DisableFeature(NRF_App_t *app, NRF_FeatureMask_t feature);
NRF_Status_t NRF_App_GetFeatures(NRF_App_t *app, NRF_FeatureMask_t *features);

/* ============================================================
 * FIFO & Event Diagnostics
 * ============================================================ */
NRF_Status_t NRF_App_FlushTx(NRF_App_t *app);
NRF_Status_t NRF_App_FlushRx(NRF_App_t *app);
NRF_Status_t NRF_App_ResetFifo(NRF_App_t *app);

NRF_Status_t NRF_App_IsTxEmpty(NRF_App_t *app, uint8_t *empty);
NRF_Status_t NRF_App_IsRxEmpty(NRF_App_t *app, uint8_t *empty);
NRF_Status_t NRF_App_IsTxFull(NRF_App_t *app, uint8_t *full);
NRF_Status_t NRF_App_IsRxFull(NRF_App_t *app, uint8_t *full);
NRF_Status_t NRF_App_IsTxReuseActive(NRF_App_t *app, uint8_t *active);

NRF_Status_t NRF_App_GetRxStatus(NRF_App_t *app, NRF_RxDrStatus_t *status_value);
NRF_Status_t NRF_App_GetTxStatus(NRF_App_t *app, NRF_TxDsStatus_t *status_value);
NRF_Status_t NRF_App_GetMaxRtStatus(NRF_App_t *app, NRF_MaxRtStatus_t *status_value);

NRF_Status_t NRF_App_ClearRxEvent(NRF_App_t *app);
NRF_Status_t NRF_App_ClearTxEvent(NRF_App_t *app);
NRF_Status_t NRF_App_ClearMaxRtEvent(NRF_App_t *app);

NRF_Status_t NRF_App_GetRxPipe(NRF_App_t *app, NRF_RxPipeStatus_t *pipe);
NRF_Status_t NRF_App_GetPacketLoss(NRF_App_t *app, NRF_PacketLossCount_t *count);
NRF_Status_t NRF_App_GetRetryCount(NRF_App_t *app, NRF_RetransmitCount_t *count);
NRF_Status_t NRF_App_SetCE(NRF_App_t *app, NRF_PinState_t state);

/* ============================================================
 * Advanced ESB Features (ACK Payload, Scanning, Diagnostics)
 * ============================================================ */
NRF_Status_t NRF_App_LoadAckPayload(NRF_App_t *app, uint8_t pipe, const uint8_t *data, uint8_t size);

NRF_Status_t NRF_App_TransmitNoAck(NRF_App_t *app, const uint8_t *data, uint8_t size);

NRF_Status_t NRF_App_ScanChannel(NRF_App_t *app, uint8_t channel, uint8_t *is_occupied);

NRF_Status_t NRF_App_ResetPacketLoss(NRF_App_t *app);

NRF_Status_t NRF_App_PollEvents(NRF_App_t *app, NRF_App_Event_t *events);

/* ============================================================
 * IRQ & Handler Functions
 * ============================================================ */
void NRF_App_IRQHandler(NRF_App_t *app);

NRF_Status_t NRF_App_SetEventCallback(NRF_App_t *app, NRF_App_EventCallback_t callback, void *user_arg);

NRF_Status_t NRF_App_ConfigureInterrupts(NRF_App_t *app, NRF_RxDrMask_t rx_dr, NRF_TxDsMask_t tx_ds, NRF_MaxRtMask_t max_rt);
#endif /* INC_NRF_APP_H_ */
