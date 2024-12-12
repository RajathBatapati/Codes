#include <stdio.h>
#include "converters.h"

int main() {
    float value;
    uint16_t result16;
    uint8_t result8;

    printf("Enter a float32 number: ");
    if (scanf("%f", &value) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    if (can_represent_as_float16(value, &result16)) {
        printf("The number %f can be represented as a float16.\n", value);
        printf("float16 (binary): ");
        print_binary(result16, 16);
        printf("float16 (hex): 0x%04X\n", result16);
    } else if (can_represent_as_float8(value, &result8)) {
        printf("The number %f can be represented as a float8.\n", value);
        printf("float8 (binary): ");
        print_binary(result8, 8);
        printf("float8 (hex): 0x%02X\n", result8);
    } else {
        printf("Sorry, the number %f cannot be represented as a float16 or float8.\n", value);
    }

    return 0;
}
