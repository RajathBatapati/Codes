#include "Converters.h"
#include <stdio.h>
#include "esp_log.h"

#define TAG "UINT_HANDLER"

error_code_t handle_8_bit_data(uint8_t reading, const char *function_name) {
    // Convert and store the value 
    my_8bit_num_t num;
    num.data = reading;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 8-bit value received: %u in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_16_bit_data(uint16_t reading, const char *function_name) {
    // Convert and store the value 
    my_16bit_num_t num;
    num.data = reading;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 16-bit value received: %u in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_24_bit_data(uint32_t reading, const char *function_name) {
    // Check if the reading can be represented in 24 bits (unsigned)
    if (reading > 0xFFFFFF) {
        // Value exceeds 24-bit unsigned limit, handle error
        ESP_LOGE(TAG, "Error in function %s: Value %lu exceeds 24-bit limit", function_name, reading);
        return VALUE_EXCEEDS_24BIT_LIMIT; // Indicate error
    }

    // Convert and store the value in the struct
    my_24bit_num_t num;
    num.bytes[0] = (reading >> 16) & 0xFF;
    num.bytes[1] = (reading >> 8) & 0xFF;
    num.bytes[2] = reading & 0xFF;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 24-bit value received: %lu in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_32_bit_data(uint32_t reading, const char *function_name) {
    // Convert and store the value
    my_32bit_num_t num;
    num.data = reading;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 32-bit value received: %lu in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_40_bit_data(uint64_t reading, const char *function_name) {
    // Check if the reading can be represented in 40 bits (unsigned)
    if (reading > 0xFFFFFFFFFF) {
        // Value exceeds 40-bit unsigned limit, handle error
        ESP_LOGE(TAG, "Error in function %s: Value %llu exceeds 40-bit limit", function_name, reading);
        return VALUE_EXCEEDS_40BIT_LIMIT; // Indicate error
    }

    // Convert and store the value in the struct
    my_40bit_num_t num;
    num.bytes[0] = (reading >> 32) & 0xFF;
    num.bytes[1] = (reading >> 24) & 0xFF;
    num.bytes[2] = (reading >> 16) & 0xFF;
    num.bytes[3] = (reading >> 8) & 0xFF;
    num.bytes[4] = reading & 0xFF;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 40-bit value received: %llu in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_48_bit_data(uint64_t reading, const char *function_name) {
    // Check if the reading can be represented in 48 bits (unsigned)
    if (reading > 0xFFFFFFFFFFFF) {
        // Value exceeds 48-bit unsigned limit, handle error
        ESP_LOGE(TAG, "Error in function %s: Value %llu exceeds 48-bit limit", function_name, reading);
        return VALUE_EXCEEDS_48BIT_LIMIT; // Indicate error
    }

    // Convert and store the value in the struct
    my_48bit_num_t num;
    num.bytes[0] = (reading >> 40) & 0xFF;
    num.bytes[1] = (reading >> 32) & 0xFF;
    num.bytes[2] = (reading >> 24) & 0xFF;
    num.bytes[3] = (reading >> 16) & 0xFF;
    num.bytes[4] = (reading >> 8) & 0xFF;
    num.bytes[5] = reading & 0xFF;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 48-bit value received: %llu in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_56_bit_data(uint64_t reading, const char *function_name) {
    // Check if the reading can be represented in 56 bits (unsigned)
    if (reading > 0xFFFFFFFFFFFFFF) {
        // Value exceeds 56-bit unsigned limit, handle error
        ESP_LOGE(TAG, "Error in function %s: Value %llu exceeds 56-bit limit", function_name, reading);
        return VALUE_EXCEEDS_56BIT_LIMIT; // Indicate error
    }

    // Convert and store the value in the struct
    my_56bit_num_t num;
    num.bytes[0] = (reading >> 48) & 0xFF;
    num.bytes[1] = (reading >> 40) & 0xFF;
    num.bytes[2] = (reading >> 32) & 0xFF;
    num.bytes[3] = (reading >> 24) & 0xFF;
    num.bytes[4] = (reading >> 16) & 0xFF;
    num.bytes[5] = (reading >> 8) & 0xFF;
    num.bytes[6] = reading & 0xFF;

    /* 
    
    write code to handle received data
    
    
    */

    ESP_LOGI(TAG, "Valid 56-bit value received: %llu in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}

error_code_t handle_64_bit_data(uint64_t reading, const char *function_name) {
    // Convert and store the value in the struct
    my_64bit_num_t num;
    num.data = reading;

    /* 
    
    write code to handle received data
    
    
    */
   
    ESP_LOGI(TAG, "Valid 64-bit value received: %llu in function %s", reading, function_name);

    return NO_ERROR; // Indicate success
}
