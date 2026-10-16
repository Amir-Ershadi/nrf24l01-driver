/*
 * nrf.h
 *
 *  Created on: Sep 25, 2026
 *      Author: amire
 */

#ifndef INC_NRF_H_
#define INC_NRF_H_

#include "nrf_hw.h"

/* ============================================================
 * NRF Driver Status (Layer 3 Namespace)
 * ============================================================ */
typedef enum {
    NRF_OK = 0,
    NRF_ERR_INVALID_HANDLE   = -20,
    NRF_ERR_SPI_COMM         = -21,
    NRF_ERR_TIMEOUT          = -22,
    NRF_ERR_INVALID_PARAM    = -23,
    NRF_ERR_MAX_RETRIES      = -24,
    NRF_ERR_FIFO_FULL        = -25,
    NRF_ERR_FIFO_EMPTY       = -26,
    NRF_ERR_BUSY             = -27,
    NRF_ERR_GPIO             = -28,
    NRF_ERR_TIMER            = -29
} NRF_Status_t;

#define NRF_App_Status_t NRF_Status_t
#define NRF_APP_OK       NRF_OK

/* ============================================================
 * Configuration Enums
 * ============================================================ */
typedef enum {
    NRF_RX_DR_INT_ENABLED = 0,
    NRF_RX_DR_INT_MASKED  = 1
} NRF_RxDrMask_t;

typedef enum {
    NRF_TX_DS_INT_ENABLED = 0,
    NRF_TX_DS_INT_MASKED  = 1
} NRF_TxDsMask_t;

typedef enum {
    NRF_MAX_RT_INT_ENABLED = 0,
    NRF_MAX_RT_INT_MASKED  = 1
} NRF_MaxRtMask_t;

typedef enum {
    NRF_CRC_DISABLED = 0,
    NRF_CRC_ENABLED  = 1
} NRF_CrcState_t;

typedef enum {
    NRF_CRC_LEN_1_BYTE  = 0,
    NRF_CRC_LEN_2_BYTES = 1
} NRF_CrcLength_t;

typedef enum {
    NRF_POWER_DOWN = 0,
    NRF_POWER_UP   = 1
} NRF_PowerMode_t;

typedef enum {
    NRF_ROLE_PTX = 0,
    NRF_ROLE_PRX = 1
} NRF_Role_t;

typedef enum {
    NRF_PIPE_NONE = 0x00,
    NRF_PIPE_0    = (1 << 0),
    NRF_PIPE_1    = (1 << 1),
    NRF_PIPE_2    = (1 << 2),
    NRF_PIPE_3    = (1 << 3),
    NRF_PIPE_4    = (1 << 4),
    NRF_PIPE_5    = (1 << 5),
    NRF_PIPE_ALL  = 0x3F
} NRF_PipeMask_t;

typedef enum {
    NRF_AW_3BYTES = 1,
    NRF_AW_4BYTES = 2,
    NRF_AW_5BYTES = 3
} NRF_AddrWidth_t;

typedef enum {
    NRF_ARD_250US  = 0x00,
    NRF_ARD_500US  = 0x01,
    NRF_ARD_750US  = 0x02,
    NRF_ARD_1000US = 0x03,
    NRF_ARD_1250US = 0x04,
    NRF_ARD_1500US = 0x05,
    NRF_ARD_1750US = 0x06,
    NRF_ARD_2000US = 0x07,
    NRF_ARD_2250US = 0x08,
    NRF_ARD_2500US = 0x09,
    NRF_ARD_2750US = 0x0A,
    NRF_ARD_3000US = 0x0B,
    NRF_ARD_3250US = 0x0C,
    NRF_ARD_3500US = 0x0D,
    NRF_ARD_3750US = 0x0E,
    NRF_ARD_4000US = 0x0F
} NRF_RetransmitDelay_t;


typedef enum {
    NRF_BR_1MBPS   = 0,
    NRF_BR_2MBPS   = 1,
    NRF_BR_250KBPS = 2
} NRF_DataRate_t;

typedef enum {
    NRF_PWR_M18DBM = 0,
    NRF_PWR_M12DBM = 1,
    NRF_PWR_M6DBM  = 2,
    NRF_PWR_0DBM   = 3
} NRF_OutputPower_t;

