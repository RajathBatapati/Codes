/**
 * @file ota.h
 * @brief OTA update configuration and function declarations.
 *
 * This header file contains the declarations and configurations for the OTA update,
 * including the HTTP event handler and the OTA update task.
 */

#ifndef OTA_H
#define OTA_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "mqtt.h"

/** @brief Maximum retry count for OTA update attempts. */
#define OTA_MAX_RETRY_COUNT 5

/** @brief Interval between OTA update retry attempts in milliseconds. */
#define OTA_RETRY_INTERVAL_MS 5000

/**
 * @brief HTTP event handler for OTA updates.
 *
 * This function handles HTTP events during the OTA update process.
 *
 * @param[in] evt Pointer to the HTTP client event.
 * @return ESP_OK on success, or an appropriate error code on failure.
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt);

/**
 * @brief Task function for performing OTA updates.
 *
 * This FreeRTOS task performs the OTA update process by downloading the firmware
 * from the specified URL and applying the update.
 *
 * @param[in] pvParameter Pointer to the task parameters (not used).
 */
void ota_update_task(void *pvParameter);

#endif // OTA_H
