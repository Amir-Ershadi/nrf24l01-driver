/*
 * NRF_Hw.c
 *
 *  Created on: Sep 24, 2026
 *      Author: amire
 */


#include "nrf_hw.h"


/* ============================================================
 * Internal Helper Functions
 * ============================================================ */


/*
 * Convert NRF_Core status to NRF_Hw status.
 *
 * This keeps the error namespaces of the two layers separated.
 */
static NRF_Hw_Status_t NRF_HW_MapCoreStatus(NRF_Core_Status_t status)
{
    switch (status)
    {
        case NRF_CORE_OK:

            return NRF_HW_OK;


        case NRF_CORE_ERR_SPI_TIMEOUT:

            return NRF_HW_ERR_TIMEOUT;


        case NRF_CORE_ERR_SPI_INIT:
        case NRF_CORE_ERR_SPI_DEINIT:
        case NRF_CORE_ERR_SPI_TRANSFER:

            return NRF_HW_ERR_SPI;


        case NRF_CORE_ERR_TIMER_INIT:
        case NRF_CORE_ERR_TIMER_DEINIT:

            return NRF_HW_ERR_TIMER;


        case NRF_CORE_ERR_GPIO_INIT:
        case NRF_CORE_ERR_GPIO_DEINIT:

            return NRF_HW_ERR_GPIO;


        case NRF_CORE_ERR_INVALID_PARAM:

        default:

            return NRF_HW_ERR_INVALID_PARAM;
    }
}
/* ============================================================
 * Register Information
 * ============================================================ */
static uint8_t NRF_HW_GetRegisterInfo(NRF_Register_t reg,uint8_t *min_size,uint8_t *max_size,uint8_t *readable,uint8_t *writable)
{
    if (min_size == NULL || max_size == NULL || readable == NULL || writable == NULL)
    {
        return 0;
    }
    *min_size = 0;
    *max_size = 0;
    *readable = 0;
    *writable = 0;
    switch (reg)
    {
        case NRF_REG_RX_ADDR_P0:
        case NRF_REG_RX_ADDR_P1:
        case NRF_REG_TX_ADDR:
            *min_size = 3;
            *max_size = 5;
            *readable = 1;
            *writable = 1;
            return 1;

        case NRF_REG_CONFIG:
        case NRF_REG_EN_AA:
        case NRF_REG_EN_RXADDR:
        case NRF_REG_SETUP_AW:
        case NRF_REG_SETUP_RETR:
        case NRF_REG_RF_CH:
        case NRF_REG_RF_SETUP:
        case NRF_REG_STATUS:

        case NRF_REG_RX_ADDR_P2:
        case NRF_REG_RX_ADDR_P3:
        case NRF_REG_RX_ADDR_P4:
        case NRF_REG_RX_ADDR_P5:

        case NRF_REG_RX_PW_P0:
        case NRF_REG_RX_PW_P1:
        case NRF_REG_RX_PW_P2:
        case NRF_REG_RX_PW_P3:
        case NRF_REG_RX_PW_P4:
        case NRF_REG_RX_PW_P5:

        case NRF_REG_DYNPD:
        case NRF_REG_FEATURE:

            *min_size = 1;
            *max_size = 1;
            *readable = 1;
            *writable = 1;

            return 1;

        case NRF_REG_OBSERVE_TX:
        case NRF_REG_RPD:
        case NRF_REG_FIFO_STATUS:

            *min_size = 1;
            *max_size = 1;
            *readable = 1;
            *writable = 0;

            return 1;


        default:

            return 0;
    }
}
/* ============================================================
 * CSN Select
 * ============================================================ */
void NRF_Select(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return;
    }
    GPIO_WritePin(&hw->csn_pin,NRF_PIN_RESET);
}
/* ============================================================
 * CSN Deselect
 * ============================================================ */
void NRF_Deselect(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return;
    }
    GPIO_WritePin(&hw->csn_pin,NRF_PIN_SET);
}
/* ============================================================
 * CE Control
 * ============================================================ */