#define NRF_PWR_18DBM NRF_PWR_M18DBM
#define NRF_PWR_12DBM NRF_PWR_M12DBM
#define NRF_PWR_6DBM  NRF_PWR_M6DBM

/* ============================================================
 * Hardware Live Status Enums
 * ============================================================ */
typedef enum {
    NRF_RX_DR_INACTIVE = 0,
    NRF_RX_DR_ACTIVE   = 1
} NRF_RxDrStatus_t;

typedef enum {
    NRF_TX_DS_INACTIVE = 0,
    NRF_TX_DS_ACTIVE   = 1
} NRF_TxDsStatus_t;

typedef enum {
    NRF_MAX_RT_INACTIVE = 0,
    NRF_MAX_RT_ACTIVE   = 1
} NRF_MaxRtStatus_t;

typedef enum {
    NRF_TX_FIFO_NOT_FULL = 0,
    NRF_TX_FIFO_FULL     = 1
} NRF_TxFullStatus_t;

typedef enum {
    NRF_RX_PIPE_0        = 0,
    NRF_RX_PIPE_1        = 1,
    NRF_RX_PIPE_2        = 2,
    NRF_RX_PIPE_3        = 3,
    NRF_RX_PIPE_4        = 4,
    NRF_RX_PIPE_5        = 5,
    NRF_RX_PIPE_RESERVED = 6,
    NRF_RX_PIPE_EMPTY    = 7
} NRF_RxPipeStatus_t;

typedef enum {
    NRF_CD_NOT_DETECTED = 0,
    NRF_CD_DETECTED     = 1
} NRF_CarrierDetect_t;

typedef uint8_t NRF_PacketLossCount_t;
typedef uint8_t NRF_RetransmitCount_t;

/* ============================================================
 * Dedicated Address Types
 * ============================================================ */
typedef struct {
    uint8_t bytes[5];
} NRF_RxAddrP0_t;

typedef struct {
    uint8_t bytes[5];
} NRF_RxAddrP1_t;

typedef uint8_t NRF_RxAddrP2_t;
typedef uint8_t NRF_RxAddrP3_t;
typedef uint8_t NRF_RxAddrP4_t;
typedef uint8_t NRF_RxAddrP5_t;

typedef struct {
    uint8_t bytes[5];
} NRF_TxAddr_t;

/* ============================================================
 * Dedicated Payload Width Types
 * ============================================================ */
typedef uint8_t NRF_RxPwP0_t;
typedef uint8_t NRF_RxPwP1_t;
typedef uint8_t NRF_RxPwP2_t;
typedef uint8_t NRF_RxPwP3_t;
typedef uint8_t NRF_RxPwP4_t;
typedef uint8_t NRF_RxPwP5_t;

/* ============================================================
 * FIFO & Features
 * ============================================================ */
typedef enum {
    NRF_RX_FIFO_NOT_EMPTY = 0,
    NRF_RX_FIFO_EMPTY     = 1
} NRF_RxFifoEmpty_t;

typedef enum {
    NRF_RX_FIFO_NOT_FULL = 0,
    NRF_RX_FIFO_FULL     = 1
} NRF_RxFifoFull_t;

typedef enum {
    NRF_TX_FIFO_NOT_EMPTY = 0,
    NRF_TX_FIFO_EMPTY     = 1
} NRF_TxFifoEmpty_t;

typedef enum {
    NRF_TX_REUSE_INACTIVE = 0,
    NRF_TX_REUSE_ACTIVE   = 1
} NRF_TxReuseStatus_t;

typedef enum {
    NRF_FEATURE_NONE       = 0x00,
    NRF_FEATURE_EN_DYN_ACK = (1 << 0),
    NRF_FEATURE_EN_ACK_PAY = (1 << 1),
    NRF_FEATURE_EN_DPL     = (1 << 2)
} NRF_FeatureMask_t;

/* ============================================================
 * Device Structure (Shadow/Configuration State)
 * ============================================================ */
