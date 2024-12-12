/**
 * @file ota.c
 * @brief OTA update implementation.
 *
 * This file contains the implementation of the OTA update process,
 * including the HTTP event handler and the OTA update task.
 */

#include "ota.h"
#include "mqtt.h"

/** @brief Tag for logging messages. */
static const char *TAG = "OTA";

/** @brief Counter for the total bytes downloaded during OTA update. */
static int total_bytes_downloaded = 0;

/** @brief External reference to the start of the server certificate in PEM format. */
extern const uint8_t server_pem_start[] asm("_binary_server_pem_start");

/** @brief External reference to the end of the server certificate in PEM format. */
extern const uint8_t server_pem_end[] asm("_binary_server_pem_end");

/**
 * @brief HTTP event handler for OTA updates.
 *
 * This function handles HTTP events during the OTA update process.
 *
 * @param[in] evt Pointer to the HTTP client event.
 * @return ESP_OK on success, or an appropriate error code on failure.
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            total_bytes_downloaded = 0; // Reset counter when connected
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            total_bytes_downloaded += evt->data_len;
            //ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d, total=%d KB", evt->data_len, total_bytes_downloaded / 1024);
            printf("\rI (OTA): HTTP_EVENT_ON_DATA, len=%d, total=%d KB", evt->data_len, total_bytes_downloaded / 1024);
            fflush(stdout);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
        default:
            ESP_LOGD(TAG, "Other HTTP Event");
            break;
    }
    return ESP_OK;
}

/**
 * @brief Task function for performing OTA updates.
 *
 * This FreeRTOS task performs the OTA update process by downloading the firmware
 * from the specified URL and applying the update. It handles retries and publishes
 * the update status via MQTT.
 *
 * @param[in] pvParameter Pointer to the task parameters (not used).
 */
void ota_update_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting OTA update task");
    ESP_LOGI(TAG, "Free heap size before OTA: %lu", esp_get_free_heap_size());

    int retry_count = 0;
    esp_err_t ret = ESP_FAIL;
    bool failure_published = false;

    while (retry_count < OTA_MAX_RETRY_COUNT) {
        total_bytes_downloaded = 0; // Reset byte counter before each attempt
        esp_http_client_config_t config = {
            .url = ota_firmware_url,
            .cert_pem = (char *)server_pem_start,
            .event_handler = _http_event_handler,
            .keep_alive_enable = true,
            .timeout_ms = 10000,
        };

        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };

        ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
        ret = esp_https_ota(&ota_config);

        if (ret == ESP_OK) {
            // Log the successful download
            ESP_LOGI(TAG, "File downloaded successfully (%d KB)", total_bytes_downloaded / 1024);
            
            // Send MQTT message indicating the file is downloaded
            const char *downloaded_msg = "{\n"
                                         "    \"action\": \"set\",\n"
                                         "    \"payload\": {\n"
                                         "        \"status\": \"downloaded\",\n"
                                         "        \"url\": \"%s\"\n"
                                         "     }\n"
                                         "}";
            char msg[512];
            snprintf(msg, sizeof(msg), downloaded_msg, ota_firmware_url);
            esp_mqtt_client_publish(client2, status_topic, msg, 0, 1, 0);

            ESP_LOGI(TAG, "OTA update succeeded. Rebooting...");
            const char *success_msg = "{\n"
                                      "    \"action\": \"set\",\n"
                                      "    \"payload\": {\n"
                                      "        \"status\": \"true\"\n"
                                      "     }\n"
                                      "}";
            esp_mqtt_client_publish(client2, status_topic, success_msg, 0, 1, 0);
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            esp_restart();
        } else {
            ESP_LOGE(TAG, "OTA update failed");
            if (!failure_published) {
                const char *fail_msg = "{\n"
                                      "    \"action\": \"set\",\n"
                                      "    \"payload\": {\n"
                                      "        \"status\": \"false\",\n"
                                      "        \"error\": \"not able to download through that url\"\n"
                                      "     }\n"
                                      "}";
                esp_mqtt_client_publish(client2, status_topic, fail_msg, 0, 0, 0);
                failure_published = true;
            }
            retry_count++;
            vTaskDelay(OTA_RETRY_INTERVAL_MS / portTICK_PERIOD_MS);
        }
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OTA update failed %d times. Rebooting...", OTA_MAX_RETRY_COUNT);
        esp_restart();
    }

    vTaskDelete(NULL);
}