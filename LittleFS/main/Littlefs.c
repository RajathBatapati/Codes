//Rajath_LittleFS


#include "esp_system.h"
#include "esp_littlefs.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

static const char *TAG = "raj";



void app_main() {
    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
            .base_path = "/littlefs",
            .partition_label = "littlefs",
            .format_if_mount_failed = true,
            .dont_mount = false,
        };

    
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK)
    {
            if (ret == ESP_FAIL)
            {
                    ESP_LOGE(TAG, "Failed to mount or format filesystem");
                }
            else if (ret == ESP_ERR_NOT_FOUND)
                {
                        ESP_LOGE(TAG, "Failed to find LittleFS partition");
                }
            else
                {
                        ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
                }
            return;
        }    

    // Open the text file for writing
    ESP_LOGI(TAG, "Opening file");
    FILE* file = fopen("/littlefs/Rajath.txt", "w");
    if (file == NULL) {
        printf("Failed to open file for writing\n");
        //esp_vfs_littlefs_unregister(NULL);
        return;
    }
    fprintf(file, "Arnowa Arnowa Arnowa!\n");
    fclose(file);
    ESP_LOGI(TAG, "File written");

    // Reopen the file for reading
    file = fopen("/littlefs/Rajath.txt", "r");
    if (file == NULL) {
        printf("Failed to open file for reading\n");
        //esp_vfs_littlefs_unregister(NULL);
        return;
    }


    // Read data from file
    char buffer[64];
    fgets(buffer, sizeof(buffer), file);
    printf("File content: %s\n", buffer);
    fclose(file);

    

    // Del
    
    ret = remove("/littlefs/Rajath.txt");
    if (ret != 0) {
    printf("Failed to delete file\n");
    }   
    else {
    printf("File deleted successfully\n");
    }

    
    

    // Unmount LittleFS
    //esp_vfs_littlefs_unregister(NULL);
    esp_vfs_littlefs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "LittleFS unmounted");
}


    