#ifndef SIGNED_CONVERTERS_H
#define SIGNED_CONVERTERS_H

#include <stdint.h>

// Function to print the binary representation of a number
void print_binary(int32_t number, int bits);

// Function to check if a signed number can be represented as an 8-bit signed integer
int convert_32bit_to_8bit_signed(int32_t number, int8_t* result);

// Function to check if a signed number can be represented as a 12-bit signed integer
int convert_32bit_to_12bit_signed(int32_t number, int16_t* result);

// Function to check if a signed number can be represented as a 16-bit signed integer
int convert_32bit_to_16bit_signed(int32_t number, int16_t* result);

#endif // SIGNED_CONVERTERS_H
