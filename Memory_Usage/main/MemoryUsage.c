#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_pm.h"
#include "esp_wifi.h"



void print_memory_info() {
    printf("Total heap: %ld bytes\n", esp_get_free_heap_size());
    printf("Minimum heap: %ld bytes\n", esp_get_minimum_free_heap_size());
    printf("Internal free heap: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

/*void print_thread_info() {
    printf("Total tasks: %d\n", uxTaskGetNumberOfTasks());
    TaskStatus_t* taskStatusArray = (TaskStatus_t*) malloc(sizeof(TaskStatus_t) * uxTaskGetNumberOfTasks());
    if (taskStatusArray != NULL) {
        int numTasks = uxTaskGetSystemState(taskStatusArray, uxTaskGetNumberOfTasks(), NULL);
        printf("Task Name\t\tStatus\t\tPriority\tStack Left\n");
        for (int i = 0; i < numTasks; i++) {
            printf("%s\t\t%d\t\t%d\t\t%ld\n", taskStatusArray[i].pcTaskName, taskStatusArray[i].eCurrentState,
                    taskStatusArray[i].uxCurrentPriority, taskStatusArray[i].usStackHighWaterMark);
        }
        free(taskStatusArray);
    }
}

void print_process_info() {
    esp_process_iterator_t proc_iterator = ESP_PROCESS_ITERATOR_INIT;
    esp_process_info_t proc_info;
    printf("Process ID\tProcess Name\tCPU Usage\n");
    while (esp_process_iterator_next(&proc_iterator, &proc_info)) {
        printf("%d\t\t%s\t\t%.2f %%\n", proc_info.pid, proc_info.name, proc_info.cpu_usage);
    }
}

void print_cpu_metrics() {
    esp_pm_cpu_freq_t cpu_freq;
    esp_pm_config_esp32_t config;
    esp_pm_esp32_get_config(&config);
    printf("CPU frequency: %d MHz\n", config.max_freq_mhz);
    esp_pm_esp32_get_cpu_freq(&cpu_freq);
    printf("Current CPU frequency: %d MHz\n", cpu_freq.mhz);
}*/

void app_main() {
    print_memory_info();
   // print_thread_info();
    //print_process_info();
    //print_cpu_metrics();
}