/*
 * NRF_App.c
 *
 *  Created on: Sep 25, 2026
 *      Author: amire
 */

#include "NRF_App.h"
#include <string.h>

/* ============================================================
 * Internal Validation Helpers
 * ============================================================ */
static NRF_Status_t NRF_App_Check(NRF_App_t *app)
{
    if (app == NULL)
    {
        return NRF_ERR_INVALID_HANDLE;
    }
    return NRF_OK;
}

static NRF_Status_t NRF_App_CheckPipe(uint8_t pipe)
{
    if (pipe > 5U)
    {
        return NRF_ERR_INVALID_PARAM;
    }
    return NRF_OK;
}

static uint8_t NRF_App_AddressWidthBytes(NRF_AddrWidth_t width)
{
    switch (width)
    {
        case NRF_AW_3BYTES: return 3U;
        case NRF_AW_4BYTES: return 4U;
        case NRF_AW_5BYTES: return 5U;
        default:            return 0U;
    }
}

static NRF_Status_t NRF_App_GetAddressWidthBytes(NRF_App_t *app, uint8_t *width)
{
    NRF_AddrWidth_t addr_width;
    NRF_Status_t status;
    if (width == NULL)
    {
        return NRF_ERR_INVALID_PARAM;
    }
    status = NRF_GetAddressWidth(&app->dev, &addr_width);
    if (status != NRF_OK)
    {
        return status;
    }
    *width = NRF_App_AddressWidthBytes(addr_width);
    if (*width == 0U)
    {
        return NRF_ERR_INVALID_PARAM;
    }
    return NRF_OK;
}

/* ============================================================
 * Lifecycle & Initialization
 * ============================================================ */
NRF_Status_t NRF_App_Init(NRF_App_t *app, const NRF_App_Config_t *config)
{
    NRF_Status_t status;
    if (app == NULL || config == NULL)
    {
        return NRF_ERR_INVALID_PARAM;
    }
    NRF_Device_t *nrf = &app->dev;

    status = NRF_Init(nrf);
    if (status != NRF_OK)
    {
        return status;
    }
    status = NRF_SetRole(nrf, config->role);
    if (status != NRF_OK) goto error;

    status = NRF_SetCrcState(nrf, config->crc_state);
    if (status != NRF_OK) goto error;

    status = NRF_SetCrcLength(nrf, config->crc_length);
    if (status != NRF_OK) goto error;

    status = NRF_SetAutoAckBitmask(nrf, config->auto_ack_mask);
    if (status != NRF_OK) goto error;

    status = NRF_SetRxAddrBitmask(nrf, config->rx_addr_mask);
    if (status != NRF_OK) goto error;

    status = NRF_SetAddressWidth(nrf, config->addr_width);
    if (status != NRF_OK) goto error;

    status = NRF_SetAutoRetransmit(nrf, config->retr_delay, config->retr_count);
    if (status != NRF_OK) goto error;

    status = NRF_SetChannel(nrf, config->rf_channel);
    if (status != NRF_OK) goto error;

    status = NRF_SetDataRate(nrf, config->data_rate);
    if (status != NRF_OK) goto error;

    status = NRF_SetOutputPower(nrf, config->rf_pwr);
    if (status != NRF_OK) goto error;

    status = NRF_SetFeatureMask(nrf, config->feature_mask);
    if (status != NRF_OK) goto error;

    status = NRF_SetDynamicPayloadMask(nrf, config->dynpd_mask);
    if (status != NRF_OK) goto error;

    return NRF_OK;

error:
    NRF_DeInit(nrf);
    return status;
}

NRF_Status_t NRF_App_DeInit(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_DeInit(&app->dev);
}

NRF_Status_t NRF_App_Start(NRF_App_t *app)
{
    NRF_Role_t role;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_GetRole(&app->dev, &role);
    if (status != NRF_OK) return status;

    status = NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    if (status != NRF_OK) return status;

    status = NRF_SetPowerMode(&app->dev, NRF_POWER_UP);
    if (status != NRF_OK) return status;

    if (role == NRF_ROLE_PRX)
    {
        return NRF_SetCEState(&app->dev, NRF_PIN_SET);
    }

    return NRF_OK;
}

