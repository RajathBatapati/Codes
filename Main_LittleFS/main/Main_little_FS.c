#include "esp_system.h"
#include "esp_littlefs.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include <stdlib.h>
#include <time.h>
#include "esp_timer.h"
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <dirent.h>
#include <sys/stat.h>

static const char *TAG = "raj_records";

typedef struct {
    int number;
    time_t timestamp;
} Record;

void create_parent_directory(const char *dir_path) {
    if (mkdir(dir_path, 0777) != 0) {
        ESP_LOGE(TAG, "Failed to create directory %s", dir_path);
    } else {
        ESP_LOGI(TAG, "Created directory: %s", dir_path);
    }
}

void create_subfolders() {
    char parent_directory[32];
    snprintf(parent_directory, sizeof(parent_directory), "/littlefs/2023");

    create_parent_directory(parent_directory);

    for (int i = 1; i <= 12; i++) {
        char subfolder_name[64];
        snprintf(subfolder_name, sizeof(subfolder_name), "/littlefs/2023/%02d", i);

        create_parent_directory(subfolder_name);

        // Create a test file in each subfolder
        char test_file_path[128];
        snprintf(test_file_path, sizeof(test_file_path), "%s/testfile.txt", subfolder_name);
        FILE *file = fopen(test_file_path, "w");
        if (file) {
            fprintf(file, "This is a test file.");
            fclose(file);
            ESP_LOGI(TAG, "Created test file: %s", test_file_path);
        }
    }
}



size_t get_folder_size(const char *folder_path) {
    size_t total_size = 0;
    DIR *dir = opendir(folder_path);
    if (dir != NULL) {
        struct dirent *ent;
        struct stat st;
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                char subpath[512];
                snprintf(subpath, sizeof(subpath), "%s/%s", folder_path, ent->d_name);
                if (stat(subpath, &st) == 0) {
                    total_size += st.st_size;
                }
            }
        }
        closedir(dir);
    }
    return total_size;
}

void app_main() {
    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "littlefs",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

    create_subfolders();

    size_t folder_size = get_folder_size("/littlefs/2023");
    ESP_LOGI(TAG, "Size of folder '/littlefs/2023': %u bytes", folder_size);

    // Now you can perform further operations with the created folders if needed.
}