void NRF_SetCE(NRF_Hardware_t *hw,NRF_PinState_t state)
{
    if (hw == NULL)
    {
        return;
    }
    GPIO_WritePin(&hw->ce_pin,state);
}
/* ============================================================
 * IRQ Callback
 * ============================================================ */
NRF_Hw_Status_t NRF_SetIRQCallback(NRF_Hardware_t *hw,GPIO_Callback_t callback,void *user_arg)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0
    if (hw->irq_pin.port == NULL || hw->irq_pin.pin == 0)
    {
        return NRF_HW_ERR_GPIO;
    }
#elif PLATFORM == 1
    if (hw->irq_pin.gpio_num < 0)
    {
        return NRF_HW_ERR_GPIO;
    }
#endif

    NRF_Core_Status_t status;

    status = GPIO_SetCallback(&hw->irq_pin,callback,user_arg);
    return NRF_HW_MapCoreStatus(status);
}
/* ============================================================
 * Hardware Initialization
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_Init(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    GPIO_Pin_init_t pin_init = {0};
    uint8_t ce_initialized = 0;
    uint8_t csn_initialized = 0;
    uint8_t irq_initialized = 0;
    uint8_t timer_initialized = 0;
    uint8_t spi_initialized = 0;
    NRF_Hw_Status_t init_error = NRF_HW_ERR_GPIO;

#if PLATFORM == 0

    pin_init.port = hw->ce_pin.port;
    pin_init.pin  = hw->ce_pin.pin;
    pin_init.mode = GPIO_MODE_OUTPUT_PP;
    pin_init.pull = GPIO_NOPULL;
    pin_init.speed = GPIO_SPEED_HIGH;
    pin_init.callback = NULL;
    pin_init.user_arg = NULL;

    if (GPIO_InitPin(&pin_init) != NRF_CORE_OK)
    {
        goto INIT_ERROR;
    }
    ce_initialized = 1;

    pin_init.port = hw->csn_pin.port;
    pin_init.pin  = hw->csn_pin.pin;
    pin_init.mode = GPIO_MODE_OUTPUT_PP;
    pin_init.pull = GPIO_NOPULL;
    pin_init.speed = GPIO_SPEED_HIGH;
    pin_init.callback = NULL;
    pin_init.user_arg = NULL;

    if (GPIO_InitPin(&pin_init) != NRF_CORE_OK)
    {
        goto INIT_ERROR;
    }
    csn_initialized = 1;

    if (hw->irq_pin.port != NULL && hw->irq_pin.pin != 0)
    {
        pin_init.port = hw->irq_pin.port;
        pin_init.pin  = hw->irq_pin.pin;
        pin_init.mode = GPIO_MODE_IT_FALLING;
        pin_init.pull = GPIO_PULLUP;
        pin_init.speed = GPIO_SPEED_HIGH;
        pin_init.callback = NULL;
        pin_init.user_arg = NULL;

        if (GPIO_InitPin(&pin_init) != NRF_CORE_OK)
        {
            goto INIT_ERROR;
        }
        irq_initialized = 1;
    }

#elif PLATFORM == 1

    pin_init.gpio_num = hw->ce_pin.gpio_num;
    pin_init.mode = GPIO_MODE_OUTPUT;
    pin_init.pull = GPIO_FLOATING;
    pin_init.intr_type = GPIO_INTR_DISABLE;
    pin_init.callback = NULL;
    pin_init.user_arg = NULL;
    if (GPIO_InitPin(&pin_init) != NRF_CORE_OK)
    {
        goto INIT_ERROR;
    }
    ce_initialized = 1;

    pin_init.gpio_num = hw->csn_pin.gpio_num;
    pin_init.mode = GPIO_MODE_OUTPUT;
    pin_init.pull = GPIO_FLOATING;
    pin_init.intr_type = GPIO_INTR_DISABLE;
    pin_init.callback = NULL;
    pin_init.user_arg = NULL;
    if (GPIO_InitPin(&pin_init) != NRF_CORE_OK)
    {
        goto INIT_ERROR;
    }
    csn_initialized = 1;

    if (hw->irq_pin.gpio_num >= 0)
    {
        pin_init.gpio_num = hw->irq_pin.gpio_num;
        pin_init.mode = GPIO_MODE_INPUT;
        pin_init.pull = GPIO_PULLUP_ONLY;
        pin_init.intr_type = GPIO_INTR_NEGEDGE;
        pin_init.callback = NULL;
        pin_init.user_arg = NULL;
        if (GPIO_InitPin(&pin_init) != NRF_CORE_OK)
        {
            goto INIT_ERROR;
        }
        irq_initialized = 1;
    }

#else

    return NRF_HW_ERR_INVALID_PARAM;

#endif
    /* ========================================================
     * Initial Pin State
     * ======================================================== */
    NRF_SetCE(hw,NRF_PIN_RESET);
    NRF_Deselect(hw);

    /* ========================================================
     * Timer Initialization
     * ======================================================== */
    NRF_Core_Status_t timer_status = NRF_Core_Timer_Init(&hw->bus_dev);
    if (timer_status != NRF_CORE_OK)
    {
        init_error = NRF_HW_MapCoreStatus(timer_status);
        goto INIT_ERROR;
    }
    timer_initialized = 1;

    /* ========================================================
     * SPI Initialization
     * ======================================================== */
    NRF_Core_Status_t spi_status;
    spi_status = NRF_Core_SPI_Init(&hw->bus_dev);
    if (spi_status != NRF_CORE_OK)
    {
        init_error = NRF_HW_MapCoreStatus(spi_status);
        goto INIT_ERROR;
    }
    spi_initialized = 1;
    return NRF_HW_OK;

