#include <stdio.h>
#include "Converters.h"

int main() {
    int32_t signed_number;
    int8_t result8_signed;
    int16_t result12_signed, result16_signed;

    printf("Enter a 32-bit signed integer: ");
    if (scanf("%d", &signed_number) != 1) {
        printf("Invalid input.\n");
        return 1; // Exit with error code
    }

    // Check and convert to 8-bit signed representation
    if (convert_32bit_to_8bit_signed(signed_number, &result8_signed)) {
        printf("The number %d can be represented as an 8-bit signed integer.\n", result8_signed);
        printf("8-bit signed binary representation: ");
        print_binary((int32_t)result8_signed, 8);
        printf("Decimal value: %d\n", result8_signed);
    } else if (convert_32bit_to_12bit_signed(signed_number, &result12_signed)) {
        printf("The number %d can be represented as a 12-bit signed integer.\n", result12_signed);
        printf("12-bit signed binary representation: ");
        print_binary((int32_t)result12_signed, 12);
        printf("Decimal value: %d\n", result12_signed);
    } else if (convert_32bit_to_16bit_signed(signed_number, &result16_signed)) {
        printf("The number %d can be represented as a 16-bit signed integer.\n", result16_signed);
        printf("16-bit signed binary representation: ");
        print_binary((int32_t)result16_signed, 16);
        printf("Decimal value: %d\n", result16_signed);
    } else {
        printf("Sorry, the number %d cannot be represented as an 8-bit, 12-bit, or 16-bit signed integer.\n", signed_number);
    }

    return 0;
}