typedef struct {
    NRF_Hardware_t hw;

    NRF_RxDrMask_t rx_dr_mask;
    NRF_TxDsMask_t tx_ds_mask;
    NRF_MaxRtMask_t max_rt_mask;
    NRF_CrcState_t crc_state;
    NRF_CrcLength_t crc_length;
    NRF_PowerMode_t power_mode;
    NRF_Role_t role;

    NRF_PipeMask_t auto_ack_mask;
    NRF_PipeMask_t rx_addr_mask;
    NRF_AddrWidth_t addr_width;
    NRF_RetransmitDelay_t retr_delay;
    uint8_t retr_count;
    uint8_t rf_channel;
    NRF_DataRate_t data_rate;
    NRF_OutputPower_t rf_pwr;

    NRF_RxAddrP0_t rx_addr_p0;
    NRF_RxAddrP1_t rx_addr_p1;
    NRF_RxAddrP2_t rx_addr_p2;
    NRF_RxAddrP3_t rx_addr_p3;
    NRF_RxAddrP4_t rx_addr_p4;
    NRF_RxAddrP5_t rx_addr_p5;
    NRF_TxAddr_t tx_addr;

    NRF_RxPwP0_t rx_pw_p0;
    NRF_RxPwP1_t rx_pw_p1;
    NRF_RxPwP2_t rx_pw_p2;
    NRF_RxPwP3_t rx_pw_p3;
    NRF_RxPwP4_t rx_pw_p4;
    NRF_RxPwP5_t rx_pw_p5;

    NRF_PipeMask_t dynpd_mask;
    NRF_FeatureMask_t feature_mask;
    uint8_t feature_access_enabled;

} NRF_Device_t;

/* ============================================================
 * Function Prototypes
 * ============================================================ */

/* ============================================================
 * Initialization & Teardown
 * ============================================================ */
NRF_Status_t NRF_Init(NRF_Device_t *dev);
NRF_Status_t NRF_DeInit(NRF_Device_t *dev);
/* ============================================================
 * Configuration Register Access
 * ============================================================ */
NRF_Status_t NRF_SetRxDrMask(NRF_Device_t *dev, NRF_RxDrMask_t mask);
NRF_Status_t NRF_GetRxDrMask(NRF_Device_t *dev, NRF_RxDrMask_t *mask);

NRF_Status_t NRF_SetTxDsMask(NRF_Device_t *dev, NRF_TxDsMask_t mask);
NRF_Status_t NRF_GetTxDsMask(NRF_Device_t *dev, NRF_TxDsMask_t *mask);

NRF_Status_t NRF_SetMaxRtMask(NRF_Device_t *dev, NRF_MaxRtMask_t mask);
NRF_Status_t NRF_GetMaxRtMask(NRF_Device_t *dev, NRF_MaxRtMask_t *mask);

NRF_Status_t NRF_SetCrcState(NRF_Device_t *dev, NRF_CrcState_t state);
NRF_Status_t NRF_GetCrcState(NRF_Device_t *dev, NRF_CrcState_t *state);

NRF_Status_t NRF_SetCrcLength(NRF_Device_t *dev, NRF_CrcLength_t length);
NRF_Status_t NRF_GetCrcLength(NRF_Device_t *dev, NRF_CrcLength_t *length);

NRF_Status_t NRF_SetPowerMode(NRF_Device_t *dev, NRF_PowerMode_t mode);
NRF_Status_t NRF_GetPowerMode(NRF_Device_t *dev, NRF_PowerMode_t *mode);

NRF_Status_t NRF_SetRole(NRF_Device_t *dev, NRF_Role_t role);
NRF_Status_t NRF_GetRole(NRF_Device_t *dev, NRF_Role_t *role);

NRF_Status_t NRF_SetAutoAckBitmask(NRF_Device_t *dev, NRF_PipeMask_t pipe_bitmask);
NRF_Status_t NRF_GetAutoAckBitmask(NRF_Device_t *dev, NRF_PipeMask_t *pipe_bitmask);

NRF_Status_t NRF_SetRxAddrBitmask(NRF_Device_t *dev, NRF_PipeMask_t pipe_bitmask);
NRF_Status_t NRF_GetRxAddrBitmask(NRF_Device_t *dev, NRF_PipeMask_t *pipe_bitmask);

NRF_Status_t NRF_SetAddressWidth(NRF_Device_t *dev, NRF_AddrWidth_t width);
NRF_Status_t NRF_GetAddressWidth(NRF_Device_t *dev, NRF_AddrWidth_t *width);

NRF_Status_t NRF_SetAutoRetransmit(NRF_Device_t *dev, NRF_RetransmitDelay_t delay, uint8_t count);
NRF_Status_t NRF_GetAutoRetransmit(NRF_Device_t *dev, NRF_RetransmitDelay_t *delay, uint8_t *count);

