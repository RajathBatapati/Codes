#include <stdio.h>
#include <stdint.h>
#include <math.h>

// Define the structure of a 32-bit floating-point number
typedef union {
    float f;
    uint32_t u;
} Float32;

// Define the structure of a 16-bit floating-point number
typedef union {
    uint16_t u;
    struct {
        uint16_t mantissa : 10;
        uint16_t exponent : 5;
        uint16_t sign : 1;
    } parts;
} Float16;

// Define the structure of an 8-bit floating-point number
typedef union {
    uint8_t u;
    struct {
        uint8_t mantissa : 4;
        uint8_t exponent : 3;
        uint8_t sign : 1;
    } parts;
} Float8;

// Function to print the binary representation of a number
void print_binary(uint32_t number, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        printf("%u", (number >> i) & 1);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");
}

// Function to convert float32 to float16
uint16_t float32_to_float16(float f) {
    Float32 in;
    in.f = f;
    
    Float16 out;
    uint32_t sign = (in.u >> 31) & 0x1;
    int32_t exponent = ((in.u >> 23) & 0xFF) - 127 + 15; // Adjust exponent bias
    uint32_t mantissa = (in.u & 0x7FFFFF);

    if (exponent <= 0) { 
        if (exponent < -10) { 
            out.u = 0; // Too small to be represented as a float16
        } else {
            mantissa = (mantissa | 0x800000) >> (1 - exponent); // Handle subnormal numbers
            out.u = (sign << 15) | (mantissa >> 13);
        }
    } else if (exponent >= 31) {
        out.u = (sign << 15) | (0x1F << 10); // Too large to be represented as a float16, set to infinity
    } else {
        out.u = (sign << 15) | (exponent << 10) | (mantissa >> 13);
    }

    return out.u;
}

// Function to convert float32 to float8
uint8_t float32_to_float8(float f) {
    Float32 in;
    in.f = f;
    
    Float8 out;
    uint32_t sign = (in.u >> 31) & 0x1;
    int32_t exponent = ((in.u >> 23) & 0xFF) - 127 + 3; // Adjust exponent bias
    uint32_t mantissa = (in.u & 0x7FFFFF);

    if (exponent <= 0) { 
        if (exponent < -6) { 
            out.u = 0; // Too small to be represented as a float8
        } else {
            mantissa = (mantissa | 0x800000) >> (1 - exponent); // Handle subnormal numbers
            out.u = (sign << 7) | (mantissa >> 19);
        }
    } else if (exponent >= 7) {
        out.u = (sign << 7) | (0x7 << 4); // Too large to be represented as a float8, set to infinity
    } else {
        out.u = (sign << 7) | (exponent << 4) | (mantissa >> 19);
    }

    return out.u;
}

// Function to check if a float32 can be represented as float16
int can_be_represented_as_float16(float f) {
    Float32 in;
    in.f = f;
    int32_t exponent = ((in.u >> 23) & 0xFF) - 127 + 15;
    return (exponent >= -10 && exponent <= 31);
}

// Function to check if a float32 can be represented as float8
int can_be_represented_as_float8(float f) {
    Float32 in;
    in.f = f;
    int32_t exponent = ((in.u >> 23) & 0xFF) - 127 + 3;
    return (exponent >= -6 && exponent <= 7);
}

int main() {
    float num;
    printf("Enter a float number: ");
    scanf("%f", &num);

    if (can_be_represented_as_float8(num)) {
        uint8_t result8 = float32_to_float8(num);
        printf("Float %.6f can be represented as float8:\n", num);
        print_binary(result8, 8);
    } else if (can_be_represented_as_float16(num)) {
        uint16_t result16 = float32_to_float16(num);
        printf("Float %.6f can be represented as float16:\n", num);
        print_binary(result16, 16);
    } else {
        printf("Float %.6f cannot be represented as float8 or float16. Keeping as float32.\n", num);
        Float32 f32;
        f32.f = num;
        print_binary(f32.u, 32);
    }

    return 0;
}