NRF_Status_t NRF_App_Stop(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    if (status != NRF_OK) return status;

    return NRF_SetPowerMode(&app->dev, NRF_POWER_DOWN);
}

NRF_Status_t NRF_App_Restart(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Stop(app);
    if (status != NRF_OK) return status;

    return NRF_App_Start(app);
}

NRF_Status_t NRF_App_Recover(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    if (status != NRF_OK) return status;

    NRF_ClearRxDrFlag(&app->dev);
    NRF_ClearTxDsFlag(&app->dev);
    NRF_ClearMaxRtFlag(&app->dev);

    return NRF_App_ResetFifo(app);
}

NRF_Status_t NRF_App_SelfTest(NRF_App_t *app)
{
    NRF_TxFullStatus_t tx_full;
    NRF_RxPipeStatus_t rx_pipe;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_GetTxFullStatus(&app->dev, &tx_full);
    if (status != NRF_OK) return status;

    status = NRF_GetRxPipeNumber(&app->dev, &rx_pipe);
    if (status != NRF_OK) return status;

    (void)tx_full;
    (void)rx_pipe;
    return NRF_OK;
}

/* ============================================================
 * Mode Switching
 * ============================================================ */
NRF_Status_t NRF_App_SetTxMode(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    if (status != NRF_OK) return status;

    status = NRF_SetRole(&app->dev, NRF_ROLE_PTX);
    if (status != NRF_OK) return status;

    return NRF_SetPowerMode(&app->dev, NRF_POWER_UP);
}

NRF_Status_t NRF_App_SetRxMode(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    if (status != NRF_OK) return status;

    status = NRF_SetRole(&app->dev, NRF_ROLE_PRX);
    if (status != NRF_OK) return status;

    status = NRF_SetPowerMode(&app->dev, NRF_POWER_UP);
    if (status != NRF_OK) return status;

    return NRF_SetCEState(&app->dev, NRF_PIN_SET);
}

NRF_Status_t NRF_App_SetStandby(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    if (status != NRF_OK) return status;

    return NRF_SetPowerMode(&app->dev, NRF_POWER_UP);
}

NRF_Status_t NRF_App_PowerDown(NRF_App_t *app)
{
    return NRF_App_Stop(app);
}

/* ============================================================
 * Data Transfer
 * ============================================================ */
NRF_Status_t NRF_App_Send(NRF_App_t *app, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (data == NULL || size == 0U || size > 32U)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    return NRF_WriteTxPayload(&app->dev, data, size);
}

NRF_Status_t NRF_App_Transmit(NRF_App_t *app, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (data == NULL || size == 0U || size > 32U)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    return NRF_TransmitPayload(&app->dev, data, size);
}

NRF_Status_t NRF_App_TransmitWait(NRF_App_t *app, const uint8_t *data, uint8_t size, uint32_t timeout_ms, NRF_TxResult_t *tx_result)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (data == NULL || size == 0U || size > 32U || tx_result == NULL)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    *tx_result = NRF_TX_FAILED;

    NRF_Role_t original_role = app->dev.role;
    if (original_role == NRF_ROLE_PRX)
    {
        NRF_SetCEState(&app->dev, NRF_PIN_RESET);
        NRF_SetRole(&app->dev, NRF_ROLE_PTX);
        NRF_delay_us(&app->dev.hw.bus_dev, 130);
    }

    status = NRF_TransmitPayload(&app->dev, data, size);
    if (status != NRF_OK)
    {
        if (original_role == NRF_ROLE_PRX)
        {
            NRF_SetRole(&app->dev, NRF_ROLE_PRX);
            NRF_SetCEState(&app->dev, NRF_PIN_SET);
        }
        return status;
    }

    NRF_TxDsStatus_t tx_ds = NRF_TX_DS_INACTIVE;
    NRF_MaxRtStatus_t max_rt = NRF_MAX_RT_INACTIVE;
    uint32_t elapsed = 0;

    while (elapsed < timeout_ms)
    {
        NRF_GetTxDsStatus(&app->dev, &tx_ds);
        NRF_GetMaxRtStatus(&app->dev, &max_rt);

        if (tx_ds == NRF_TX_DS_ACTIVE || max_rt == NRF_MAX_RT_ACTIVE)
        {
            break;
        }

        NRF_delay(&app->dev.hw.bus_dev, 1);
        elapsed++;
    }

    if (tx_ds == NRF_TX_DS_ACTIVE)
    {
        NRF_ClearTxDsFlag(&app->dev);
        *tx_result = NRF_TX_SUCCESS;
        status = NRF_OK;
    }
    else if (max_rt == NRF_MAX_RT_ACTIVE)
    {
        NRF_ClearMaxRtFlag(&app->dev);
        NRF_FlushTx(&app->dev);
        *tx_result = NRF_TX_MAX_RT;
        status = NRF_OK;
    }
    else
    {
        NRF_FlushTx(&app->dev);
        *tx_result = NRF_TX_TIMEOUT;
        status = NRF_ERR_TIMEOUT;
    }

    if (original_role == NRF_ROLE_PRX)
    {
        NRF_SetRole(&app->dev, NRF_ROLE_PRX);
        NRF_SetCEState(&app->dev, NRF_PIN_SET);
    }

    return status;
}

