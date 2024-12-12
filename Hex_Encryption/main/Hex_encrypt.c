// Rajath_LittleFS

#include "esp_system.h"
#include "esp_littlefs.h"
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include <stdlib.h>
#include <time.h>
#include "esp_timer.h"
#include <sys/time.h>

#include "mbedtls/aes.h"

static const char *TAG = "raj_records";

typedef struct {
    int number;
    time_t timestamp;
} Record;

void intToHex(int num, char *hexStr) {
    sprintf(hexStr, "%08X", num);
}

int hexToInt(const char *hexStr) {
    int num;
    sscanf(hexStr, "%X", &num);
    return num;
}

void encryptBuffer(unsigned char *buffer) {
    unsigned char key[32] = "0123456789abcdef0123456789abcdef";
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_enc(&aes_ctx, key, 256);
    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_ENCRYPT, buffer, buffer);
    mbedtls_aes_free(&aes_ctx);
}

void decryptBuffer(unsigned char *buffer) {
    unsigned char key[32] = "0123456789abcdef0123456789abcdef";
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    mbedtls_aes_setkey_dec(&aes_ctx, key, 256);
    mbedtls_aes_crypt_ecb(&aes_ctx, MBEDTLS_AES_DECRYPT, buffer, buffer);
    mbedtls_aes_free(&aes_ctx);
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

    // Seed the random number generator
    srand(time(NULL));

    // Generate and convert to hex 10 random numbers
    unsigned char buffer[10][16];
    int random_numbers[10];
    char hex_numbers[10][9]; // 8 characters + null-terminator

    uint64_t total_hexing_start_time = esp_timer_get_time();

    for (int i = 0; i < 10; i++) {
        random_numbers[i] = rand();
        intToHex(random_numbers[i], hex_numbers[i]);
    }

    uint64_t total_hexing_end_time = esp_timer_get_time();

    uint64_t total_encryption_start_time = esp_timer_get_time();

    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Random Number %d before Encryption: %s", i + 1, hex_numbers[i]);
        memcpy(buffer[i], hex_numbers[i], sizeof(hex_numbers[i]));
        encryptBuffer(buffer[i]);
    }

    uint64_t total_encryption_end_time = esp_timer_get_time();

    // Create and write the encrypted buffers to the file
    const char *file_path = "/littlefs/Rajath.txt";
    FILE *file = fopen(file_path, "wb");
    if (file) {
        fwrite(buffer, sizeof(buffer), 1, file);
        fclose(file);
    }

    // Read the encrypted buffers from the file
    file = fopen(file_path, "rb");
    if (file) {
        fread(buffer, sizeof(buffer), 1, file);
        fclose(file);
    }

    // Decrypt the buffers, convert hex back to int, and calculate total decryption time
    uint64_t total_dehexing_start_time = esp_timer_get_time();
    uint64_t total_decryption_start_time = 0; // Initialize to 0 for now

    for (int i = 0; i < 10; i++) {
        decryptBuffer(buffer[i]);
        char hex_num[9];
        memcpy(hex_num, buffer[i], sizeof(hex_num));
        hex_num[sizeof(hex_num) - 1] = '\0';
        int decrypted_number = hexToInt(hex_num);
        ESP_LOGI(TAG, "Decrypted Number %d: %d", i + 1, decrypted_number);
    }

    uint64_t total_dehexing_end_time = esp_timer_get_time();
    uint64_t total_decryption_end_time = esp_timer_get_time();

    // Calculate and print times
    ESP_LOGI(TAG, "Total Hexing time for 10 numbers: %lld microseconds", total_hexing_end_time - total_hexing_start_time);
    ESP_LOGI(TAG, "Total Encryption time for 10 numbers: %lld microseconds", total_encryption_end_time - total_encryption_start_time);
    ESP_LOGI(TAG, "Total Dehexing time for 10 numbers: %lld microseconds", total_dehexing_end_time - total_dehexing_start_time);
    ESP_LOGI(TAG, "Total Decryption time for 10 numbers: %lld microseconds", total_decryption_end_time - total_decryption_start_time);
}