NRF_Status_t NRF_SetChannel(NRF_Device_t *dev, uint8_t channel);
NRF_Status_t NRF_GetChannel(NRF_Device_t *dev, uint8_t *channel);

NRF_Status_t NRF_SetDataRate(NRF_Device_t *dev, NRF_DataRate_t rate);
NRF_Status_t NRF_GetDataRate(NRF_Device_t *dev, NRF_DataRate_t *rate);

NRF_Status_t NRF_SetOutputPower(NRF_Device_t *dev, NRF_OutputPower_t pwr);
NRF_Status_t NRF_GetOutputPower(NRF_Device_t *dev, NRF_OutputPower_t *pwr);
/* ============================================================
 * Live Hardware Status Queries
 * ============================================================ */
NRF_Status_t NRF_GetRxDrStatus(NRF_Device_t *dev, NRF_RxDrStatus_t *status);
NRF_Status_t NRF_ClearRxDrFlag(NRF_Device_t *dev);

NRF_Status_t NRF_GetTxDsStatus(NRF_Device_t *dev, NRF_TxDsStatus_t *status);
NRF_Status_t NRF_ClearTxDsFlag(NRF_Device_t *dev);

NRF_Status_t NRF_GetMaxRtStatus(NRF_Device_t *dev, NRF_MaxRtStatus_t *status);
NRF_Status_t NRF_ClearMaxRtFlag(NRF_Device_t *dev);

NRF_Status_t NRF_GetRxPipeNumber(NRF_Device_t *dev, NRF_RxPipeStatus_t *pipe_num);

NRF_Status_t NRF_GetTxFullStatus(NRF_Device_t *dev, NRF_TxFullStatus_t *status);

NRF_Status_t NRF_GetCarrierDetect(NRF_Device_t *dev, NRF_CarrierDetect_t *detected);

NRF_Status_t NRF_GetObserveTx(NRF_Device_t *dev, NRF_PacketLossCount_t *plos_cnt, NRF_RetransmitCount_t *arc_cnt);
NRF_Status_t NRF_GetPacketLossCount(NRF_Device_t *dev, NRF_PacketLossCount_t *plos_cnt);
NRF_Status_t NRF_GetRetransmitCount(NRF_Device_t *dev, NRF_RetransmitCount_t *arc_cnt);
/* ============================================================
 * Dedicated RX & TX Addresses
 * ============================================================ */
NRF_Status_t NRF_SetRxAddrP0(NRF_Device_t *dev, const NRF_RxAddrP0_t *addr);
NRF_Status_t NRF_GetRxAddrP0(NRF_Device_t *dev, NRF_RxAddrP0_t *addr);

NRF_Status_t NRF_SetRxAddrP1(NRF_Device_t *dev, const NRF_RxAddrP1_t *addr);
NRF_Status_t NRF_GetRxAddrP1(NRF_Device_t *dev, NRF_RxAddrP1_t *addr);

NRF_Status_t NRF_SetRxAddrP2(NRF_Device_t *dev, NRF_RxAddrP2_t addr);
NRF_Status_t NRF_GetRxAddrP2(NRF_Device_t *dev, NRF_RxAddrP2_t *addr);

NRF_Status_t NRF_SetRxAddrP3(NRF_Device_t *dev, NRF_RxAddrP3_t addr);
NRF_Status_t NRF_GetRxAddrP3(NRF_Device_t *dev, NRF_RxAddrP3_t *addr);

NRF_Status_t NRF_SetRxAddrP4(NRF_Device_t *dev, NRF_RxAddrP4_t addr);
NRF_Status_t NRF_GetRxAddrP4(NRF_Device_t *dev, NRF_RxAddrP4_t *addr);

NRF_Status_t NRF_SetRxAddrP5(NRF_Device_t *dev, NRF_RxAddrP5_t addr);
NRF_Status_t NRF_GetRxAddrP5(NRF_Device_t *dev, NRF_RxAddrP5_t *addr);

NRF_Status_t NRF_SetTxAddr(NRF_Device_t *dev, const NRF_TxAddr_t *addr);
NRF_Status_t NRF_GetTxAddr(NRF_Device_t *dev, NRF_TxAddr_t *addr);
/* ============================================================
 * Dedicated RX Payload Widths
 * ============================================================ */