NRF_Status_t NRF_App_DataAvailable(NRF_App_t *app, uint8_t *available)
{
    NRF_RxFifoEmpty_t fifo_empty;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (available == NULL)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    status = NRF_GetRxFifoEmptyStatus(&app->dev, &fifo_empty);
    if (status != NRF_OK) return status;

    *available = (fifo_empty == NRF_RX_FIFO_NOT_EMPTY) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_Receive(NRF_App_t *app, uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (data == NULL || size == 0U || size > 32U)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    status = NRF_ReadRxPayload(&app->dev, data, size);
    if (status != NRF_OK) return status;

    NRF_ClearRxDrFlag(&app->dev);
    return NRF_OK;
}

NRF_Status_t NRF_App_ReceivePipe(NRF_App_t *app, uint8_t *data, uint8_t size, NRF_RxPipeStatus_t *pipe)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (pipe == NULL || data == NULL || size == 0U || size > 32U)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    status = NRF_GetRxPipeNumber(&app->dev, pipe);
    if (status != NRF_OK) return status;

    if (*pipe > NRF_RX_PIPE_5)
    {
        return NRF_ERR_FIFO_EMPTY;
    }

    status = NRF_ReadRxPayload(&app->dev, data, size);
    if (status != NRF_OK) return status;

    NRF_ClearRxDrFlag(&app->dev);
    return NRF_OK;
}

NRF_Status_t NRF_App_ReadPacket(NRF_App_t *app, uint8_t *data, uint8_t *actual_size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;
    if (data == NULL || actual_size == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t payload_len = 0;

    uint8_t available = 0;
    status = NRF_App_DataAvailable(app, &available);
    if (status != NRF_OK || !available) return NRF_ERR_FIFO_EMPTY;

    if (app->dev.feature_mask & NRF_FEATURE_EN_DPL)
    {
        status = NRF_ReadRxPayloadWidth(&app->dev, &payload_len);
        if (status != NRF_OK) return status;

        if (payload_len > 32U)
        {
            NRF_FlushRx(&app->dev);
            NRF_ClearRxDrFlag(&app->dev);
            return NRF_ERR_FIFO_EMPTY;
        }
    }
    else
    {
        NRF_RxPipeStatus_t pipe;
        status = NRF_GetRxPipeNumber(&app->dev, &pipe);
        if (status != NRF_OK || pipe > NRF_RX_PIPE_5) return NRF_ERR_FIFO_EMPTY;

        status = NRF_App_GetPipePayloadSize(app, (uint8_t)pipe, &payload_len);
        if (status != NRF_OK) return status;
    }

    if (payload_len == 0U) return NRF_ERR_FIFO_EMPTY;

    status = NRF_ReadRxPayload(&app->dev, data, payload_len);
    if (status != NRF_OK) return status;

    *actual_size = payload_len;

    NRF_ClearRxDrFlag(&app->dev);
    return NRF_OK;
}

/* ============================================================
 * Pipe & Address Management
 * ============================================================ */
NRF_Status_t NRF_App_EnablePipe(NRF_App_t *app, uint8_t pipe)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    status = NRF_GetRxAddrBitmask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_PipeMask_t)(mask | (NRF_PipeMask_t)(1U << pipe));
    return NRF_SetRxAddrBitmask(&app->dev, mask);
}

