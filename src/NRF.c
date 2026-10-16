/*
 * nrf.c
 *
 *  Created on: Sep 25, 2026
 *      Author: amire
 */

#include "nrf.h"
#include <string.h>

/* ============================================================
 * Internal Helpers
 * ============================================================ */
static NRF_Status_t NRF_MapHwStatus(NRF_Hw_Status_t hw_status)
{
    switch (hw_status)
    {
        case NRF_HW_OK:                return NRF_OK;
        case NRF_HW_ERR_INVALID_PARAM: return NRF_ERR_INVALID_PARAM;
        case NRF_HW_ERR_INVALID_SIZE:  return NRF_ERR_INVALID_PARAM;
        case NRF_HW_ERR_GPIO:          return NRF_ERR_GPIO;
        case NRF_HW_ERR_TIMER:         return NRF_ERR_TIMER;
        case NRF_HW_ERR_SPI:           return NRF_ERR_SPI_COMM;
        case NRF_HW_ERR_TIMEOUT:       return NRF_ERR_TIMEOUT;
        case NRF_HW_ERR_BUSY:          return NRF_ERR_BUSY;
        default:                       return NRF_ERR_SPI_COMM;
    }
}

static uint8_t NRF_GetAddressWidthBytes(NRF_AddrWidth_t width)
{
    switch (width)
    {
        case NRF_AW_3BYTES: return 3U;
        case NRF_AW_4BYTES: return 4U;
        case NRF_AW_5BYTES: return 5U;
        default:            return 5U;
    }
}

static NRF_Status_t NRF_CheckDevice(NRF_Device_t *dev)
{
    if (dev == NULL)
    {
        return NRF_ERR_INVALID_HANDLE;
    }
    return NRF_OK;
}

/* ============================================================
 * Initialization & Teardown
 * ============================================================ */
NRF_Status_t NRF_Init(NRF_Device_t *dev)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    NRF_Hw_Status_t hw_res = NRF_HW_Init(&dev->hw);
    if (hw_res != NRF_HW_OK)
    {
        return NRF_MapHwStatus(hw_res);
    }

    uint8_t aw_check = 0;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_SETUP_AW, &aw_check) != NRF_HW_OK)
    {
        return NRF_ERR_SPI_COMM;
    }

    uint8_t aw_val = aw_check & 0x03U;
    if (aw_val < 1U || aw_val > 3U)
    {
        return NRF_ERR_SPI_COMM;
    }

    uint8_t clear_status = (1U << NRF_BIT_RX_DR) | (1U << NRF_BIT_TX_DS) | (1U << NRF_BIT_MAX_RT);
    NRF_WriteRegister(&dev->hw, NRF_REG_STATUS, clear_status);

    NRF_FlushTX(&dev->hw);
    NRF_FlushRX(&dev->hw);

    dev->feature_access_enabled = 0;

    return NRF_OK;
}

NRF_Status_t NRF_DeInit(NRF_Device_t *dev)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    return NRF_MapHwStatus(NRF_HW_DeInit(&dev->hw));
}

/* ============================================================
 * Configuration Register Access
 * ============================================================ */
