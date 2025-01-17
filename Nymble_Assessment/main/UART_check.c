/**
 * @file uart_eeprom_nvs.c
 * @brief UART communication with chunked data storage and retrieval using EEPROM (NVS).
 *
 * This program implements UART communication, storing received data in EEPROM
 * (emulated using NVS) in fixed-size chunks and retrieving it for retransmission.
 *
 * @author Rajath B T
 * @date 17/01/2025
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include <string.h>

#define UART_PORT UART_NUM_1       ///< UART port to use
#define TXD_PIN GPIO_NUM_17        ///< UART TX pin
#define RXD_PIN GPIO_NUM_16        ///< UART RX pin
#define UART_BUFFER_SIZE 1024      ///< UART buffer size
#define EEPROM_CHUNK_SIZE 101      ///< Number of bytes per chunk stored in EEPROM

static const char *LOG_TAG = "UART_COMM"; ///< Log tag for UART communication module
static nvs_handle_t eeprom_handle;       ///< Handle for NVS (EEPROM emulation)
static SemaphoreHandle_t stored_successfully; ///< Semaphore for signaling data readiness
static int eeprom_chunk_index = 0;       ///< Global index for chunked data storage

/**
 * @brief Initialize the UART peripheral.
 *
 * Configures UART with the specified baud rate, data bits, parity, stop bits,
 * and disables hardware flow control.
 */
void uart_initialize(void) {
    uart_config_t uart_config = {
        .baud_rate = 2400,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT, UART_BUFFER_SIZE * 2, UART_BUFFER_SIZE * 2, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(LOG_TAG, "UART initialized successfully");
}

/**
 * @brief Initialize the EEPROM (NVS).
 *
 * Initializes the NVS storage. If the storage is corrupt or has no free pages,
 * it erases and reinitializes it.
 */
void eeprom_initialize(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(nvs_open("eeprom_storage", NVS_READWRITE, &eeprom_handle));

    ESP_LOGI(LOG_TAG, "EEPROM (NVS) initialized successfully");
}

/**
 * @brief Store data chunks into EEPROM.
 *
 * Splits the input data into fixed-size chunks and stores each chunk into
 * NVS under a unique key.
 *
 * @param[in] key_prefix Prefix for NVS keys.
 * @param[in] data Pointer to the data buffer.
 * @param[in] length Length of the data buffer.
//  */
void eeprom_store_chunks(const char *key_prefix, uint8_t *data, size_t length) {
    while (length > 0) {
        size_t chunk_length = length > EEPROM_CHUNK_SIZE ? EEPROM_CHUNK_SIZE : length;

        // Create a unique key for the chunk
        char key[16];
        snprintf(key, sizeof(key), "%s_%d", key_prefix, eeprom_chunk_index++);

        // Prepare the chunk buffer
        uint8_t chunk_buffer[EEPROM_CHUNK_SIZE] = {0};
        memcpy(chunk_buffer, data, chunk_length);

        // Log the data being stored in the chunk
        ESP_LOGI(LOG_TAG, "Storing chunk under key '%s': %.*s", key, (int)chunk_length, chunk_buffer);

        // Store the chunk in NVS
        ESP_ERROR_CHECK(nvs_set_blob(eeprom_handle, key, chunk_buffer, chunk_length));
        ESP_ERROR_CHECK(nvs_commit(eeprom_handle));

        ESP_LOGI(LOG_TAG, "Stored chunk under key '%s' (%d bytes)", key, chunk_length);

        // Move to the next chunk
        data += chunk_length;
        length -= chunk_length;
    }
}


/**
 * @brief Retrieve and send stored data chunks from EEPROM.
 *
 * Reads all stored chunks from NVS and sends them over UART.
 *
 * @param[in] key_prefix Prefix for NVS keys.
 */
void eeprom_retrieve_and_send_chunks(const char *key_prefix) {
    uint8_t chunk[EEPROM_CHUNK_SIZE + 1];
    size_t chunk_size;
    int chunk_index = 0;

    while (1) {
        char key[16];
        snprintf(key, sizeof(key), "%s_%d", key_prefix, chunk_index);

        chunk_size = EEPROM_CHUNK_SIZE;
        esp_err_t err = nvs_get_blob(eeprom_handle, key, chunk, &chunk_size);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(LOG_TAG, "All chunks retrieved and sent");
            break;
        } else if (err == ESP_OK) {
            uart_write_bytes(UART_PORT, (const char *)chunk, chunk_size);
            ESP_LOGI(LOG_TAG, "Sent chunk '%s' (%d bytes)", key, chunk_size);
        } else {
            ESP_LOGE(LOG_TAG, "Error retrieving chunk '%s': %s", key, esp_err_to_name(err));
            break;
        }
        chunk_index++;
    }
}

/**
 * @brief Task to receive data from UART and store it in EEPROM.
 */
void uart_receive_task(void *pvParameters) {
    uint8_t rx_buffer[UART_BUFFER_SIZE];
    while (1) {
        int bytes_read = uart_read_bytes(UART_PORT, rx_buffer, UART_BUFFER_SIZE, pdMS_TO_TICKS(1000));
        if (bytes_read > 0) {
            ESP_LOGI(LOG_TAG, "Received %d bytes from UART", bytes_read);
            eeprom_store_chunks("uart_data", rx_buffer, bytes_read);
            xSemaphoreGive(stored_successfully);
        } else {
            ESP_LOGW(LOG_TAG, "No data received in the last interval");
        }
    }
}

/**
 * @brief Task to retrieve and send stored data from EEPROM when signaled.
 */
void uart_retrieve_and_send_task(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(stored_successfully, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(LOG_TAG, "Data ready signal received");
            vTaskDelay(pdMS_TO_TICKS(10000));
            eeprom_retrieve_and_send_chunks("uart_data");
        }
    }
}

/**
 * @brief Main application entry point.
 *
 * Initializes UART, EEPROM, and creates FreeRTOS tasks for UART reception and
 * data retrieval.
 */
void app_main(void) {
    uart_initialize();
    eeprom_initialize();

    stored_successfully = xSemaphoreCreateBinary();
    if (stored_successfully == NULL) {
        ESP_LOGE(LOG_TAG, "Failed to create semaphore");
        return;
    }

    xTaskCreate(uart_receive_task, "UART Receive Task", 4096, NULL, 10, NULL);
    xTaskCreate(uart_retrieve_and_send_task, "UART Retrieve and Send Task", 4096, NULL, 10, NULL);
}