NRF_Status_t NRF_App_DisablePipe(NRF_App_t *app, uint8_t pipe)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    status = NRF_GetRxAddrBitmask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_PipeMask_t)(mask & (NRF_PipeMask_t)~(1U << pipe));
    return NRF_SetRxAddrBitmask(&app->dev, mask);
}

NRF_Status_t NRF_App_EnableAutoAck(NRF_App_t *app, uint8_t pipe)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    status = NRF_GetAutoAckBitmask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_PipeMask_t)(mask | (NRF_PipeMask_t)(1U << pipe));
    return NRF_SetAutoAckBitmask(&app->dev, mask);
}

NRF_Status_t NRF_App_DisableAutoAck(NRF_App_t *app, uint8_t pipe)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    status = NRF_GetAutoAckBitmask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_PipeMask_t)(mask & (NRF_PipeMask_t)~(1U << pipe));
    return NRF_SetAutoAckBitmask(&app->dev, mask);
}

NRF_Status_t NRF_App_SetPipePayloadSize(NRF_App_t *app, uint8_t pipe, uint8_t size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    if (size > 32U) return NRF_ERR_INVALID_PARAM;

    switch (pipe)
    {
        case 0U: return NRF_SetRxPwP0(&app->dev, size);
        case 1U: return NRF_SetRxPwP1(&app->dev, size);
        case 2U: return NRF_SetRxPwP2(&app->dev, size);
        case 3U: return NRF_SetRxPwP3(&app->dev, size);
        case 4U: return NRF_SetRxPwP4(&app->dev, size);
        case 5U: return NRF_SetRxPwP5(&app->dev, size);
        default: return NRF_ERR_INVALID_PARAM;
    }
}

NRF_Status_t NRF_App_GetPipePayloadSize(NRF_App_t *app, uint8_t pipe, uint8_t *size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    if (size == NULL) return NRF_ERR_INVALID_PARAM;

    switch (pipe)
    {
        case 0U: return NRF_GetRxPwP0(&app->dev, size);
        case 1U: return NRF_GetRxPwP1(&app->dev, size);
        case 2U: return NRF_GetRxPwP2(&app->dev, size);
        case 3U: return NRF_GetRxPwP3(&app->dev, size);
        case 4U: return NRF_GetRxPwP4(&app->dev, size);
        case 5U: return NRF_GetRxPwP5(&app->dev, size);
        default: return NRF_ERR_INVALID_PARAM;
    }
}

NRF_Status_t NRF_App_SetRxAddress(NRF_App_t *app, uint8_t pipe, const uint8_t *address, uint8_t size)
{
    uint8_t address_width;
    NRF_RxAddrP0_t addr_p0;
    NRF_RxAddrP1_t addr_p1;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    if (address == NULL) return NRF_ERR_INVALID_PARAM;

    if (pipe == 0U || pipe == 1U)
    {
        status = NRF_App_GetAddressWidthBytes(app, &address_width);
        if (status != NRF_OK) return status;

        if (size != address_width) return NRF_ERR_INVALID_PARAM;

        if (pipe == 0U)
        {
            memset(&addr_p0, 0, sizeof(addr_p0));
            memcpy(addr_p0.bytes, address, address_width);
            return NRF_SetRxAddrP0(&app->dev, &addr_p0);
        }

        memset(&addr_p1, 0, sizeof(addr_p1));
        memcpy(addr_p1.bytes, address, address_width);
        return NRF_SetRxAddrP1(&app->dev, &addr_p1);
    }

    if (size != 1U) return NRF_ERR_INVALID_PARAM;

    switch (pipe)
    {
        case 2U: return NRF_SetRxAddrP2(&app->dev, address[0]);
        case 3U: return NRF_SetRxAddrP3(&app->dev, address[0]);
        case 4U: return NRF_SetRxAddrP4(&app->dev, address[0]);
        case 5U: return NRF_SetRxAddrP5(&app->dev, address[0]);
        default: return NRF_ERR_INVALID_PARAM;
    }
}

