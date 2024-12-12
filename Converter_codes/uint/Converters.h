#ifndef UINT_H
#define UINT_H

#include <stdint.h>
#include <stdbool.h>

// Struct for 8-bit number
typedef struct {
    uint8_t data;
} my_8bit_num_t;

// Struct for 16-bit number
typedef struct {
    uint16_t data;
} my_16bit_num_t;

// Struct for 32-bit number
typedef struct {
    uint32_t data;
} my_32bit_num_t;

// Struct for 64-bit number
typedef struct {
    uint64_t data;
} my_64bit_num_t;

// Struct for 24-bit number
typedef struct {
    uint8_t bytes[3];
} my_24bit_num_t;

// Struct for 40-bit number
typedef struct {
    uint8_t bytes[5];
} my_40bit_num_t;

// Struct for 48-bit number
typedef struct {
    uint8_t bytes[6];
} my_48bit_num_t;

// Struct for 56-bit number
typedef struct {
    uint8_t bytes[7];
} my_56bit_num_t;

// Enum for error codes
typedef enum {
    NO_ERROR = 0,
    VALUE_EXCEEDS_8BIT_LIMIT,
    VALUE_EXCEEDS_16BIT_LIMIT,
    VALUE_EXCEEDS_24BIT_LIMIT,
    VALUE_EXCEEDS_40BIT_LIMIT,
    VALUE_EXCEEDS_48BIT_LIMIT,
    VALUE_EXCEEDS_56BIT_LIMIT,
    VALUE_EXCEEDS_32BIT_LIMIT,
    VALUE_EXCEEDS_64BIT_LIMIT,
    FILE_WRITE_ERROR,
    UNKNOWN_FUNCTION
} error_code_t;

// Function to handle 8-bit data
error_code_t handle_8_bit_data(uint8_t reading, const char *function_name);

// Function to handle 16-bit data
error_code_t handle_16_bit_data(uint16_t reading, const char *function_name);

// Function to handle 24-bit data
error_code_t handle_24_bit_data(uint32_t reading, const char *function_name);

// Function to handle 32-bit data
error_code_t handle_32_bit_data(uint32_t reading, const char *function_name);

// Function to handle 40-bit data
error_code_t handle_40_bit_data(uint64_t reading, const char *function_name);

// Function to handle 48-bit data
error_code_t handle_48_bit_data(uint64_t reading, const char *function_name);

// Function to handle 56-bit data
error_code_t handle_56_bit_data(uint64_t reading, const char *function_name);

// Function to handle 64-bit data
error_code_t handle_64_bit_data(uint64_t reading, const char *function_name);

#endif // UINT_H