NRF_Status_t NRF_SetRxDrMask(NRF_Device_t *dev, NRF_RxDrMask_t mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (mask == NRF_RX_DR_INT_MASKED)
        val |= (1U << NRF_BIT_MASK_RX_DR);
    else
        val &= ~(1U << NRF_BIT_MASK_RX_DR);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_dr_mask = mask;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxDrMask(NRF_Device_t *dev, NRF_RxDrMask_t *mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || mask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *mask = (val & (1U << NRF_BIT_MASK_RX_DR)) ? NRF_RX_DR_INT_MASKED : NRF_RX_DR_INT_ENABLED;
    dev->rx_dr_mask = *mask;
    return NRF_OK;
}

NRF_Status_t NRF_SetTxDsMask(NRF_Device_t *dev, NRF_TxDsMask_t mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (mask == NRF_TX_DS_INT_MASKED)
        val |= (1U << NRF_BIT_MASK_TX_DS);
    else
        val &= ~(1U << NRF_BIT_MASK_TX_DS);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->tx_ds_mask = mask;
    return NRF_OK;
}
NRF_Status_t NRF_GetTxDsMask(NRF_Device_t *dev, NRF_TxDsMask_t *mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || mask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *mask = (val & (1U << NRF_BIT_MASK_TX_DS)) ? NRF_TX_DS_INT_MASKED : NRF_TX_DS_INT_ENABLED;
    dev->tx_ds_mask = *mask;
    return NRF_OK;
}

NRF_Status_t NRF_SetMaxRtMask(NRF_Device_t *dev, NRF_MaxRtMask_t mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (mask == NRF_MAX_RT_INT_MASKED)
        val |= (1U << NRF_BIT_MASK_MAX_RT);
    else
        val &= ~(1U << NRF_BIT_MASK_MAX_RT);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->max_rt_mask = mask;
    return NRF_OK;
}
NRF_Status_t NRF_GetMaxRtMask(NRF_Device_t *dev, NRF_MaxRtMask_t *mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || mask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *mask = (val & (1U << NRF_BIT_MASK_MAX_RT)) ? NRF_MAX_RT_INT_MASKED : NRF_MAX_RT_INT_ENABLED;
    dev->max_rt_mask = *mask;
    return NRF_OK;
}

NRF_Status_t NRF_SetCrcState(NRF_Device_t *dev, NRF_CrcState_t state)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (state == NRF_CRC_ENABLED)
        val |= (1U << NRF_BIT_EN_CRC);
    else
        val &= ~(1U << NRF_BIT_EN_CRC);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->crc_state = state;
    return NRF_OK;
}
NRF_Status_t NRF_GetCrcState(NRF_Device_t *dev, NRF_CrcState_t *state)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || state == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *state = (val & (1U << NRF_BIT_EN_CRC)) ? NRF_CRC_ENABLED : NRF_CRC_DISABLED;
    dev->crc_state = *state;
    return NRF_OK;
}

NRF_Status_t NRF_SetCrcLength(NRF_Device_t *dev, NRF_CrcLength_t length)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (length == NRF_CRC_LEN_2_BYTES)
        val |= (1U << NRF_BIT_CRCO);
    else
        val &= ~(1U << NRF_BIT_CRCO);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->crc_length = length;
    return NRF_OK;
}
NRF_Status_t NRF_GetCrcLength(NRF_Device_t *dev, NRF_CrcLength_t *length)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || length == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *length = (val & (1U << NRF_BIT_CRCO)) ? NRF_CRC_LEN_2_BYTES : NRF_CRC_LEN_1_BYTE;
    dev->crc_length = *length;
    return NRF_OK;
}

NRF_Status_t NRF_SetPowerMode(NRF_Device_t *dev, NRF_PowerMode_t mode)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (mode == NRF_POWER_UP)
        val |= (1U << NRF_BIT_PWR_UP);
    else
        val &= ~(1U << NRF_BIT_PWR_UP);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    dev->power_mode = mode;

    if (mode == NRF_POWER_UP)
    {
        NRF_delay(&dev->hw.bus_dev, 2);
    }

    return NRF_OK;
}
NRF_Status_t NRF_GetPowerMode(NRF_Device_t *dev, NRF_PowerMode_t *mode)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || mode == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *mode = (val & (1U << NRF_BIT_PWR_UP)) ? NRF_POWER_UP : NRF_POWER_DOWN;
    dev->power_mode = *mode;
    return NRF_OK;
}

NRF_Status_t NRF_SetRole(NRF_Device_t *dev, NRF_Role_t role)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (role == NRF_ROLE_PRX)
        val |= (1U << NRF_BIT_PRIM_RX);
    else
        val &= ~(1U << NRF_BIT_PRIM_RX);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_CONFIG, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->role = role;
    return NRF_OK;
}
NRF_Status_t NRF_GetRole(NRF_Device_t *dev, NRF_Role_t *role)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || role == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_CONFIG, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *role = (val & (1U << NRF_BIT_PRIM_RX)) ? NRF_ROLE_PRX : NRF_ROLE_PTX;
    dev->role = *role;
    return NRF_OK;
}

