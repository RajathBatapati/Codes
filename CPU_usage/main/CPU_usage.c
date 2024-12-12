#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "spi_flash_mmap.h"

static const char *TAG = "SystemMetrics";

void monitor_system_metrics(void *pvParameters) {
    while (1) {
        // Memory Usage (Heap and RAM)
        ESP_LOGI(TAG, "Free heap size: %ld bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Minimum free heap size: %ld bytes", esp_get_minimum_free_heap_size());

        multi_heap_info_t heap_info;
        heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
        ESP_LOGI(TAG, "Total free bytes: %d", heap_info.total_free_bytes);
        ESP_LOGI(TAG, "Largest free block: %d", heap_info.largest_free_block);
        ESP_LOGI(TAG, "Total allocated bytes: %d", heap_info.total_allocated_bytes);

        ESP_LOGI(TAG, "Internal RAM size: %d bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
        
        #if CONFIG_SPIRAM_SUPPORT
        ESP_LOGI(TAG, "External RAM size: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        #endif

        // Task Stack Usage
        TaskStatus_t *task_array;
        UBaseType_t task_count = uxTaskGetNumberOfTasks();
        task_array = malloc(task_count * sizeof(TaskStatus_t));

        if (task_array != NULL) {
            UBaseType_t task_array_size = uxTaskGetSystemState(task_array, task_count, NULL);
            ESP_LOGI(TAG, "Task count: %u", task_array_size);

            for (int i = 0; i < task_array_size; i++) {
                ESP_LOGI(TAG, "Task: %s, Stack high watermark: %lu bytes",
                       task_array[i].pcTaskName, task_array[i].usStackHighWaterMark);
            }

            // Core and CPU Usage
            uint32_t total_runtime;
            task_array_size = uxTaskGetSystemState(task_array, task_count, &total_runtime);
            ESP_LOGI(TAG, "Total runtime: %lu ticks", total_runtime);

            for (int i = 0; i < task_array_size; i++) {
                ESP_LOGI(TAG, "Task: %s, Runtime: %ld ticks, CPU usage: %.2f%%",
                       task_array[i].pcTaskName,
                       task_array[i].ulRunTimeCounter,
                       (100.0 * task_array[i].ulRunTimeCounter) / total_runtime);
            }

            free(task_array);
        }

        // Chip Information
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s",
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

        // System Uptime
        int64_t uptime = esp_timer_get_time();  // in microseconds
        ESP_LOGI(TAG, "System uptime: %lld seconds", uptime / 1000000);

        // Delay before the next iteration
        vTaskDelay(pdMS_TO_TICKS(10000));  // 10 seconds delay
    }
}

void app_main() {
    // Create a task to monitor system metrics
    xTaskCreatePinnedToCore(&monitor_system_metrics, "monitor_system_metrics", 4096, NULL, 5, NULL, tskNO_AFFINITY);
}
