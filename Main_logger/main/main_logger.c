#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <esp_spi_flash.h>
#include <sys/time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include "esp_err.h"
#include <sys/unistd.h>
#include <sys/stat.h>

#include "ulog_sqlite.h"

static const char *TAG = "sqlite_example";
FILE *myFile;

#define BUF_SIZE 4096
#define DISPLAY_BUFFER_SIZE 4096

char display_buffer[DISPLAY_BUFFER_SIZE];
size_t buffer_offset = 0;


// Implement file read logic suitable for ESP-IDF
int32_t read_fn_wctx(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len) {
    fseek(myFile, pos, SEEK_SET);
    size_t ret = fread(buf, 1, len, myFile);
    if (ret != len)
        return DBLOG_RES_READ_ERR;
    return ret;
}

int32_t read_fn_rctx(struct dblog_read_context *ctx, void *buf, uint32_t pos, size_t len) {
    if (fseek(myFile, pos, SEEK_SET))
        return DBLOG_RES_SEEK_ERR;
    size_t ret = fread(buf, 1, len, myFile);
    if (ret != len)
        return DBLOG_RES_READ_ERR;
    return ret;
}

// Implement file write logic suitable for ESP-IDF
int32_t write_fn(struct dblog_write_context *ctx, void *buf, uint32_t pos, size_t len) {
    fseek(myFile, pos, SEEK_SET);
    size_t ret = fwrite(buf, 1, len, myFile);
    if (ret != len)
        return DBLOG_RES_ERR;
    if (fflush(myFile))
        return DBLOG_RES_FLUSH_ERR;

    return ret;
}

// Implement file flush logic suitable for ESP-IDF
int flush_fn(struct dblog_write_context *ctx) {
    return DBLOG_RES_OK;
}

int16_t read_int16(const byte *ptr) {
  return (*ptr << 8) | ptr[1];
}

int32_t read_int32(const byte *ptr) {
  int32_t ret;
  ret  = ((int32_t)*ptr++) << 24;
  ret |= ((int32_t)*ptr++) << 16;
  ret |= ((int32_t)*ptr++) << 8;
  ret |= *ptr;
  return ret;
}

int64_t read_int64(const byte *ptr) {
  int64_t ret;
  ret  = ((int64_t)*ptr++) << 56;
  ret |= ((int64_t)*ptr++) << 48;
  ret |= ((int64_t)*ptr++) << 40;
  ret |= ((int64_t)*ptr++) << 32;
  ret |= ((int64_t)*ptr++) << 24;
  ret |= ((int64_t)*ptr++) << 16;
  ret |= ((int64_t)*ptr++) << 8;
  ret |= *ptr;
  return ret;
}

int pow10(int8_t len) {
  return (len == 3 ? 1000 : (len == 2 ? 100 : (len == 1 ? 10 : 1)));
}

// Function to append formatted content to the display buffer
void append_to_display_buffer(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int written = vsnprintf(display_buffer + buffer_offset, DISPLAY_BUFFER_SIZE - buffer_offset, format, args);
    va_end(args);
    
    if (written >= 0 && written < DISPLAY_BUFFER_SIZE - buffer_offset) {
        buffer_offset += written;
    }
}

// Function to clear the display buffer
void clear_display_buffer() {
    memset(display_buffer, 0, sizeof(display_buffer));
    buffer_offset = 0;
}

void display_row(struct dblog_read_context *ctx) {
    char formatted_row[256];  // Adjust size as needed
    
    int i = 0;
    do {
        uint32_t col_type;
        const byte *col_val = (const byte *)dblog_read_col_val(ctx, i, &col_type);
        if (!col_val) {
            if (i == 0)
                append_to_display_buffer("Error reading value");
            append_to_display_buffer("\n");
            return;
        }
        
        switch (col_type) {
            case 0:
                append_to_display_buffer("null");
                break;
            case 1:
                append_to_display_buffer("%2d", *((int8_t *)col_val));
                break;
            case 2: {
                int16_t ival = read_int16(col_val);
                append_to_display_buffer("%2d", ival);
                break;
            }
            case 4: {
                int32_t ival = read_int32(col_val);
                append_to_display_buffer("%4ld", ival);
                break;
            }
            // Add cases for int64_t and double if needed
            default: {
                uint32_t col_len = dblog_derive_data_len(col_type);
                for (int j = 0; j < col_len; j++) {
                    if (col_type % 2)
                        append_to_display_buffer("%c", col_val[j]);
                    else {
                        append_to_display_buffer("%d ", col_val[j]);
                    }
                }
            }
        }
        
        if (i == 0) {
            append_to_display_buffer("  |");
        }
        
        if (i > 0) {
            append_to_display_buffer("  ");
        }
    } while (++i);
    
    append_to_display_buffer("\n");
}