NRF_Status_t NRF_SetAutoAckBitmask(NRF_Device_t *dev, NRF_PipeMask_t pipe_bitmask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_EN_AA, (uint8_t)(pipe_bitmask & NRF_PIPE_ALL)) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    dev->auto_ack_mask = pipe_bitmask;
    return NRF_OK;
}
NRF_Status_t NRF_GetAutoAckBitmask(NRF_Device_t *dev, NRF_PipeMask_t *pipe_bitmask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pipe_bitmask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_EN_AA, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *pipe_bitmask = (NRF_PipeMask_t)(val & NRF_PIPE_ALL);
    dev->auto_ack_mask = *pipe_bitmask;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxAddrBitmask(NRF_Device_t *dev, NRF_PipeMask_t pipe_bitmask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_EN_RXADDR, (uint8_t)(pipe_bitmask & NRF_PIPE_ALL)) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    dev->rx_addr_mask = pipe_bitmask;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrBitmask(NRF_Device_t *dev, NRF_PipeMask_t *pipe_bitmask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pipe_bitmask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_EN_RXADDR, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *pipe_bitmask = (NRF_PipeMask_t)(val & NRF_PIPE_ALL);
    dev->rx_addr_mask = *pipe_bitmask;
    return NRF_OK;
}

NRF_Status_t NRF_SetAddressWidth(NRF_Device_t *dev, NRF_AddrWidth_t width)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (width < NRF_AW_3BYTES || width > NRF_AW_5BYTES) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_SETUP_AW, (uint8_t)width) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    dev->addr_width = width;
    return NRF_OK;
}
NRF_Status_t NRF_GetAddressWidth(NRF_Device_t *dev, NRF_AddrWidth_t *width)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || width == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_SETUP_AW, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *width = (NRF_AddrWidth_t)(val & 0x03U);
    dev->addr_width = *width;
    return NRF_OK;
}

NRF_Status_t NRF_SetAutoRetransmit(NRF_Device_t *dev, NRF_RetransmitDelay_t delay, uint8_t count)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (count > 15U) return NRF_ERR_INVALID_PARAM;

    uint8_t val = (uint8_t)(((uint8_t)delay << 4U) | (count & 0x0FU));
    if (NRF_WriteRegister(&dev->hw, NRF_REG_SETUP_RETR, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    dev->retr_delay = delay;
    dev->retr_count = count;
    return NRF_OK;
}
NRF_Status_t NRF_GetAutoRetransmit(NRF_Device_t *dev, NRF_RetransmitDelay_t *delay, uint8_t *count)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || delay == NULL || count == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_SETUP_RETR, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *delay = (NRF_RetransmitDelay_t)((val >> 4U) & 0x0FU);
    *count = (val & 0x0FU);
    dev->retr_delay = *delay;
    dev->retr_count = *count;
    return NRF_OK;
}

NRF_Status_t NRF_SetChannel(NRF_Device_t *dev, uint8_t channel)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (channel > 125U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RF_CH, channel & 0x7FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    dev->rf_channel = channel;
    return NRF_OK;
}
NRF_Status_t NRF_GetChannel(NRF_Device_t *dev, uint8_t *channel)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || channel == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RF_CH, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *channel = (val & 0x7FU);
    dev->rf_channel = *channel;
    return NRF_OK;
}

NRF_Status_t NRF_SetDataRate(NRF_Device_t *dev, NRF_DataRate_t rate)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RF_SETUP, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    val &= ~((1U << NRF_BIT_RF_DR_LOW) | (1U << NRF_BIT_RF_DR_HIGH));

    if (rate == NRF_BR_1MBPS)
    {

    }
    else if (rate == NRF_BR_2MBPS)
    {
        val |= (1U << NRF_BIT_RF_DR_HIGH);
    }
    else if (rate == NRF_BR_250KBPS)
    {
        val |= (1U << NRF_BIT_RF_DR_LOW);
    }
    else
    {
        return NRF_ERR_INVALID_PARAM;
    }

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RF_SETUP, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    dev->data_rate = rate;
    return NRF_OK;
}
NRF_Status_t NRF_GetDataRate(NRF_Device_t *dev, NRF_DataRate_t *rate)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || rate == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RF_SETUP, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (val & (1U << NRF_BIT_RF_DR_LOW))
    {
        *rate = NRF_BR_250KBPS;
    }
    else if (val & (1U << NRF_BIT_RF_DR_HIGH))
    {
        *rate = NRF_BR_2MBPS;
    }
    else
    {
        *rate = NRF_BR_1MBPS;
    }

    dev->data_rate = *rate;
    return NRF_OK;
}

