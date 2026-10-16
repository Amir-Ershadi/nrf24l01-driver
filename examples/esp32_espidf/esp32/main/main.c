#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "NRF_App.h"

static const char *TAG = "ESP32_UNIVERSAL";

#define PIN_NUM_MISO   GPIO_NUM_19
#define PIN_NUM_MOSI   GPIO_NUM_23
#define PIN_NUM_CLK    GPIO_NUM_18
#define PIN_NUM_CSN    GPIO_NUM_21
#define PIN_NUM_CE     GPIO_NUM_22
#define PIN_NUM_IRQ    GPIO_NUM_4

#define NRF_ROLE_TRANSMITTER   0  
#define NRF_ROLE_RECEIVER      1   

#define NRF_DEVICE_ROLE        NRF_ROLE_TRANSMITTER  

#define NRF_MODE_POLLING       0   
#define NRF_MODE_INTERRUPT     1   

#define NRF_OPERATION_MODE     NRF_MODE_POLLING 

NRF_App_t nrf_app;
static uint32_t packet_counter = 0;
uint8_t tx_data[4] = {0xBB, 0x66, 0x02, 0x00};


#if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)

static TaskHandle_t s_nrf_isr_task_handle = NULL;

void ESP32_Radio_Callback(NRF_App_t *app, const NRF_App_Event_t *event, void *user_arg)
{
#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)
    if (event->tx_data_sent) {
        packet_counter++;
        tx_data[3]++;
        ESP_LOGI(TAG, " [IRQ TX SUCCESS #%lu] Packet sent and ACK received!", packet_counter);
    }
    if (event->tx_max_retry) {
        ESP_LOGE(TAG, " [IRQ TX MAX_RT] Receiver didn't acknowledge!");
        NRF_App_FlushTx(app);
    }
#else
    if (event->rx_data_ready) {
        uint8_t rx_buffer[4];
        if (NRF_App_Receive(app, rx_buffer, 4) == NRF_OK) {
            packet_counter++;
            ESP_LOGI(TAG, "⚡ [IRQ RX PACKET #%lu] Data: [0x%02X, 0x%02X, 0x%02X, 0x%02X]",
                     packet_counter, rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3]);
        }
    }
#endif
}

static void nrf_irq_worker_task(void *pvParameters)
{
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        NRF_App_IRQHandler(&nrf_app);
    }
}

static void IRAM_ATTR nrf_gpio_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(s_nrf_isr_task_handle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif

void app_main(void)
{
#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)
    ESP_LOGI(TAG, "Starting ESP32 in TRANSMITTER (PTX) role...");
#else
    ESP_LOGI(TAG, "Starting ESP32 in RECEIVER (PRX) role...");
#endif

#if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)
    ESP_LOGI(TAG, "Operating Mode: HARDWARE INTERRUPT (FreeRTOS Worker Task)");
    xTaskCreate(nrf_irq_worker_task, "nrf_irq_task", 4096, NULL, configMAX_PRIORITIES - 1, &s_nrf_isr_task_handle);
#else
    ESP_LOGI(TAG, "Operating Mode: PURE POLLING");
#endif


    spi_bus_config_t buscfg = {
        .miso_io_num     = PIN_NUM_MISO,
        .mosi_io_num     = PIN_NUM_MOSI,
        .sclk_io_num     = PIN_NUM_CLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = 64
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4 * 1000 * 1000,
        .mode           = 0,
        .spics_io_num   = -1,
        .queue_size     = 7
    };
    spi_device_handle_t spi_handle;
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);

   
    nrf_app.dev.hw.bus_dev.spi_handle = spi_handle;
    nrf_app.dev.hw.ce_pin.gpio_num    = PIN_NUM_CE;
    nrf_app.dev.hw.csn_pin.gpio_num   = PIN_NUM_CSN;

#if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)
    nrf_app.dev.hw.irq_pin.gpio_num   = PIN_NUM_IRQ;
#else
    nrf_app.dev.hw.irq_pin.gpio_num   = -1;
#endif

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
        nrf_app.event_callback = ESP32_Radio_Callback;
        gpio_install_isr_service(0);
        gpio_set_intr_type(PIN_NUM_IRQ, GPIO_INTR_NEGEDGE);
        gpio_isr_handler_add(PIN_NUM_IRQ, nrf_gpio_isr_handler, NULL);
#endif

        NRF_App_Start(&nrf_app);
        ESP_LOGI(TAG, "Radio Node initialized and ONLINE!");
    }

    while (1)
    {
#if (NRF_DEVICE_ROLE == NRF_ROLE_TRANSMITTER)
    #if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)
        NRF_App_Transmit(&nrf_app, tx_data, 4);
    #else
        NRF_TxResult_t result;
        NRF_App_TransmitWait(&nrf_app, tx_data, 4, 100, &result);
        if (result == NRF_TX_SUCCESS) {
            packet_counter++;
            tx_data[3]++;
            ESP_LOGI(TAG, " [POLLING TX SUCCESS #%lu] ACK Received!", packet_counter);
        } else {
            ESP_LOGW(TAG, " [POLLING TX FAIL] Receiver did not respond!");
        }
    #endif
        vTaskDelay(pdMS_TO_TICKS(500));

#else

    #if (NRF_OPERATION_MODE == NRF_MODE_INTERRUPT)
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    #else
        uint8_t available = 0;
        NRF_App_DataAvailable(&nrf_app, &available);
        if (available) {
            uint8_t rx_buffer[4] = {0};
            if (NRF_App_Receive(&nrf_app, rx_buffer, 4) == NRF_OK) {
                packet_counter++;
                ESP_LOGI(TAG, " [POLLING RX PACKET #%lu] Data: [0x%02X, 0x%02X, 0x%02X, 0x%02X]",
                         packet_counter, rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3]);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    #endif

#endif
    }
}