#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "Converters.h"



// Function to handle 24-bit data
error_code_t handle_24_bit_data(uint32_t reading, const char *function_name) {
    if (reading > 0xFFFFFF) {
        printf("Error in function %s: Value %lu exceeds 24-bit limit\n", function_name, reading);
        return NO_ERROR;
    }

    printf("Valid 24-bit value received: %lu in function %s\n", reading, function_name);
    return NO_ERROR;
}

// Function to write a 24-bit number to a text file
int write_24bit_number_to_file(const char *filename, my_24bit_num_t num) {
    FILE *file = fopen(filename, "w");  // Open file in write mode
    if (!file) {
        perror("Failed to open file");
        return -1; // Return an error code if file opening fails
    }

    // Write the 24-bit number as hexadecimal to the file
    fprintf(file, "%02X%02X%02X", num.bytes[0], num.bytes[1], num.bytes[2]);
    fclose(file); // Close the file
    return 0; // Return success
}

int main() {
    uint32_t input;
    printf("Enter a 24-bit number (in hexadecimal): ");
    scanf("%x", &input);

    my_24bit_num_t num = {{(input >> 16) & 0xFF, (input >> 8) & 0xFF, input & 0xFF}};
    error_code_t result = handle_24_bit_data(input, "main");
    if (result != NO_ERROR) {
        printf("An error occurred: %d\n", result);
        return -1;
    }

    if (write_24bit_number_to_file("output.txt", num) != 0) {
        printf("Failed to write to file\n");
        return -1;
    }

    printf("The number was written successfully to 'output.txt'\n");
    return 0;
}