NRF_Status_t NRF_SetOutputPower(NRF_Device_t *dev, NRF_OutputPower_t pwr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RF_SETUP, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    val &= ~(0x03U << 1U);
    val |= (uint8_t)(((uint8_t)pwr & 0x03U) << 1U);

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RF_SETUP, val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    dev->rf_pwr = pwr;
    return NRF_OK;
}
NRF_Status_t NRF_GetOutputPower(NRF_Device_t *dev, NRF_OutputPower_t *pwr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pwr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RF_SETUP, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *pwr = (NRF_OutputPower_t)((val >> 1U) & 0x03U);
    dev->rf_pwr = *pwr;
    return NRF_OK;
}

/* ============================================================
 * Live Hardware Status Queries
 * ============================================================ */
NRF_Status_t NRF_GetRxDrStatus(NRF_Device_t *dev, NRF_RxDrStatus_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t reg_status;
    if (NRF_GetStatus(&dev->hw, &reg_status) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (reg_status & (1U << NRF_BIT_RX_DR)) ? NRF_RX_DR_ACTIVE : NRF_RX_DR_INACTIVE;
    return NRF_OK;
}
NRF_Status_t NRF_ClearRxDrFlag(NRF_Device_t *dev)
{
    if (dev == NULL) return NRF_ERR_INVALID_PARAM;
    return NRF_MapHwStatus(NRF_WriteRegister(&dev->hw, NRF_REG_STATUS, (1U << NRF_BIT_RX_DR)));
}

NRF_Status_t NRF_GetTxDsStatus(NRF_Device_t *dev, NRF_TxDsStatus_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t reg_status;
    if (NRF_GetStatus(&dev->hw, &reg_status) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (reg_status & (1U << NRF_BIT_TX_DS)) ? NRF_TX_DS_ACTIVE : NRF_TX_DS_INACTIVE;
    return NRF_OK;
}
NRF_Status_t NRF_ClearTxDsFlag(NRF_Device_t *dev)
{
    if (dev == NULL) return NRF_ERR_INVALID_PARAM;
    return NRF_MapHwStatus(NRF_WriteRegister(&dev->hw, NRF_REG_STATUS, (1U << NRF_BIT_TX_DS)));
}

NRF_Status_t NRF_GetMaxRtStatus(NRF_Device_t *dev, NRF_MaxRtStatus_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t reg_status;
    if (NRF_GetStatus(&dev->hw, &reg_status) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (reg_status & (1U << NRF_BIT_MAX_RT)) ? NRF_MAX_RT_ACTIVE : NRF_MAX_RT_INACTIVE;
    return NRF_OK;
}
NRF_Status_t NRF_ClearMaxRtFlag(NRF_Device_t *dev)
{
    if (dev == NULL) return NRF_ERR_INVALID_PARAM;
    return NRF_MapHwStatus(NRF_WriteRegister(&dev->hw, NRF_REG_STATUS, (1U << NRF_BIT_MAX_RT)));
}

NRF_Status_t NRF_GetRxPipeNumber(NRF_Device_t *dev, NRF_RxPipeStatus_t *pipe_num)
{
    if (dev == NULL || pipe_num == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t reg_status;
    if (NRF_GetStatus(&dev->hw, &reg_status) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *pipe_num = (NRF_RxPipeStatus_t)((reg_status & NRF_MASK_RX_P_NO) >> NRF_SHIFT_RX_P_NO);
    return NRF_OK;
}

NRF_Status_t NRF_GetTxFullStatus(NRF_Device_t *dev, NRF_TxFullStatus_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t reg_status;
    if (NRF_GetStatus(&dev->hw, &reg_status) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (reg_status & (1U << NRF_BIT_TX_FULL)) ? NRF_TX_FIFO_FULL : NRF_TX_FIFO_NOT_FULL;
    return NRF_OK;
}

NRF_Status_t NRF_GetCarrierDetect(NRF_Device_t *dev, NRF_CarrierDetect_t *detected)
{
    if (dev == NULL || detected == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RPD, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *detected = (val & 0x01U) ? NRF_CD_DETECTED : NRF_CD_NOT_DETECTED;
    return NRF_OK;
}

NRF_Status_t NRF_GetObserveTx(NRF_Device_t *dev, NRF_PacketLossCount_t *plos_cnt, NRF_RetransmitCount_t *arc_cnt)
{
    if (dev == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_OBSERVE_TX, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    if (plos_cnt != NULL) *plos_cnt = (val >> 4U) & 0x0FU;
    if (arc_cnt != NULL)  *arc_cnt  = (val & 0x0FU);
    return NRF_OK;
}
NRF_Status_t NRF_GetPacketLossCount(NRF_Device_t *dev, NRF_PacketLossCount_t *plos_cnt)
{
    return NRF_GetObserveTx(dev, plos_cnt, NULL);
}
NRF_Status_t NRF_GetRetransmitCount(NRF_Device_t *dev, NRF_RetransmitCount_t *arc_cnt)
{
    return NRF_GetObserveTx(dev, NULL, arc_cnt);
}

/* ============================================================
 * Dedicated RX & TX Addresses
 * ============================================================ */
NRF_Status_t NRF_SetRxAddrP0(NRF_Device_t *dev, const NRF_RxAddrP0_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t len = NRF_GetAddressWidthBytes(dev->addr_width);
    if (NRF_WriteMultiRegister(&dev->hw, NRF_REG_RX_ADDR_P0, addr->bytes, len) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    memcpy(&dev->rx_addr_p0, addr, sizeof(NRF_RxAddrP0_t));
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrP0(NRF_Device_t *dev, NRF_RxAddrP0_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t len = NRF_GetAddressWidthBytes(dev->addr_width);
    if (NRF_ReadMultiRegister(&dev->hw, NRF_REG_RX_ADDR_P0, addr->bytes, len) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    memcpy(&dev->rx_addr_p0, addr, sizeof(NRF_RxAddrP0_t));
    return NRF_OK;
}

NRF_Status_t NRF_SetRxAddrP1(NRF_Device_t *dev, const NRF_RxAddrP1_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t len = NRF_GetAddressWidthBytes(dev->addr_width);
    if (NRF_WriteMultiRegister(&dev->hw, NRF_REG_RX_ADDR_P1, addr->bytes, len) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    memcpy(&dev->rx_addr_p1, addr, sizeof(NRF_RxAddrP1_t));
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrP1(NRF_Device_t *dev, NRF_RxAddrP1_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t len = NRF_GetAddressWidthBytes(dev->addr_width);
    if (NRF_ReadMultiRegister(&dev->hw, NRF_REG_RX_ADDR_P1, addr->bytes, len) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    memcpy(&dev->rx_addr_p1, addr, sizeof(NRF_RxAddrP1_t));
    return NRF_OK;
}

NRF_Status_t NRF_SetRxAddrP2(NRF_Device_t *dev, NRF_RxAddrP2_t addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_ADDR_P2, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p2 = addr;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrP2(NRF_Device_t *dev, NRF_RxAddrP2_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_ADDR_P2, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p2 = *addr;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxAddrP3(NRF_Device_t *dev, NRF_RxAddrP3_t addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_ADDR_P3, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p3 = addr;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrP3(NRF_Device_t *dev, NRF_RxAddrP3_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_ADDR_P3, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p3 = *addr;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxAddrP4(NRF_Device_t *dev, NRF_RxAddrP4_t addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_ADDR_P4, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p4 = addr;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrP4(NRF_Device_t *dev, NRF_RxAddrP4_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_ADDR_P4, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p4 = *addr;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxAddrP5(NRF_Device_t *dev, NRF_RxAddrP5_t addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_ADDR_P5, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p5 = addr;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxAddrP5(NRF_Device_t *dev, NRF_RxAddrP5_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_ADDR_P5, addr) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_addr_p5 = *addr;
    return NRF_OK;
}

NRF_Status_t NRF_SetTxAddr(NRF_Device_t *dev, const NRF_TxAddr_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t len = NRF_GetAddressWidthBytes(dev->addr_width);
    if (NRF_WriteMultiRegister(&dev->hw, NRF_REG_TX_ADDR, addr->bytes, len) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    memcpy(&dev->tx_addr, addr, sizeof(NRF_TxAddr_t));
    return NRF_OK;
}
NRF_Status_t NRF_GetTxAddr(NRF_Device_t *dev, NRF_TxAddr_t *addr)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || addr == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t len = NRF_GetAddressWidthBytes(dev->addr_width);
    if (NRF_ReadMultiRegister(&dev->hw, NRF_REG_TX_ADDR, addr->bytes, len) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    memcpy(&dev->tx_addr, addr, sizeof(NRF_TxAddr_t));
    return NRF_OK;
}

/* ============================================================
 * Dedicated RX Payload Widths
 * ============================================================ */
NRF_Status_t NRF_SetRxPwP0(NRF_Device_t *dev, NRF_RxPwP0_t pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    if (pw > 32U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_PW_P0, pw & 0x3FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_pw_p0 = pw;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxPwP0(NRF_Device_t *dev, NRF_RxPwP0_t *pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pw == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_PW_P0, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    *pw = (val & 0x3FU);
    dev->rx_pw_p0 = *pw;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxPwP1(NRF_Device_t *dev, NRF_RxPwP1_t pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    if (pw > 32U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_PW_P1, pw & 0x3FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_pw_p1 = pw;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxPwP1(NRF_Device_t *dev, NRF_RxPwP1_t *pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pw == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_PW_P1, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    *pw = (val & 0x3FU);
    dev->rx_pw_p1 = *pw;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxPwP2(NRF_Device_t *dev, NRF_RxPwP2_t pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    if (pw > 32U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_PW_P2, pw & 0x3FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_pw_p2 = pw;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxPwP2(NRF_Device_t *dev, NRF_RxPwP2_t *pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pw == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_PW_P2, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    *pw = (val & 0x3FU);
    dev->rx_pw_p2 = *pw;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxPwP3(NRF_Device_t *dev, NRF_RxPwP3_t pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    if (pw > 32U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_PW_P3, pw & 0x3FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_pw_p3 = pw;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxPwP3(NRF_Device_t *dev, NRF_RxPwP3_t *pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pw == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_PW_P3, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    *pw = (val & 0x3FU);
    dev->rx_pw_p3 = *pw;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxPwP4(NRF_Device_t *dev, NRF_RxPwP4_t pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    if (pw > 32U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_PW_P4, pw & 0x3FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_pw_p4 = pw;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxPwP4(NRF_Device_t *dev, NRF_RxPwP4_t *pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pw == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_PW_P4, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    *pw = (val & 0x3FU);
    dev->rx_pw_p4 = *pw;
    return NRF_OK;
}

NRF_Status_t NRF_SetRxPwP5(NRF_Device_t *dev, NRF_RxPwP5_t pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    if (pw > 32U) return NRF_ERR_INVALID_PARAM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_RX_PW_P5, pw & 0x3FU) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    dev->rx_pw_p5 = pw;
    return NRF_OK;
}
NRF_Status_t NRF_GetRxPwP5(NRF_Device_t *dev, NRF_RxPwP5_t *pw)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || pw == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_RX_PW_P5, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
    *pw = (val & 0x3FU);
    dev->rx_pw_p5 = *pw;
    return NRF_OK;
}

/* ============================================================
 * FIFO Status Queries
 * ============================================================ */
NRF_Status_t NRF_GetRxFifoEmptyStatus(NRF_Device_t *dev, NRF_RxFifoEmpty_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_FIFO_STATUS, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (val & (1U << NRF_BIT_RX_EMPTY)) ? NRF_RX_FIFO_EMPTY : NRF_RX_FIFO_NOT_EMPTY;
    return NRF_OK;
}

NRF_Status_t NRF_GetRxFifoFullStatus(NRF_Device_t *dev, NRF_RxFifoFull_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_FIFO_STATUS, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (val & (1U << NRF_BIT_RX_FULL)) ? NRF_RX_FIFO_FULL : NRF_RX_FIFO_NOT_FULL;
    return NRF_OK;
}

NRF_Status_t NRF_GetTxFifoEmptyStatus(NRF_Device_t *dev, NRF_TxFifoEmpty_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_FIFO_STATUS, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (val & (1U << NRF_BIT_TX_EMPTY)) ? NRF_TX_FIFO_EMPTY : NRF_TX_FIFO_NOT_EMPTY;
    return NRF_OK;
}

NRF_Status_t NRF_GetTxFifoFullStatus(NRF_Device_t *dev, NRF_TxFullStatus_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_FIFO_STATUS, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (val & (1U << NRF_BIT_TX_FULL_FIFO)) ? NRF_TX_FIFO_FULL : NRF_TX_FIFO_NOT_FULL;
    return NRF_OK;
}

NRF_Status_t NRF_GetTxReuseStatus(NRF_Device_t *dev, NRF_TxReuseStatus_t *status)
{
    if (dev == NULL || status == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_FIFO_STATUS, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *status = (val & (1U << NRF_BIT_TX_REUSE)) ? NRF_TX_REUSE_ACTIVE : NRF_TX_REUSE_INACTIVE;
    return NRF_OK;
}

/* ============================================================
 * Dynamic Payload & Features
 * ============================================================ */
static NRF_Status_t NRF_UnlockFeatures(NRF_Device_t *dev)
{
    if (!dev->feature_access_enabled)
    {
        if (NRF_Activate(&dev->hw) != NRF_HW_OK) return NRF_ERR_SPI_COMM;
        dev->feature_access_enabled = 1;
    }
    return NRF_OK;
}

NRF_Status_t NRF_SetDynamicPayloadMask(NRF_Device_t *dev, NRF_PipeMask_t mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_UnlockFeatures(dev) != NRF_OK) return NRF_ERR_SPI_COMM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_DYNPD, (uint8_t)(mask & NRF_PIPE_ALL)) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    dev->dynpd_mask = mask;
    return NRF_OK;
}
NRF_Status_t NRF_GetDynamicPayloadMask(NRF_Device_t *dev, NRF_PipeMask_t *mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || mask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_DYNPD, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *mask = (NRF_PipeMask_t)(val & NRF_PIPE_ALL);
    dev->dynpd_mask = *mask;
    return NRF_OK;
}

NRF_Status_t NRF_SetFeatureMask(NRF_Device_t *dev, NRF_FeatureMask_t mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    if (NRF_UnlockFeatures(dev) != NRF_OK) return NRF_ERR_SPI_COMM;

    if (NRF_WriteRegister(&dev->hw, NRF_REG_FEATURE, (uint8_t)(mask & 0x07U)) != NRF_HW_OK)
        return NRF_ERR_SPI_COMM;

    dev->feature_mask = mask;
    return NRF_OK;
}
NRF_Status_t NRF_GetFeatureMask(NRF_Device_t *dev, NRF_FeatureMask_t *mask)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK || mask == NULL) return NRF_ERR_INVALID_PARAM;

    uint8_t val;
    if (NRF_ReadRegister(&dev->hw, NRF_REG_FEATURE, &val) != NRF_HW_OK) return NRF_ERR_SPI_COMM;

    *mask = (NRF_FeatureMask_t)(val & 0x07U);
    dev->feature_mask = *mask;
    return NRF_OK;
}

/* ============================================================
 * CE Control & Data Transfer
 * ============================================================ */
NRF_Status_t NRF_SetCEState(NRF_Device_t *dev, NRF_PinState_t state)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    NRF_SetCE(&dev->hw, state);
    return NRF_OK;
}

NRF_Status_t NRF_WriteTxPayload(NRF_Device_t *dev, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_WritePayload(&dev->hw, data, size));
}

NRF_Status_t NRF_ReadRxPayload(NRF_Device_t *dev, uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_ReadPayload(&dev->hw, data, size));
}

NRF_Status_t NRF_FlushTx(NRF_Device_t *dev)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_FlushTX(&dev->hw));
}

NRF_Status_t NRF_FlushRx(NRF_Device_t *dev)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_FlushRX(&dev->hw));
}

NRF_Status_t NRF_TransmitPayload(NRF_Device_t *dev, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;

    NRF_Hw_Status_t hw_status = NRF_WritePayload(&dev->hw, data, size);
    if (hw_status != NRF_HW_OK) return NRF_MapHwStatus(hw_status);

    NRF_SetCE(&dev->hw, NRF_PIN_SET);
    NRF_delay_us(&dev->hw.bus_dev, 10);
    NRF_SetCE(&dev->hw, NRF_PIN_RESET);

    return NRF_OK;
}

/* ============================================================
 * Advanced Commands
 * ============================================================ */
NRF_Status_t NRF_ReadRxPayloadWidth(NRF_Device_t *dev, uint8_t *width)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_HW_ReadRxPayloadWidth(&dev->hw, width));
}

NRF_Status_t NRF_WriteAckPayload(NRF_Device_t *dev, uint8_t pipe, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_HW_WriteAckPayload(&dev->hw, pipe, data, size));
}

NRF_Status_t NRF_WritePayloadNoAck(NRF_Device_t *dev, const uint8_t *data, uint8_t size)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_HW_WritePayloadNoAck(&dev->hw, data, size));
}

NRF_Status_t NRF_ReuseTxPayload(NRF_Device_t *dev)
{
    NRF_Status_t status = NRF_CheckDevice(dev);
    if (status != NRF_OK) return status;
    return NRF_MapHwStatus(NRF_HW_ReuseTxPayload(&dev->hw));
}
