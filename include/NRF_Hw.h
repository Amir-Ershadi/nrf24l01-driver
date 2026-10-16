/*
 * NRF_Hw.h
 *
 *  Created on: Sep 24, 2026
 *      Author: amire
 */

#ifndef INC_NRF_HW_H_
#define INC_NRF_HW_H_

#include "nrf_core.h"
#include "nrf_reg.h"

/* ============================================================
 * NRF Hardware Layer Status
 * ============================================================ */
typedef enum
{
    NRF_HW_OK = 0,

    NRF_HW_ERR_INVALID_PARAM,
    NRF_HW_ERR_INVALID_SIZE,

    NRF_HW_ERR_GPIO,
    NRF_HW_ERR_TIMER,

    NRF_HW_ERR_SPI,
    NRF_HW_ERR_TIMEOUT,

    NRF_HW_ERR_BUSY

} NRF_Hw_Status_t;

/* ============================================================
 * NRF Hardware Object
 * ============================================================ */
typedef struct
{
    Bus_Device_t bus_dev;

    GPIO_Pin_t ce_pin;
    GPIO_Pin_t csn_pin;
    GPIO_Pin_t irq_pin;

} NRF_Hardware_t;

/* ============================================================
 * CSN Control
 * ============================================================ */
void NRF_Select(NRF_Hardware_t *hw);
void NRF_Deselect(NRF_Hardware_t *hw);

/* ============================================================
 * CE Control
 * ============================================================ */
void NRF_SetCE(NRF_Hardware_t *hw,NRF_PinState_t state);

/* ============================================================
 * IRQ Callback
 * ============================================================ */
NRF_Hw_Status_t NRF_SetIRQCallback(NRF_Hardware_t *hw,GPIO_Callback_t callback,void *user_arg);

/* ============================================================
 * Hardware Initialization
 * ============================================================ */
NRF_Hw_Status_t NRF_HW_Init(NRF_Hardware_t *hw);
NRF_Hw_Status_t NRF_HW_DeInit(NRF_Hardware_t *hw);

/* ============================================================
 * Register Access
 * ============================================================ */
NRF_Hw_Status_t NRF_ReadRegister(NRF_Hardware_t *hw,NRF_Register_t reg,uint8_t *pData);
NRF_Hw_Status_t NRF_WriteRegister(NRF_Hardware_t *hw,NRF_Register_t reg,uint8_t value);
NRF_Hw_Status_t NRF_ReadMultiRegister(NRF_Hardware_t *hw,NRF_Register_t reg,uint8_t *pData,uint8_t size);
NRF_Hw_Status_t NRF_WriteMultiRegister(NRF_Hardware_t *hw,NRF_Register_t reg,const uint8_t *pData,uint8_t size);

/* ============================================================
 * Payload Access
 * ============================================================ */
NRF_Hw_Status_t NRF_ReadPayload(NRF_Hardware_t *hw,uint8_t *pData,uint8_t size);
NRF_Hw_Status_t NRF_WritePayload(NRF_Hardware_t *hw,const uint8_t *pData,uint8_t size);

/* ============================================================
 * FIFO Commands
 * ============================================================ */
NRF_Hw_Status_t NRF_FlushTX(NRF_Hardware_t *hw);
NRF_Hw_Status_t NRF_FlushRX(NRF_Hardware_t *hw);

/* ============================================================
 * Status
 * ============================================================ */
NRF_Hw_Status_t NRF_GetStatus(NRF_Hardware_t *hw,uint8_t *pStatus);

/* ============================================================
 * Special Commands (Table 16 - Datasheet)
 * ============================================================ */
NRF_Hw_Status_t NRF_Activate(NRF_Hardware_t *hw);
NRF_Hw_Status_t NRF_HW_ReadRxPayloadWidth(NRF_Hardware_t *hw,uint8_t *pWidth);
NRF_Hw_Status_t NRF_HW_WriteAckPayload(NRF_Hardware_t *hw,uint8_t pipe,const uint8_t *pData,uint8_t size);
NRF_Hw_Status_t NRF_HW_WritePayloadNoAck(NRF_Hardware_t *hw,const uint8_t *pData,uint8_t size);
NRF_Hw_Status_t NRF_HW_ReuseTxPayload(NRF_Hardware_t *hw);

#endif /* INC_NRF_HW_H_ */