NRF_Status_t NRF_App_GetRxAddress(NRF_App_t *app, uint8_t pipe, uint8_t *address, uint8_t size)
{
    uint8_t address_width;
    NRF_RxAddrP0_t addr_p0;
    NRF_RxAddrP1_t addr_p1;
    uint8_t addr_value;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    if (address == NULL) return NRF_ERR_INVALID_PARAM;

    if (pipe == 0U || pipe == 1U)
    {
        status = NRF_App_GetAddressWidthBytes(app, &address_width);
        if (status != NRF_OK) return status;

        if (size != address_width) return NRF_ERR_INVALID_PARAM;

        if (pipe == 0U)
        {
            status = NRF_GetRxAddrP0(&app->dev, &addr_p0);
            if (status != NRF_OK) return status;
            memcpy(address, addr_p0.bytes, address_width);
            return NRF_OK;
        }

        status = NRF_GetRxAddrP1(&app->dev, &addr_p1);
        if (status != NRF_OK) return status;
        memcpy(address, addr_p1.bytes, address_width);
        return NRF_OK;
    }

    if (size != 1U) return NRF_ERR_INVALID_PARAM;

    switch (pipe)
    {
        case 2U: status = NRF_GetRxAddrP2(&app->dev, &addr_value); break;
        case 3U: status = NRF_GetRxAddrP3(&app->dev, &addr_value); break;
        case 4U: status = NRF_GetRxAddrP4(&app->dev, &addr_value); break;
        case 5U: status = NRF_GetRxAddrP5(&app->dev, &addr_value); break;
        default: return NRF_ERR_INVALID_PARAM;
    }

    if (status != NRF_OK) return status;

    address[0] = addr_value;
    return NRF_OK;
}

NRF_Status_t NRF_App_SetTxAddress(NRF_App_t *app, const uint8_t *address, uint8_t size)
{
    uint8_t address_width;
    NRF_TxAddr_t tx_addr;
    NRF_RxAddrP0_t rx_addr_p0;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (address == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_App_GetAddressWidthBytes(app, &address_width);
    if (status != NRF_OK) return status;

    if (size != address_width) return NRF_ERR_INVALID_PARAM;

    memset(&tx_addr, 0, sizeof(tx_addr));
    memcpy(tx_addr.bytes, address, address_width);
    status = NRF_SetTxAddr(&app->dev, &tx_addr);
    if (status != NRF_OK) return status;

    memset(&rx_addr_p0, 0, sizeof(rx_addr_p0));
    memcpy(rx_addr_p0.bytes, address, address_width);
    return NRF_SetRxAddrP0(&app->dev, &rx_addr_p0);
}

NRF_Status_t NRF_App_GetTxAddress(NRF_App_t *app, uint8_t *address, uint8_t size)
{
    uint8_t address_width;
    NRF_TxAddr_t tx_addr;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (address == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_App_GetAddressWidthBytes(app, &address_width);
    if (status != NRF_OK) return status;

    if (size != address_width) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetTxAddr(&app->dev, &tx_addr);
    if (status != NRF_OK) return status;

    memcpy(address, tx_addr.bytes, address_width);
    return NRF_OK;
}

/* ============================================================
 * Dynamic Payload & Features
 * ============================================================ */
NRF_Status_t NRF_App_EnableDynamicPayload(NRF_App_t *app, uint8_t pipe)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    status = NRF_GetDynamicPayloadMask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_PipeMask_t)(mask | (NRF_PipeMask_t)(1U << pipe));
    status = NRF_SetDynamicPayloadMask(&app->dev, mask);
    if (status != NRF_OK) return status;

    return NRF_App_EnableFeature(app, NRF_FEATURE_EN_DPL);
}

NRF_Status_t NRF_App_DisableDynamicPayload(NRF_App_t *app, uint8_t pipe)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    status = NRF_GetDynamicPayloadMask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_PipeMask_t)(mask & (NRF_PipeMask_t)~(1U << pipe));
    return NRF_SetDynamicPayloadMask(&app->dev, mask);
}

