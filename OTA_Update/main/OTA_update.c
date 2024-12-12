/**
 * @file app_main.c
 * @brief Main application entry point.
 *
 * This file contains the main entry point of the application, initializing NVS,
 * setting up Wi-Fi, and starting the MQTT and OTA update tasks.
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "Wifi.h"
#include "mqtt.h"
#include "ota.h"

/** @brief Tag for logging messages. */
#define TAG "MAIN"

/**
 * @brief Main application entry point.
 *
 * This function initializes the Non-Volatile Storage (NVS), sets up the Wi-Fi
 * connection, and starts the MQTT client. It waits for an OTA message to be received
 * and then starts the OTA update task.
 */
void app_main() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the TCP/IP stack
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    mqtt_app_start();

    vTaskDelay(8000 / portTICK_PERIOD_MS);
    mqtt_ota_app_start();

    vTaskDelay(3000 / portTICK_PERIOD_MS);
    while (!ota_message_received) {
        ESP_LOGI(TAG, "Waiting for OTA message...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    xTaskCreate(&ota_update_task, "ota_update_task", 8192, NULL, 5, NULL);
}
