/*
 * NRF_Core.c
 *
 *  Created on: Sep 24, 2026
 *      Author: amire
 */


#include "nrf_core.h"
#include <string.h>

#define NRF_SPI_TIMEOUT_MS  (100U)

/* ============================================================
 * STM32 EXTI Callback Registry & Multi-Instance Management
 * ============================================================ */
#if PLATFORM == 0

static GPIO_TypeDef *g_exti_ports[16] = {0};

static void *g_exti_user_args[16] = {0};

static GPIO_Callback_t g_exti_callbacks[16] = {0};

static uint8_t g_nvic_ref_count[7] = {0};

static IRQn_Type NRF_Core_GetPinIRQn(uint16_t pin)
{
    switch (pin)
    {
        case GPIO_PIN_0:  return EXTI0_IRQn;
        case GPIO_PIN_1:  return EXTI1_IRQn;
        case GPIO_PIN_2:  return EXTI2_IRQn;
        case GPIO_PIN_3:  return EXTI3_IRQn;
        case GPIO_PIN_4:  return EXTI4_IRQn;
        case GPIO_PIN_5:
        case GPIO_PIN_6:
        case GPIO_PIN_7:
        case GPIO_PIN_8:
        case GPIO_PIN_9:  return EXTI9_5_IRQn;
        default:          return EXTI15_10_IRQn;
    }
}

static uint8_t NRF_Core_GetIRQnIndex(IRQn_Type irqn)
{
    switch (irqn)
    {
        case EXTI0_IRQn:     return 0U;
        case EXTI1_IRQn:     return 1U;
        case EXTI2_IRQn:     return 2U;
        case EXTI3_IRQn:     return 3U;
        case EXTI4_IRQn:     return 4U;
        case EXTI9_5_IRQn:   return 5U;
        default:             return 6U;
    }
}

#endif

/* ============================================================
 * Timer Initialization
 * ============================================================ */