void log_random_data() {
    const char *db_path = "/spiffs/raj.DB";
    myFile = fopen(db_path, "w+b");
    int num_recs = 10;

    if (myFile) {
        byte ctx_buf[BUF_SIZE]; // Use a stack-allocated buffer
        struct dblog_write_context ctx;
        ctx.buf = ctx_buf;
        ctx.col_count = 2; // For ID and Value columns
        ctx.page_resv_bytes = 0;
        ctx.page_size_exp = 12;
        ctx.max_pages_exp = 0;
        ctx.read_fn = read_fn_wctx;
        ctx.flush_fn = flush_fn;
        ctx.write_fn = write_fn;

        int res = dblog_write_init(&ctx);
        if (!res) {
            for (int i = 1; i <= num_recs; i++) {
                int value = rand() % 1000;
                res = dblog_set_col_val(&ctx, 0, DBLOG_TYPE_INT, &i, sizeof(int));
                if (!res) {
                    res = dblog_set_col_val(&ctx, 1, DBLOG_TYPE_INT, &value, sizeof(int));
                    if (!res) {
                        res = dblog_append_empty_row(&ctx);
                        if (res) {
                            ESP_LOGI(TAG, "Error appending row: %d", res);
                            break;
                        }
                    } else {
                        ESP_LOGI(TAG, "Error setting value column: %d", res);
                        break;
                    }
                } else {
                    ESP_LOGI(TAG, "Error setting ID column: %d", res);
                    break;
                }
            }
            if (!res) {
                res = dblog_finalize(&ctx);
                if (res) {
                    ESP_LOGI(TAG, "Error finalizing database: %d", res);
                } else {
                    ESP_LOGI(TAG, "Logging completed");
                }
            }
        } else {
            ESP_LOGI(TAG, "Error initializing database: %d", res);
        }

        fclose(myFile);
    } else {
        ESP_LOGI(TAG, "Failed to open database file");
    } 

}

void readRecordsFromFile(const char *filename) {
    myFile = fopen(filename, "r");
    if (myFile == NULL) {
        ESP_LOGE(TAG, "Failed to open file %s for reading", filename);
        return;
    }

    byte ctx_buf[BUF_SIZE];
    struct dblog_read_context ctx;
    ctx.buf = ctx_buf;
    ctx.read_fn = read_fn_rctx;
    ctx.page_size_exp = 9;

    int res = dblog_read_init(&ctx);

    // Position the cursor at the last record
    if (dblog_read_last_row(&ctx) != DBLOG_RES_OK) {
        ESP_LOGE(TAG, "Error positioning cursor at last row");
    }

    while (res != DBLOG_RES_ERR) {
        res = dblog_read_prev_row(&ctx); // Move to previous row
        if (res == DBLOG_RES_OK) {
            display_row(&ctx);
        } else if (res == DBLOG_RES_ERR) {
            // Error reading previous row
            ESP_LOGE(TAG, "Error reading previous row");
        } else if (res == DBLOG_RES_NOT_FOUND) {
            // No more records to read
            break;
        }
    }

    fclose(myFile);

    ESP_LOGI(TAG, "Reading complete from %s", filename);
}

void app_main() {
    

    ESP_LOGI(TAG, "Initializing SPIFFS");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Successfully mounted SPIFFS");

    log_random_data();
    readRecordsFromFile("/spiffs/raj.DB");
    printf("%s", display_buffer);
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    
}









