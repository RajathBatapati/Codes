#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <stdint.h>

// Function to print the binary representation of a number
void print_binary(uint32_t number, int bits);

// Functions to convert float32 to float16 and float8
uint16_t float32_to_float16(float value);
uint8_t float32_to_float8(float value);

// Functions to check if a float32 can be represented as float16 or float8
int can_represent_as_float16(float value, uint16_t* result);
int can_represent_as_float8(float value, uint8_t* result);

#endif // CONVERTERS_H