NRF_Core_Status_t NRF_Core_Timer_Init(Bus_Device_t *bus_dev)
{
    if (bus_dev == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->htim == NULL)
    {
        return NRF_CORE_ERR_TIMER_INIT;
    }

    if (HAL_TIM_Base_Start(bus_dev->htim) != HAL_OK)
    {
        return NRF_CORE_ERR_TIMER_INIT;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    /* ESP32 Timer Init */
    return NRF_CORE_OK;

#else

    return NRF_CORE_ERR_TIMER_INIT;

#endif
}

/* ============================================================
 * Timer DeInitialization
 * ============================================================ */
NRF_Core_Status_t NRF_Core_Timer_DeInit(Bus_Device_t *bus_dev)
{
    if (bus_dev == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->htim == NULL)
    {
        return NRF_CORE_ERR_TIMER_DEINIT;
    }

    if (HAL_TIM_Base_Stop(bus_dev->htim) != HAL_OK)
    {
        return NRF_CORE_ERR_TIMER_DEINIT;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    /* ESP32 Timer DeInit */
    return NRF_CORE_OK;

#else

    return NRF_CORE_ERR_TIMER_DEINIT;

#endif
}

/* ============================================================
 * Delay
 * ============================================================ */
void NRF_delay_us(Bus_Device_t *bus_dev,uint32_t us)
{
    if (bus_dev == NULL || us == 0)
    {
        return;
    }

#if PLATFORM == 0

    if (bus_dev->htim == NULL)
    {
        return;
    }

    while (us > 0)
    {
        uint16_t step = (us > 60000U) ? 60000U : (uint16_t)us;
        __HAL_TIM_SET_COUNTER(bus_dev->htim,0);
        while (__HAL_TIM_GET_COUNTER(bus_dev->htim) < step);
        us -= step;
    }

#elif PLATFORM == 1
	esp_rom_delay_us(us);

#endif
}

void NRF_delay(Bus_Device_t *bus_dev,uint32_t ms)
{
    if (ms == 0)
    {
        return;
    }

#if PLATFORM == 0

    if (bus_dev != NULL && bus_dev->htim != NULL)
    {
        while (ms--)
        {
            NRF_delay_us(bus_dev,1000U);
        }
    }
    else
    {
        HAL_Delay(ms);
    }

#elif PLATFORM == 1
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        TickType_t ticks = pdMS_TO_TICKS(ms);
        vTaskDelay(ticks ? ticks : 1);
    }
    else
    {
        esp_rom_delay_us(ms * 1000U);
    }
#if SIM_USE_FREERTOS
    vTaskDelay(pdMS_TO_TICKS(ms));
#endif

#else

    HAL_Delay(ms);

#endif
}

uint32_t NRF_Core_GetTick(void)
{
#if PLATFORM == 0
    return HAL_GetTick();
#elif PLATFORM == 1
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
#else
    return 0;
#endif
}

/* ============================================================
 * GPIO Initialization
 * ============================================================ */
NRF_Core_Status_t GPIO_InitPin(GPIO_Pin_init_t *gpio_pin)
{
    if (gpio_pin == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (gpio_pin->port == NULL || gpio_pin->pin == 0)
    {
        return NRF_CORE_ERR_GPIO_INIT;
    }

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = gpio_pin->pin;
    GPIO_InitStruct.Mode  = gpio_pin->mode;
    GPIO_InitStruct.Pull  = gpio_pin->pull;
    GPIO_InitStruct.Speed = gpio_pin->speed;

    HAL_GPIO_Init(gpio_pin->port,&GPIO_InitStruct);

    if ((gpio_pin->mode & 0x10000U) != 0U)
    {
        IRQn_Type irqn = NRF_Core_GetPinIRQn(gpio_pin->pin);
        uint8_t idx = NRF_Core_GetIRQnIndex(irqn);

        if (g_nvic_ref_count[idx] == 0U)
        {
            HAL_NVIC_SetPriority(irqn, 5, 0);
            HAL_NVIC_EnableIRQ(irqn);
        }
        g_nvic_ref_count[idx]++;
    }

    if (gpio_pin->callback != NULL)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            if (gpio_pin->pin & (1U << i))
            {
                g_exti_ports[i] = gpio_pin->port;
                g_exti_callbacks[i] = gpio_pin->callback;
                g_exti_user_args[i] = gpio_pin->user_arg;
            }
        }
    }

    return NRF_CORE_OK;


#elif PLATFORM == 1

    if (gpio_pin->gpio_num < 0)
    {
        return NRF_CORE_ERR_GPIO_INIT;
    }

    gpio_config_t io_conf = {0};

    io_conf.pin_bit_mask = (1ULL << gpio_pin->gpio_num);
    io_conf.mode = gpio_pin->mode;

    io_conf.pull_up_en =
        (gpio_pin->pull == GPIO_PULLUP_ONLY ||
         gpio_pin->pull == GPIO_PULLUP_PULLDOWN)
        ? GPIO_PULLUP_ENABLE
        : GPIO_PULLUP_DISABLE;

    io_conf.pull_down_en =
        (gpio_pin->pull == GPIO_PULLDOWN_ONLY ||
         gpio_pin->pull == GPIO_PULLUP_PULLDOWN)
        ? GPIO_PULLDOWN_ENABLE
        : GPIO_PULLDOWN_DISABLE;

    io_conf.intr_type =
        (gpio_pin->intr_type != 0)
        ? gpio_pin->intr_type
        : GPIO_INTR_DISABLE;


    if (gpio_config(&io_conf) != ESP_OK)
    {
        return NRF_CORE_ERR_GPIO_INIT;
    }


    if (gpio_pin->callback != NULL)
    {
        esp_err_t err;

        err = gpio_install_isr_service(0);

        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            gpio_reset_pin(gpio_pin->gpio_num);
            return NRF_CORE_ERR_GPIO_INIT;
        }


        err = gpio_isr_handler_add(gpio_pin->gpio_num,(gpio_isr_t)gpio_pin->callback,gpio_pin->user_arg);


        if (err != ESP_OK)
        {
            gpio_reset_pin(gpio_pin->gpio_num);
            return NRF_CORE_ERR_GPIO_INIT;
        }
    }

    return NRF_CORE_OK;


