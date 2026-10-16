/*
 * nrf_reg.h
 *
 *  Created on: Sep 24, 2026
 *      Author: amire
 */

#ifndef INC_NRF_REG_H_
#define INC_NRF_REG_H_

#include <stdint.h>

/* ============================================================
 * NRF24L01 Register Addresses
 * ============================================================ */
typedef enum
{
    NRF_REG_CONFIG      = 0x00,
    NRF_REG_EN_AA       = 0x01,
    NRF_REG_EN_RXADDR   = 0x02,
    NRF_REG_SETUP_AW    = 0x03,
    NRF_REG_SETUP_RETR  = 0x04,
    NRF_REG_RF_CH       = 0x05,
    NRF_REG_RF_SETUP    = 0x06,
    NRF_REG_STATUS      = 0x07,
    NRF_REG_OBSERVE_TX  = 0x08,
    NRF_REG_RPD         = 0x09,
    NRF_REG_CD          = 0x09,

    NRF_REG_RX_ADDR_P0  = 0x0A,
    NRF_REG_RX_ADDR_P1  = 0x0B,
    NRF_REG_RX_ADDR_P2  = 0x0C,
    NRF_REG_RX_ADDR_P3  = 0x0D,
    NRF_REG_RX_ADDR_P4  = 0x0E,
    NRF_REG_RX_ADDR_P5  = 0x0F,

    NRF_REG_TX_ADDR     = 0x10,

    NRF_REG_RX_PW_P0    = 0x11,
    NRF_REG_RX_PW_P1    = 0x12,
    NRF_REG_RX_PW_P2    = 0x13,
    NRF_REG_RX_PW_P3    = 0x14,
    NRF_REG_RX_PW_P4    = 0x15,
    NRF_REG_RX_PW_P5    = 0x16,

    NRF_REG_FIFO_STATUS = 0x17,

    NRF_REG_DYNPD       = 0x1C,
    NRF_REG_FEATURE     = 0x1D

} NRF_Register_t;

/* ============================================================
 * NRF24L01 Commands
 * ============================================================ */
typedef enum
{
    NRF_CMD_R_REGISTER          = 0x00,
    NRF_CMD_W_REGISTER          = 0x20,
    NRF_CMD_R_RX_PAYLOAD        = 0x61,
    NRF_CMD_W_TX_PAYLOAD        = 0xA0,
    NRF_CMD_FLUSH_TX            = 0xE1,
    NRF_CMD_FLUSH_RX            = 0xE2,
    NRF_CMD_REUSE_TX_PL         = 0xE3,
    NRF_CMD_R_RX_PL_WID         = 0x60,
    NRF_CMD_W_ACK_PAYLOAD       = 0xA8,
    NRF_CMD_W_TX_PAYLOAD_NOACK  = 0xB0,
    NRF_CMD_ACTIVATE            = 0x50,
    NRF_CMD_NOP                 = 0xFF

} NRF_Command_t;

#define NRF_ACTIVATE_DATA       (0x73U)

/* ============================================================
 * Register Bit Definitions
 * ============================================================ */

/* CONFIG Register Bits */
#define NRF_BIT_PRIM_RX         (0U)
#define NRF_BIT_PWR_UP          (1U)
#define NRF_BIT_CRCO            (2U)
#define NRF_BIT_EN_CRC          (3U)
#define NRF_BIT_MASK_MAX_RT     (4U)
#define NRF_BIT_MASK_TX_DS      (5U)
#define NRF_BIT_MASK_RX_DR      (6U)

/* RF_SETUP Register Bits */
#define NRF_BIT_LNA_HCURR       (0U)
#define NRF_BIT_RF_PWR_LOW      (1U)
#define NRF_BIT_RF_PWR_HIGH     (2U)
#define NRF_BIT_RF_DR_HIGH      (3U)
#define NRF_BIT_PLL_LOCK        (4U)
#define NRF_BIT_RF_DR_LOW       (5U)
#define NRF_BIT_CONT_WAVE       (7U)

/* STATUS Register Bits & Masks */
#define NRF_BIT_TX_FULL         (0U)
#define NRF_MASK_RX_P_NO        (0x0EU)
#define NRF_SHIFT_RX_P_NO       (1U)
#define NRF_BIT_MAX_RT          (4U)
#define NRF_BIT_TX_DS           (5U)
#define NRF_BIT_RX_DR           (6U)

/* FIFO_STATUS Register Bits */
#define NRF_BIT_RX_EMPTY        (0U)
#define NRF_BIT_RX_FULL         (1U)
#define NRF_BIT_TX_EMPTY        (4U)
#define NRF_BIT_TX_FULL_FIFO    (5U)
#define NRF_BIT_TX_REUSE        (6U)

/* FEATURE Register Bits */
#define NRF_BIT_EN_DYN_ACK      (0U)
#define NRF_BIT_EN_ACK_PAY      (1U)
#define NRF_BIT_EN_DPL          (2U)

#endif /* INC_NRF_REG_H_ */