NRF_Status_t NRF_App_IsDynamicPayloadEnabled(NRF_App_t *app, uint8_t pipe, uint8_t *enabled)
{
    NRF_PipeMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    if (enabled == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetDynamicPayloadMask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    *enabled = ((mask & (NRF_PipeMask_t)(1U << pipe)) != 0U) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_EnableFeature(NRF_App_t *app, NRF_FeatureMask_t feature)
{
    NRF_FeatureMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (feature == NRF_FEATURE_NONE) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetFeatureMask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_FeatureMask_t)(mask | feature);
    return NRF_SetFeatureMask(&app->dev, mask);
}

NRF_Status_t NRF_App_DisableFeature(NRF_App_t *app, NRF_FeatureMask_t feature)
{
    NRF_FeatureMask_t mask;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (feature == NRF_FEATURE_NONE) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetFeatureMask(&app->dev, &mask);
    if (status != NRF_OK) return status;

    mask = (NRF_FeatureMask_t)(mask & (NRF_FeatureMask_t)~feature);
    return NRF_SetFeatureMask(&app->dev, mask);
}

NRF_Status_t NRF_App_GetFeatures(NRF_App_t *app, NRF_FeatureMask_t *features)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (features == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetFeatureMask(&app->dev, features);
}

/* ============================================================
 * FIFO & Event Diagnostics
 * ============================================================ */
NRF_Status_t NRF_App_FlushTx(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_FlushTx(&app->dev);
}

NRF_Status_t NRF_App_FlushRx(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_FlushRx(&app->dev);
}

NRF_Status_t NRF_App_ResetFifo(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_FlushRx(app);
    if (status != NRF_OK) return status;

    return NRF_App_FlushTx(app);
}

NRF_Status_t NRF_App_IsTxEmpty(NRF_App_t *app, uint8_t *empty)
{
    NRF_TxFifoEmpty_t fifo_empty;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (empty == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetTxFifoEmptyStatus(&app->dev, &fifo_empty);
    if (status != NRF_OK) return status;

    *empty = (fifo_empty == NRF_TX_FIFO_EMPTY) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_IsRxEmpty(NRF_App_t *app, uint8_t *empty)
{
    NRF_RxFifoEmpty_t fifo_empty;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (empty == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetRxFifoEmptyStatus(&app->dev, &fifo_empty);
    if (status != NRF_OK) return status;

    *empty = (fifo_empty == NRF_RX_FIFO_EMPTY) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_IsTxFull(NRF_App_t *app, uint8_t *full)
{
    NRF_TxFullStatus_t fifo_full;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (full == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetTxFifoFullStatus(&app->dev, &fifo_full);
    if (status != NRF_OK) return status;

    *full = (fifo_full == NRF_TX_FIFO_FULL) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_IsRxFull(NRF_App_t *app, uint8_t *full)
{
    NRF_RxFifoFull_t fifo_full;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (full == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetRxFifoFullStatus(&app->dev, &fifo_full);
    if (status != NRF_OK) return status;

    *full = (fifo_full == NRF_RX_FIFO_FULL) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_IsTxReuseActive(NRF_App_t *app, uint8_t *active)
{
    NRF_TxReuseStatus_t reuse_status;
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (active == NULL) return NRF_ERR_INVALID_PARAM;

    status = NRF_GetTxReuseStatus(&app->dev, &reuse_status);
    if (status != NRF_OK) return status;

    *active = (reuse_status == NRF_TX_REUSE_ACTIVE) ? 1U : 0U;
    return NRF_OK;
}

NRF_Status_t NRF_App_GetRxStatus(NRF_App_t *app, NRF_RxDrStatus_t *status_value)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (status_value == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetRxDrStatus(&app->dev, status_value);
}

NRF_Status_t NRF_App_GetTxStatus(NRF_App_t *app, NRF_TxDsStatus_t *status_value)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (status_value == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetTxDsStatus(&app->dev, status_value);
}

NRF_Status_t NRF_App_GetMaxRtStatus(NRF_App_t *app, NRF_MaxRtStatus_t *status_value)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (status_value == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetMaxRtStatus(&app->dev, status_value);
}

NRF_Status_t NRF_App_ClearRxEvent(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_ClearRxDrFlag(&app->dev);
}

NRF_Status_t NRF_App_ClearTxEvent(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_ClearTxDsFlag(&app->dev);
}

NRF_Status_t NRF_App_ClearMaxRtEvent(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_ClearMaxRtFlag(&app->dev);
}

NRF_Status_t NRF_App_GetRxPipe(NRF_App_t *app, NRF_RxPipeStatus_t *pipe)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (pipe == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetRxPipeNumber(&app->dev, pipe);
}

NRF_Status_t NRF_App_GetPacketLoss(NRF_App_t *app, NRF_PacketLossCount_t *count)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (count == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetPacketLossCount(&app->dev, count);
}

NRF_Status_t NRF_App_GetRetryCount(NRF_App_t *app, NRF_RetransmitCount_t *count)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (count == NULL) return NRF_ERR_INVALID_PARAM;

    return NRF_GetRetransmitCount(&app->dev, count);
}

NRF_Status_t NRF_App_SetCE(NRF_App_t *app, NRF_PinState_t state)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_SetCEState(&app->dev, state);
}

/* ============================================================
 * Advanced ESB Features Implementation
 * ============================================================ */

NRF_Status_t NRF_App_LoadAckPayload(NRF_App_t *app, uint8_t pipe, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_App_CheckPipe(pipe);
    if (status != NRF_OK) return status;

    if (data == NULL || size == 0U || size > 32U) return NRF_ERR_INVALID_PARAM;

    if (!(app->dev.feature_mask & NRF_FEATURE_EN_ACK_PAY))
    {
        status = NRF_App_EnableFeature(app, NRF_FEATURE_EN_ACK_PAY);
        if (status != NRF_OK) return status;
    }

    NRF_Hw_Status_t hw_status = NRF_HW_WriteAckPayload(&app->dev.hw, pipe, data, size);
    if (hw_status != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    return NRF_OK;
}

NRF_Status_t NRF_App_TransmitNoAck(NRF_App_t *app, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    if (data == NULL || size == 0U || size > 32U)
    {
        return NRF_ERR_INVALID_PARAM;
    }

    if (!(app->dev.feature_mask & NRF_FEATURE_EN_DYN_ACK))
    {
        status = NRF_App_EnableFeature(app, NRF_FEATURE_EN_DYN_ACK);
        if (status != NRF_OK) return status;
    }

    NRF_Role_t original_role = app->dev.role;
    if (original_role == NRF_ROLE_PRX)
    {
        NRF_SetCEState(&app->dev, NRF_PIN_RESET);
        NRF_SetRole(&app->dev, NRF_ROLE_PTX);
        NRF_delay_us(&app->dev.hw.bus_dev, 130);
    }

    NRF_Hw_Status_t hw_status = NRF_HW_WritePayloadNoAck(&app->dev.hw, data, size);
    if (hw_status != NRF_HW_OK)
    {
        if (original_role == NRF_ROLE_PRX)
        {
            NRF_SetRole(&app->dev, NRF_ROLE_PRX);
            NRF_SetCEState(&app->dev, NRF_PIN_SET);
        }
        return NRF_ERR_SPI_COMM;
    }

    NRF_SetCEState(&app->dev, NRF_PIN_SET);
    NRF_delay_us(&app->dev.hw.bus_dev, 10);
    NRF_SetCEState(&app->dev, NRF_PIN_RESET);

    if (original_role == NRF_ROLE_PRX)
    {
        NRF_SetRole(&app->dev, NRF_ROLE_PRX);
        NRF_SetCEState(&app->dev, NRF_PIN_SET);
    }

    return NRF_OK;
}

NRF_Status_t NRF_App_ScanChannel(NRF_App_t *app, uint8_t channel, uint8_t *is_occupied)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK || is_occupied == NULL || channel > 125U) return NRF_ERR_INVALID_PARAM;

    uint8_t orig_channel = app->dev.rf_channel;
    NRF_Role_t orig_role = app->dev.role;

    NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    NRF_SetChannel(&app->dev, channel);
    NRF_SetRole(&app->dev, NRF_ROLE_PRX);
    NRF_SetPowerMode(&app->dev, NRF_POWER_UP);
    NRF_SetCEState(&app->dev, NRF_PIN_SET);

    NRF_delay_us(&app->dev.hw.bus_dev, 270);

    NRF_CarrierDetect_t cd;
    status = NRF_GetCarrierDetect(&app->dev, &cd);
    *is_occupied = (cd == NRF_CD_DETECTED) ? 1U : 0U;

    NRF_SetCEState(&app->dev, NRF_PIN_RESET);
    NRF_SetChannel(&app->dev, orig_channel);
    NRF_SetRole(&app->dev, orig_role);
    if (orig_role == NRF_ROLE_PRX)
    {
        NRF_SetCEState(&app->dev, NRF_PIN_SET);
    }

    return status;
}

NRF_Status_t NRF_App_ResetPacketLoss(NRF_App_t *app)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    return NRF_SetChannel(&app->dev, app->dev.rf_channel);
}

NRF_Status_t NRF_App_PollEvents(NRF_App_t *app, NRF_App_Event_t *events)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK || events == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t reg_status;
    if (NRF_GetStatus(&app->dev.hw, &reg_status) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    events->rx_data_ready = (reg_status & (1U << NRF_BIT_RX_DR)) ? 1U : 0U;
    events->tx_data_sent  = (reg_status & (1U << NRF_BIT_TX_DS)) ? 1U : 0U;
    events->tx_max_retry  = (reg_status & (1U << NRF_BIT_MAX_RT)) ? 1U : 0U;
    events->rx_pipe       = (reg_status & NRF_MASK_RX_P_NO) >> NRF_SHIFT_RX_P_NO;

    uint8_t clear_mask = reg_status & ((1U << NRF_BIT_RX_DR) | (1U << NRF_BIT_TX_DS) | (1U << NRF_BIT_MAX_RT));
    if (clear_mask)
    {
        NRF_WriteRegister(&app->dev.hw, NRF_REG_STATUS, clear_mask);
    }

    return NRF_OK;
}

/* ============================================================
 * IRQ & Interrupt Handling Implementation
 * ============================================================ */

static void NRF_App_Internal_ISR(void *arg)
{
    NRF_App_t *app = (NRF_App_t *)arg;
    if (app != NULL)
    {
        NRF_App_IRQHandler(app);
    }
}

void NRF_App_IRQHandler(NRF_App_t *app)
{
    if (app == NULL) return;

    uint8_t reg_status = 0;
    if (NRF_GetStatus(&app->dev.hw, &reg_status) != NRF_HW_OK) return;

    NRF_App_Event_t event = {0};

    event.rx_data_ready = (reg_status & (1U << NRF_BIT_RX_DR)) ? 1U : 0U;
    event.tx_data_sent  = (reg_status & (1U << NRF_BIT_TX_DS)) ? 1U : 0U;
    event.tx_max_retry  = (reg_status & (1U << NRF_BIT_MAX_RT)) ? 1U : 0U;
    event.rx_pipe       = (reg_status & NRF_MASK_RX_P_NO) >> NRF_SHIFT_RX_P_NO;

    uint8_t clear_mask = reg_status & ((1U << NRF_BIT_RX_DR) | (1U << NRF_BIT_TX_DS) | (1U << NRF_BIT_MAX_RT));
    if (clear_mask)
    {
        NRF_WriteRegister(&app->dev.hw, NRF_REG_STATUS, clear_mask);
    }

    if (event.tx_max_retry)
    {
        NRF_FlushTX(&app->dev.hw);
    }

    if (app->event_callback != NULL)
    {
        app->event_callback(app, &event, app->callback_user_arg);
    }
}

NRF_Status_t NRF_App_SetEventCallback(NRF_App_t *app, NRF_App_EventCallback_t callback, void *user_arg)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    app->event_callback    = callback;
    app->callback_user_arg = user_arg;

    if (callback != NULL)
    {
        NRF_Hw_Status_t hw_status = NRF_SetIRQCallback(&app->dev.hw, NRF_App_Internal_ISR, app);
        if (hw_status != NRF_HW_OK) return NRF_ERR_GPIO;
    }
    else
    {
        NRF_SetIRQCallback(&app->dev.hw, NULL, NULL);
    }

    return NRF_OK;
}

NRF_Status_t NRF_App_ConfigureInterrupts(NRF_App_t *app, NRF_RxDrMask_t rx_dr, NRF_TxDsMask_t tx_ds, NRF_MaxRtMask_t max_rt)
{
    NRF_Status_t status = NRF_App_Check(app);
    if (status != NRF_OK) return status;

    status = NRF_SetRxDrMask(&app->dev, rx_dr);
    if (status != NRF_OK) return status;

    status = NRF_SetTxDsMask(&app->dev, tx_ds);
    if (status != NRF_OK) return status;

    return NRF_SetMaxRtMask(&app->dev, max_rt);
}