#else

    return NRF_CORE_ERR_GPIO_INIT;

#endif
}
/* ============================================================
 * GPIO DeInitialization
 * ============================================================ */
NRF_Core_Status_t GPIO_DeInitPin(GPIO_Pin_t *pin)
{
    if (pin == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }


#if PLATFORM == 0

    if (pin->port == NULL || pin->pin == 0)
    {
        return NRF_CORE_ERR_GPIO_DEINIT;
    }

    IRQn_Type irqn = NRF_Core_GetPinIRQn(pin->pin);
    uint8_t idx = NRF_Core_GetIRQnIndex(irqn);

    if (g_nvic_ref_count[idx] > 0U)
    {
        g_nvic_ref_count[idx]--;
        if (g_nvic_ref_count[idx] == 0U)
        {
            HAL_NVIC_DisableIRQ(irqn);
        }
    }

    HAL_GPIO_DeInit(pin->port,pin->pin);


    for (uint8_t i = 0; i < 16; i++)
    {
        if (pin->pin & (1U << i))
        {
            if (g_exti_ports[i] == pin->port)
            {
                g_exti_ports[i] = NULL;
                g_exti_callbacks[i] = NULL;
                g_exti_user_args[i] = NULL;
            }
        }
    }

    return NRF_CORE_OK;


#elif PLATFORM == 1

    if (pin->gpio_num < 0)
    {
        return NRF_CORE_ERR_GPIO_DEINIT;
    }

    esp_err_t err;

    err = gpio_isr_handler_remove(pin->gpio_num);

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE &&
        err != ESP_ERR_NOT_FOUND)
    {
        return NRF_CORE_ERR_GPIO_DEINIT;
    }


    err = gpio_reset_pin(pin->gpio_num);

    if (err != ESP_OK)
    {
        return NRF_CORE_ERR_GPIO_DEINIT;
    }

    return NRF_CORE_OK;


#else

    return NRF_CORE_ERR_GPIO_DEINIT;

#endif
}
/* ============================================================
 * GPIO Callback Registration
 * ============================================================ */
NRF_Core_Status_t GPIO_SetCallback(GPIO_Pin_t *pin,GPIO_Callback_t callback,void *user_arg)
{
    if (pin == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (pin->port == NULL || pin->pin == 0)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < 16; i++)
    {
        if (pin->pin & (1U << i))
        {
            g_exti_ports[i] = pin->port;
            g_exti_callbacks[i] = callback;
            g_exti_user_args[i] = user_arg;
        }
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    if (pin->gpio_num < 0)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }


    esp_err_t err;

    err = gpio_isr_handler_remove(pin->gpio_num);

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE &&
        err != ESP_ERR_NOT_FOUND)
    {
        return NRF_CORE_ERR_GPIO_DEINIT;
    }


    if (callback == NULL)
    {
        return NRF_CORE_OK;
    }


    err = gpio_install_isr_service(0);

    if (err != ESP_OK &&
        err != ESP_ERR_INVALID_STATE)
    {
        return NRF_CORE_ERR_GPIO_INIT;
    }


    err = gpio_isr_handler_add(
        pin->gpio_num,
        (gpio_isr_t)callback,
        user_arg
    );


    if (err != ESP_OK)
    {
        return NRF_CORE_ERR_GPIO_INIT;
    }

    return NRF_CORE_OK;


#else

    return NRF_CORE_ERR_INVALID_PARAM;

#endif
}
/* ============================================================
 * GPIO Read
 * ============================================================ */