/* ============================================================
 * Initialization Rollback
 * ============================================================ */
INIT_ERROR:
    if (csn_initialized)
    {
        NRF_Deselect(hw);
    }
    if (ce_initialized)
    {
        NRF_SetCE(hw,NRF_PIN_RESET);
    }
    if (spi_initialized)
    {
        NRF_Core_SPI_DeInit(&hw->bus_dev);
    }
    if (timer_initialized)
    {
        NRF_Core_Timer_DeInit(&hw->bus_dev);
    }
    if (irq_initialized)
    {
        GPIO_DeInitPin(&hw->irq_pin);
    }
    if (csn_initialized)
    {
        GPIO_DeInitPin(&hw->csn_pin);
    }
    if (ce_initialized)
    {
        GPIO_DeInitPin(&hw->ce_pin);
    }
    return init_error;
}
/* ============================================================
 * Hardware DeInitialization
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_DeInit(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    NRF_Hw_Status_t result = NRF_HW_OK;
    NRF_Core_Status_t status;

#if PLATFORM == 0
    if (hw->ce_pin.pin != 0 || hw->ce_pin.port != NULL)
#elif PLATFORM == 1
    if (hw->ce_pin.gpio_num >= 0)
#endif
    {
        NRF_SetCE(hw,NRF_PIN_RESET);
    }

#if PLATFORM == 0
    if (hw->csn_pin.pin != 0 || hw->csn_pin.port != NULL)
#elif PLATFORM == 1
    if (hw->csn_pin.gpio_num >= 0)
#endif
    {
        NRF_Deselect(hw);
    }

#if PLATFORM == 0
    if (hw->irq_pin.port != NULL && hw->irq_pin.pin != 0)
#elif PLATFORM == 1
    if (hw->irq_pin.gpio_num >= 0)
#endif
    {
        status = GPIO_DeInitPin(&hw->irq_pin);
        if (status != NRF_CORE_OK && result == NRF_HW_OK)
        {
            result = NRF_HW_MapCoreStatus(status);
        }
    }

    status = GPIO_DeInitPin(&hw->csn_pin);
    if (status != NRF_CORE_OK && result == NRF_HW_OK)
    {
        result = NRF_HW_MapCoreStatus(status);
    }

    status = GPIO_DeInitPin(&hw->ce_pin);
    if (status != NRF_CORE_OK && result == NRF_HW_OK)
    {
        result = NRF_HW_MapCoreStatus(status);
    }

    status = NRF_Core_Timer_DeInit(&hw->bus_dev);
    if (status != NRF_CORE_OK && result == NRF_HW_OK)
    {
        result = NRF_HW_MapCoreStatus(status);
    }

    status = NRF_Core_SPI_DeInit(&hw->bus_dev);
    if (status != NRF_CORE_OK && result == NRF_HW_OK)
    {
        result = NRF_HW_MapCoreStatus(status);
    }

    return result;
}
/* ============================================================
 * Read Register
 * ============================================================ */
