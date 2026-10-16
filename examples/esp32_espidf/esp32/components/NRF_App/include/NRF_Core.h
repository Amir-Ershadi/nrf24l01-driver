/*
 * NRF_Core.h
 *
 *  Created on: Sep 24, 2026
 *      Author: amire
 */

#ifndef INC_NRF_CORE_H_
#define INC_NRF_CORE_H_

#include <stdint.h>

#ifndef PLATFORM
#define PLATFORM 1
#endif

#if PLATFORM == 0

#include "main.h"

#elif PLATFORM == 1

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#endif

#if SIM_USE_FREERTOS

#include "FreeRTOS.h"
#include "task.h"

#endif


/* ============================================================
 * NRF Core Status
 * ============================================================ */

typedef enum
{
    NRF_CORE_OK                  = 0,

    NRF_CORE_ERR_INVALID_PARAM   = -1,

    NRF_CORE_ERR_SPI_INIT        = -2,
    NRF_CORE_ERR_SPI_DEINIT      = -3,
    NRF_CORE_ERR_SPI_TRANSFER    = -4,
    NRF_CORE_ERR_SPI_TIMEOUT     = -5,

    NRF_CORE_ERR_GPIO_INIT       = -6,
    NRF_CORE_ERR_GPIO_DEINIT     = -7,

    NRF_CORE_ERR_TIMER_INIT      = -8,
    NRF_CORE_ERR_TIMER_DEINIT    = -9

} NRF_Core_Status_t;


/* ============================================================
 * GPIO
 * ============================================================ */

typedef enum
{
    NRF_PIN_RESET = 0,
    NRF_PIN_SET   = 1

} NRF_PinState_t;


/* GPIO Pin */

typedef struct
{
#if PLATFORM == 0

    /* STM32 */

    GPIO_TypeDef *port;
    uint16_t pin;

#elif PLATFORM == 1

    /* ESP32 */

    gpio_num_t gpio_num;

#endif

} GPIO_Pin_t;


/* GPIO Callback */

typedef void (*GPIO_Callback_t)(void *arg);


/* GPIO Initialization Configuration */

typedef struct
{
#if PLATFORM == 0

    /* STM32 */

    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t mode;
    uint32_t pull;
    uint32_t speed;

#elif PLATFORM == 1

    /* ESP32 */

    gpio_num_t gpio_num;
    gpio_mode_t mode;
    gpio_pull_mode_t pull;
    int intr_type;

#endif

    GPIO_Callback_t callback;
    void *user_arg;

} GPIO_Pin_init_t;


/* ============================================================
 * Bus & Timing Device (Unified SPI + Timer)
 * ============================================================ */

typedef struct
{
#if PLATFORM == 0
    SPI_HandleTypeDef *hspi;
    TIM_HandleTypeDef *htim;
#elif PLATFORM == 1
    spi_device_handle_t spi_handle;
    void *timer_handle;
#endif
} Bus_Device_t;


/* ============================================================
 * Timer API
 * ============================================================ */
NRF_Core_Status_t NRF_Core_Timer_Init(Bus_Device_t *bus_dev);
NRF_Core_Status_t NRF_Core_Timer_DeInit(Bus_Device_t *bus_dev);

/* ============================================================
 * Delay
 * ============================================================ */
void NRF_delay(Bus_Device_t *bus_dev,uint32_t ms);
void NRF_delay_us(Bus_Device_t *bus_dev,uint32_t us);
uint32_t NRF_Core_GetTick(void);
/* ============================================================
 * GPIO API
 * ============================================================ */
NRF_Core_Status_t GPIO_InitPin(GPIO_Pin_init_t *gpio_pin);
NRF_Core_Status_t GPIO_DeInitPin(GPIO_Pin_t *pin);
NRF_Core_Status_t GPIO_SetCallback(GPIO_Pin_t *pin,GPIO_Callback_t callback,void *user_arg);
NRF_PinState_t GPIO_ReadPin(GPIO_Pin_t *gpio_pin);
void GPIO_WritePin(GPIO_Pin_t *gpio_pin,NRF_PinState_t state);
void NRF_Core_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin);

/* ============================================================
 * SPI API
 * ============================================================ */
NRF_Core_Status_t NRF_Core_SPI_Init(Bus_Device_t *bus_dev);
NRF_Core_Status_t NRF_Core_SPI_DeInit(Bus_Device_t *bus_dev);
NRF_Core_Status_t SPI_Transmit(Bus_Device_t *bus_dev,const uint8_t *pData,uint16_t size);
NRF_Core_Status_t SPI_Receive(Bus_Device_t *bus_dev,uint8_t *pData,uint16_t size);
NRF_Core_Status_t SPI_TransmitReceive(Bus_Device_t *bus_dev,const uint8_t *pTxData,uint8_t *pRxData,uint16_t size);

#endif /* INC_NRF_CORE_H_ */