NRF_PinState_t GPIO_ReadPin(GPIO_Pin_t *gpio_pin)
{
    if (gpio_pin == NULL)
    {
        return NRF_PIN_RESET;
    }

#if PLATFORM == 0

    if (gpio_pin->port == NULL || gpio_pin->pin == 0)
    {
        return NRF_PIN_RESET;
    }

    return(HAL_GPIO_ReadPin(gpio_pin->port,gpio_pin->pin) == GPIO_PIN_SET)? NRF_PIN_SET: NRF_PIN_RESET;

#elif PLATFORM == 1

    if (gpio_pin->gpio_num < 0)
    {
        return NRF_PIN_RESET;
    }

    return(gpio_get_level(gpio_pin->gpio_num) == 1)? NRF_PIN_SET: NRF_PIN_RESET;


#else

    return NRF_PIN_RESET;

#endif
}
/* ============================================================
 * GPIO Write
 * ============================================================ */
void GPIO_WritePin(GPIO_Pin_t *gpio_pin,NRF_PinState_t state)
{
    if (gpio_pin == NULL)
    {
        return;
    }

#if PLATFORM == 0

    if (gpio_pin->port == NULL || gpio_pin->pin == 0)
    {
        return;
    }

    HAL_GPIO_WritePin(gpio_pin->port,gpio_pin->pin,(state == NRF_PIN_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET);

#elif PLATFORM == 1

    if (gpio_pin->gpio_num < 0)
    {
        return;
    }

    gpio_set_level(gpio_pin->gpio_num,(state == NRF_PIN_SET) ? 1 : 0);

#endif
}

/* ============================================================
 * STM32 EXTI Dispatcher & Callback
 * ============================================================ */
#if PLATFORM == 0

void NRF_Core_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
    for (uint8_t i = 0; i < 16; i++)
    {
        if (GPIO_Pin & (1U << i))
        {
            if (g_exti_callbacks[i] != NULL)
            {
                g_exti_callbacks[i](
                    g_exti_user_args[i]
                );
            }
        }
    }
}

__weak void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    NRF_Core_GPIO_EXTI_IRQHandler(GPIO_Pin);
}

#endif

/* ============================================================
 * SPI Initialization
 * ============================================================ */