NRF_Hw_Status_t NRF_ReadRegister(NRF_Hardware_t *hw,NRF_Register_t reg,uint8_t *pData)
{
    if (hw == NULL ||pData == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t min_size;
    uint8_t max_size;
    uint8_t readable;
    uint8_t writable;
    if (!NRF_HW_GetRegisterInfo(reg,&min_size,&max_size,&readable,&writable))
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (readable == 0 || min_size != 1 || max_size != 1)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t tx_data = NRF_CMD_R_REGISTER |((uint8_t)reg & 0x1FU);
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&tx_data,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }

    status =SPI_Receive(&hw->bus_dev,pData,1);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Write Register
 * ============================================================ */
NRF_Hw_Status_t NRF_WriteRegister(NRF_Hardware_t *hw,NRF_Register_t reg,uint8_t value)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t min_size;
    uint8_t max_size;
    uint8_t readable;
    uint8_t writable;
    if (!NRF_HW_GetRegisterInfo(reg,&min_size,&max_size,&readable,&writable))
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (writable == 0 || min_size != 1 || max_size != 1)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t tx_data[2];
    tx_data[0] = NRF_CMD_W_REGISTER |((uint8_t)reg & 0x1FU);
    tx_data[1] = value;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,tx_data,2);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Read Multiple Register Bytes
 * ============================================================ */
