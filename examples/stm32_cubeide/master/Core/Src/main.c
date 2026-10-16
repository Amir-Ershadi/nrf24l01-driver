/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "NRF_App.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define NRF_ROLE_TRANSMITTER   0
#define NRF_ROLE_RECEIVER      1

#define NRF_DEVICE_ROLE        NRF_ROLE_RECEIVER


#define NRF_MODE_POLLING       0
#define NRF_MODE_INTERRUPT     1

#define NRF_OPERATION_MODE     NRF_MODE_POLLING

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef htim6;
NRF_App_t nrf_app;


volatile struct {
    uint32_t success_count;
    uint32_t fail_count;
    uint8_t  last_rx_data[4];
} node_monitor;

uint8_t tx_data[4] = {0xAA, 0x55, 0x01, 0x00};
uint8_t rx_data[4] = {0};


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim6;

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

#if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)

void STM32_Radio_Callback(NRF_App_t *app, const NRF_App_Event_t *event, void *user_arg)
{
#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)

    if (event->tx_data_sent)
    {
        node_monitor.success_count++;
        tx_data[3]++;
    }
    if (event->tx_max_retry)
    {
        node_monitor.fail_count++;
        NRF_App_FlushTx(app);
    }
#else
    if (event->rx_data_ready)
    {
        if (NRF_App_Receive(app, rx_data, 4) == NRF_OK)
        {
            node_monitor.success_count++;
            memcpy((void *)node_monitor.last_rx_data, rx_data, 4);
        }
    }
#endif
}
#endif

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  /* ---------- Hardware mapping ---------- */
  nrf_app.dev.hw.bus_dev.hspi = &hspi1;
  nrf_app.dev.hw.bus_dev.htim = &htim6;

  nrf_app.dev.hw.ce_pin.port  = CE_GPIO_Port;
  nrf_app.dev.hw.ce_pin.pin   = CE_Pin;

  nrf_app.dev.hw.csn_pin.port = SPI_CSN_GPIO_Port;
  nrf_app.dev.hw.csn_pin.pin  = SPI_CSN_Pin;

#if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)
    nrf_app.dev.hw.irq_pin.port = IRQ_GPIO_Port;
    nrf_app.dev.hw.irq_pin.pin  = IRQ_Pin;
#else
    nrf_app.dev.hw.irq_pin.port = NULL;
    nrf_app.dev.hw.irq_pin.pin  = 0;
#endif

    memset((void *)&node_monitor, 0, sizeof(node_monitor));

    NRF_App_Config_t config = {
#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)
        .role          = NRF_ROLE_PTX,
#else
        .role          = NRF_ROLE_PRX,
#endif
        .crc_state     = NRF_CRC_ENABLED,
        .crc_length    = NRF_CRC_LEN_2_BYTES,
        .auto_ack_mask = NRF_PIPE_ALL,
        .rx_addr_mask  = NRF_PIPE_ALL,
        .addr_width    = NRF_AW_5BYTES,
        .retr_delay    = NRF_ARD_2000US,
        .retr_count    = 15,
        .rf_channel    = 90,
        .data_rate     = NRF_BR_1MBPS,
        .rf_pwr        = NRF_PWR_0DBM,
        .dynpd_mask    = NRF_PIPE_NONE,
        .feature_mask  = NRF_FEATURE_NONE
    };

    if (NRF_App_Init(&nrf_app, &config) == NRF_OK)
    {
        uint8_t common_addr[5] = {0x78, 0x78, 0x78, 0x78, 0x78};

#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)

        NRF_App_SetTxAddress(&nrf_app, common_addr, 5);
        NRF_App_EnablePipe(&nrf_app, 0);
        NRF_App_EnableAutoAck(&nrf_app, 0);
        NRF_App_SetPipePayloadSize(&nrf_app, 0, 4);
#else

        NRF_App_SetRxAddress(&nrf_app, 0, common_addr, 5);
        NRF_App_EnablePipe(&nrf_app, 0);
        NRF_App_EnableAutoAck(&nrf_app, 0);
        NRF_App_SetPipePayloadSize(&nrf_app, 0, 4);
        NRF_App_FlushTx(&nrf_app);

#endif

#if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)

        NRF_App_SetEventCallback(&nrf_app, STM32_Radio_Callback, NULL);
#endif

        NRF_App_Start(&nrf_app);
    }

    HAL_Delay(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)

    #if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)

        NRF_App_Transmit(&nrf_app, tx_data, 4);
    #else

        NRF_TxResult_t result;
        NRF_App_TransmitWait(&nrf_app, tx_data, 4, 100, &result);
        if (result == NRF_TX_SUCCESS)
        {
            node_monitor.success_count++;
            tx_data[3]++;
        }
        else {
            node_monitor.fail_count++;
        }
    #endif
        HAL_Delay(500);

#else

    #if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)

        HAL_Delay(1000);
    #else
        uint8_t available = 0;
        NRF_App_DataAvailable(&nrf_app, &available);
        if (available) {
            if (NRF_App_Receive(&nrf_app, rx_data, 4) == NRF_OK) {
                node_monitor.success_count++;
                memcpy((void *)node_monitor.last_rx_data, rx_data, 4);
            }
        }
        HAL_Delay(10);
    #endif

#endif
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 83;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 65535;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CE_GPIO_Port, CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CSN_GPIO_Port, SPI_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : CE_Pin SPI_CSN_Pin */
  GPIO_InitStruct.Pin = CE_Pin|SPI_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : IRQ_Pin */
  GPIO_InitStruct.Pin = IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IRQ_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