NRF_Core_Status_t NRF_Core_SPI_Init(Bus_Device_t *bus_dev)
{
    if (bus_dev == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->hspi == NULL)
    {
        return NRF_CORE_ERR_SPI_INIT;
    }

    if (HAL_SPI_Init(bus_dev->hspi) != HAL_OK)
    {
        return NRF_CORE_ERR_SPI_INIT;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    if (bus_dev->spi_handle == NULL)
    {
        return NRF_CORE_ERR_SPI_INIT;
    }

    if (bus_dev->spi_handle == NULL) return NRF_CORE_ERR_SPI_INIT;
    return NRF_CORE_OK;

#else

    return NRF_CORE_ERR_SPI_INIT;

#endif
}
/* ============================================================
 * SPI DeInitialization
 * ============================================================ */
NRF_Core_Status_t NRF_Core_SPI_DeInit(Bus_Device_t *bus_dev)
{
    if (bus_dev == NULL)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->hspi == NULL)
    {
        return NRF_CORE_ERR_SPI_DEINIT;
    }

    if (HAL_SPI_DeInit(bus_dev->hspi) != HAL_OK)
    {
        return NRF_CORE_ERR_SPI_DEINIT;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1
    return NRF_CORE_OK;


#else

    return NRF_CORE_ERR_SPI_DEINIT;

#endif
}
/* ============================================================
 * SPI Transmit
 * ============================================================ */
NRF_Core_Status_t SPI_Transmit(Bus_Device_t *bus_dev,const uint8_t *pData,uint16_t size)
{
    if (bus_dev == NULL || pData == NULL || size == 0)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->hspi == NULL)
    {
        return NRF_CORE_ERR_SPI_INIT;
    }

    HAL_StatusTypeDef status = HAL_SPI_Transmit(bus_dev->hspi,(uint8_t *)pData,size,NRF_SPI_TIMEOUT_MS);

    if (status == HAL_TIMEOUT)
    {
        return NRF_CORE_ERR_SPI_TIMEOUT;
    }

    if (status != HAL_OK)
    {
        return NRF_CORE_ERR_SPI_TRANSFER;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    if (bus_dev->spi_handle == NULL) return NRF_CORE_ERR_SPI_INIT;
    if (size > 36) return NRF_CORE_ERR_INVALID_PARAM;

    uint8_t tx_temp[36] = {0};
    memcpy(tx_temp, pData, size);

    spi_transaction_t t = {0};
    t.length    = (size_t)size * 8U;
    t.tx_buffer = tx_temp;
    t.rx_buffer = NULL;

    esp_err_t err = spi_device_transmit(bus_dev->spi_handle, &t);
    return (err == ESP_OK) ? NRF_CORE_OK : NRF_CORE_ERR_SPI_TRANSFER;

#else

    return NRF_CORE_ERR_SPI_TRANSFER;

#endif
}
/* ============================================================
 * SPI Receive
 * ============================================================ */
NRF_Core_Status_t SPI_Receive(Bus_Device_t *bus_dev,uint8_t *pData,uint16_t size)
{
    if (bus_dev == NULL || pData == NULL || size == 0)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->hspi == NULL)
    {
        return NRF_CORE_ERR_SPI_INIT;
    }

    HAL_StatusTypeDef status = HAL_SPI_Receive(bus_dev->hspi,pData,size,NRF_SPI_TIMEOUT_MS);

    if (status == HAL_TIMEOUT)
    {
        return NRF_CORE_ERR_SPI_TIMEOUT;
    }

    if (status != HAL_OK)
    {
        return NRF_CORE_ERR_SPI_TRANSFER;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    if (bus_dev->spi_handle == NULL) return NRF_CORE_ERR_SPI_INIT;
    if (size > 36) return NRF_CORE_ERR_INVALID_PARAM;

    uint8_t rx_temp[36] = {0};

    spi_transaction_t t = {0};
    t.length    = (size_t)size * 8U;
    t.tx_buffer = NULL;
    t.rx_buffer = rx_temp;

    esp_err_t err = spi_device_transmit(bus_dev->spi_handle, &t);
    if (err != ESP_OK) return NRF_CORE_ERR_SPI_TRANSFER;

    memcpy(pData, rx_temp, size);
    return NRF_CORE_OK;
#else

    return NRF_CORE_ERR_SPI_TRANSFER;

#endif
}
/* ============================================================
 * SPI Transmit + Receive
 * ============================================================ */
NRF_Core_Status_t SPI_TransmitReceive(Bus_Device_t *bus_dev,const uint8_t *pTxData,uint8_t *pRxData,uint16_t size)
{
    if (bus_dev == NULL || pTxData == NULL || pRxData == NULL || size == 0)
    {
        return NRF_CORE_ERR_INVALID_PARAM;
    }

#if PLATFORM == 0

    if (bus_dev->hspi == NULL)
    {
        return NRF_CORE_ERR_SPI_INIT;
    }

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(bus_dev->hspi,(uint8_t *)pTxData,pRxData,size,NRF_SPI_TIMEOUT_MS);

    if (status == HAL_TIMEOUT)
    {
        return NRF_CORE_ERR_SPI_TIMEOUT;
    }

    if (status != HAL_OK)
    {
        return NRF_CORE_ERR_SPI_TRANSFER;
    }

    return NRF_CORE_OK;

#elif PLATFORM == 1

    if (bus_dev->spi_handle == NULL) return NRF_CORE_ERR_SPI_INIT;
    if (size > 36) return NRF_CORE_ERR_INVALID_PARAM;

    uint8_t tx_temp[36] = {0};
    uint8_t rx_temp[36] = {0};
    memcpy(tx_temp, pTxData, size);

    spi_transaction_t t = {0};
    t.length    = (size_t)size * 8U;
    t.tx_buffer = tx_temp;
    t.rx_buffer = rx_temp;

    esp_err_t err = spi_device_transmit(bus_dev->spi_handle, &t);
    if (err != ESP_OK) return NRF_CORE_ERR_SPI_TRANSFER;

    memcpy(pRxData, rx_temp, size);
    return NRF_CORE_OK;
#else
    return NRF_CORE_ERR_SPI_TRANSFER;
#endif
}