NRF_Hw_Status_t NRF_ReadMultiRegister(NRF_Hardware_t *hw,NRF_Register_t reg,uint8_t *pData,uint8_t size)
{
    if (hw == NULL ||pData == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t min_size;
    uint8_t max_size;
    uint8_t readable;
    uint8_t writable;
    if (!NRF_HW_GetRegisterInfo(reg,&min_size,&max_size,&readable,&writable))
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (readable == 0)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (size < min_size || size > max_size)
    {
        return NRF_HW_ERR_INVALID_SIZE;
    }
    uint8_t tx_cmd = NRF_CMD_R_REGISTER |((uint8_t)reg & 0x1FU);
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&tx_cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Receive(&hw->bus_dev,pData,size);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Write Multiple Register Bytes
 * ============================================================ */
NRF_Hw_Status_t NRF_WriteMultiRegister(NRF_Hardware_t *hw,NRF_Register_t reg,const uint8_t *pData,uint8_t size)
{
    if (hw == NULL ||pData == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t min_size;
    uint8_t max_size;
    uint8_t readable;
    uint8_t writable;
    if (!NRF_HW_GetRegisterInfo(reg,&min_size,&max_size,&readable,&writable))
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (writable == 0)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (size < min_size || size > max_size)
    {
        return NRF_HW_ERR_INVALID_SIZE;
    }
    uint8_t tx_cmd = NRF_CMD_W_REGISTER |((uint8_t)reg & 0x1FU);
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&tx_cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Transmit(&hw->bus_dev,pData,size);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Read RX Payload
 * ============================================================ */
NRF_Hw_Status_t NRF_ReadPayload(NRF_Hardware_t *hw,uint8_t *pData,uint8_t size)
{
    if (hw == NULL ||pData == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (size == 0 || size > 32)
    {
        return NRF_HW_ERR_INVALID_SIZE;
    }
    uint8_t cmd = NRF_CMD_R_RX_PAYLOAD;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Receive(&hw->bus_dev,pData,size);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Write TX Payload
 * ============================================================ */
NRF_Hw_Status_t NRF_WritePayload(NRF_Hardware_t *hw,const uint8_t *pData,uint8_t size)
{
    if (hw == NULL ||pData == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (size == 0 || size > 32)
    {
        return NRF_HW_ERR_INVALID_SIZE;
    }
    uint8_t cmd = NRF_CMD_W_TX_PAYLOAD;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Transmit(&hw->bus_dev,pData,size);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Flush TX
 * ============================================================ */
NRF_Hw_Status_t NRF_FlushTX(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t cmd = NRF_CMD_FLUSH_TX;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Flush RX
 * ============================================================ */
NRF_Hw_Status_t NRF_FlushRX(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t cmd = NRF_CMD_FLUSH_RX;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Get STATUS Register
 * ============================================================ */
NRF_Hw_Status_t NRF_GetStatus(NRF_Hardware_t *hw,uint8_t *pStatus)
{
    if (hw == NULL ||pStatus == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t tx_cmd = NRF_CMD_NOP;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_TransmitReceive(&hw->bus_dev,&tx_cmd,pStatus,1);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * ACTIVATE Command
 * ============================================================ */
NRF_Hw_Status_t NRF_Activate(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t tx_data[2];
    tx_data[0] = NRF_CMD_ACTIVATE;
    tx_data[1] = NRF_ACTIVATE_DATA;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,tx_data,2);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
/* ============================================================
 * Read RX Payload Width (Command: R_RX_PL_WID per Page 46)
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_ReadRxPayloadWidth(NRF_Hardware_t *hw,uint8_t *pWidth)
{
    if (hw == NULL || pWidth == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t cmd = NRF_CMD_R_RX_PL_WID;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Receive(&hw->bus_dev,pWidth,1);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}

/* ============================================================
 * Write ACK Payload (Command: W_ACK_PAYLOAD per Page 46)
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_WriteAckPayload(NRF_Hardware_t *hw,uint8_t pipe,const uint8_t *pData,uint8_t size)
{
    if (hw == NULL || pData == NULL || pipe > 5U)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (size == 0 || size > 32)
    {
        return NRF_HW_ERR_INVALID_SIZE;
    }
    uint8_t cmd = NRF_CMD_W_ACK_PAYLOAD | (pipe & 0x07U);
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Transmit(&hw->bus_dev,pData,size);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}

/* ============================================================
 * Write TX Payload Without ACK (Command: W_TX_PAYLOAD_NOACK)
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_WritePayloadNoAck(NRF_Hardware_t *hw,const uint8_t *pData,uint8_t size)
{
    if (hw == NULL || pData == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    if (size == 0 || size > 32)
    {
        return NRF_HW_ERR_INVALID_SIZE;
    }
    uint8_t cmd = NRF_CMD_W_TX_PAYLOAD_NOACK;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    if (status != NRF_CORE_OK)
    {
        NRF_Deselect(hw);
        return NRF_HW_MapCoreStatus(status);
    }
    status = SPI_Transmit(&hw->bus_dev,pData,size);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}

/* ============================================================
 * Reuse Last Transmitted Payload (Command: REUSE_TX_PL)
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_ReuseTxPayload(NRF_Hardware_t *hw)
{
    if (hw == NULL)
    {
        return NRF_HW_ERR_INVALID_PARAM;
    }
    uint8_t cmd = NRF_CMD_REUSE_TX_PL;
    NRF_Select(hw);
    NRF_Core_Status_t status = SPI_Transmit(&hw->bus_dev,&cmd,1);
    NRF_Deselect(hw);
    if (status != NRF_CORE_OK)
    {
        return NRF_HW_MapCoreStatus(status);
    }
    return NRF_HW_OK;
}
