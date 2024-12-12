#include "Converters.h"
#include <stdio.h>

// Function to print the binary representation of a number
void print_binary(int32_t number, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        printf("%u", (number >> i) & 1);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");
}

// Function to check if a signed number can be represented as an 8-bit signed integer
int convert_32bit_to_8bit_signed(int32_t number, int8_t* result) {
    if (number >= -128 && number <= 127) {
        *result = (int8_t)number;
        return 1;
    }
    return 0;
}

// Function to check if a signed number can be represented as a 12-bit signed integer
int convert_32bit_to_12bit_signed(int32_t number, int16_t* result) {
    if (number >= -2048 && number <= 2047) {
        *result = (int16_t)number;
        return 1;
    }
    return 0;
}

// Function to check if a signed number can be represented as a 16-bit signed integer
int convert_32bit_to_16bit_signed(int32_t number, int16_t* result) {
    if (number >= -32768 && number <= 32767) {
        *result = (int16_t)number;
        return 1;
    }
    return 0;
}