NRF_Status_t NRF_SetRxPwP0(NRF_Device_t *dev, NRF_RxPwP0_t pw);
NRF_Status_t NRF_GetRxPwP0(NRF_Device_t *dev, NRF_RxPwP0_t *pw);

NRF_Status_t NRF_SetRxPwP1(NRF_Device_t *dev, NRF_RxPwP1_t pw);
NRF_Status_t NRF_GetRxPwP1(NRF_Device_t *dev, NRF_RxPwP1_t *pw);

NRF_Status_t NRF_SetRxPwP2(NRF_Device_t *dev, NRF_RxPwP2_t pw);
NRF_Status_t NRF_GetRxPwP2(NRF_Device_t *dev, NRF_RxPwP2_t *pw);

NRF_Status_t NRF_SetRxPwP3(NRF_Device_t *dev, NRF_RxPwP3_t pw);
NRF_Status_t NRF_GetRxPwP3(NRF_Device_t *dev, NRF_RxPwP3_t *pw);

NRF_Status_t NRF_SetRxPwP4(NRF_Device_t *dev, NRF_RxPwP4_t pw);
NRF_Status_t NRF_GetRxPwP4(NRF_Device_t *dev, NRF_RxPwP4_t *pw);

NRF_Status_t NRF_SetRxPwP5(NRF_Device_t *dev, NRF_RxPwP5_t pw);
NRF_Status_t NRF_GetRxPwP5(NRF_Device_t *dev, NRF_RxPwP5_t *pw);
/* ============================================================
 * FIFO Status Queries
 * ============================================================ */
NRF_Status_t NRF_GetRxFifoEmptyStatus(NRF_Device_t *dev, NRF_RxFifoEmpty_t *status);

NRF_Status_t NRF_GetRxFifoFullStatus(NRF_Device_t *dev, NRF_RxFifoFull_t *status);

NRF_Status_t NRF_GetTxFifoEmptyStatus(NRF_Device_t *dev, NRF_TxFifoEmpty_t *status);

NRF_Status_t NRF_GetTxFifoFullStatus(NRF_Device_t *dev, NRF_TxFullStatus_t *status);

NRF_Status_t NRF_GetTxReuseStatus(NRF_Device_t *dev, NRF_TxReuseStatus_t *status);
/* ============================================================
 * Dynamic Payload & Features
 * ============================================================ */
NRF_Status_t NRF_SetDynamicPayloadMask(NRF_Device_t *dev, NRF_PipeMask_t mask);
NRF_Status_t NRF_GetDynamicPayloadMask(NRF_Device_t *dev, NRF_PipeMask_t *mask);

NRF_Status_t NRF_SetFeatureMask(NRF_Device_t *dev, NRF_FeatureMask_t mask);
NRF_Status_t NRF_GetFeatureMask(NRF_Device_t *dev, NRF_FeatureMask_t *mask);
/* ============================================================
 * CE Control & Data Transfer
 * ============================================================ */
NRF_Status_t NRF_SetCEState(NRF_Device_t *dev, NRF_PinState_t state);

NRF_Status_t NRF_WriteTxPayload(NRF_Device_t *dev, const uint8_t *data, uint8_t size);

NRF_Status_t NRF_ReadRxPayload(NRF_Device_t *dev, uint8_t *data, uint8_t size);

NRF_Status_t NRF_FlushTx(NRF_Device_t *dev);

NRF_Status_t NRF_FlushRx(NRF_Device_t *dev);

NRF_Status_t NRF_TransmitPayload(NRF_Device_t *dev, const uint8_t *data, uint8_t size);
/* ============================================================
 * Advanced Commands
 * ============================================================ */
NRF_Status_t NRF_ReadRxPayloadWidth(NRF_Device_t *dev, uint8_t *width);

NRF_Status_t NRF_WriteAckPayload(NRF_Device_t *dev, uint8_t pipe, const uint8_t *data, uint8_t size);

NRF_Status_t NRF_WritePayloadNoAck(NRF_Device_t *dev, const uint8_t *data, uint8_t size);

NRF_Status_t NRF_ReuseTxPayload(NRF_Device_t *dev);

#endif /* INC_NRF_H_ */
