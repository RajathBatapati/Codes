#include <stdio.h>
#include <math.h>

// Function to perform bit packing compression
void bitPackCompression(double original[], int size, int precision) {
    int i;
    int quantized[size];
    int compressed[size];
    int bitsNeeded = 0;

    // Quantization: multiply each value by 10^precision
    for (i = 0; i < size; i++) {
        quantized[i] = (int)(original[i] * pow(10, precision));
    }

    // Find the maximum value to determine the number of bits needed
    int maxVal = quantized[0];
    for (i = 1; i < size; i++) {
        if (quantized[i] > maxVal) {
            maxVal = quantized[i];
        }
    }

    // Calculate the number of bits needed to represent the maximum value
    while (maxVal > 0) {
        bitsNeeded++;
        maxVal >>= 1;
    }

    // Bit packing compression
    for (i = 0; i < size; i++) {
        compressed[i] = quantized[i] << (sizeof(int) * 8 - bitsNeeded);
    }

    // Output the compressed values
    printf("Compressed values:\n");
    for (i = 0; i < size; i++) {
        printf("%d\n", compressed[i]);
    }
}

int main() {
    double original[] = {49.978, 50.012, 50.034, 50.031, 50.028, 50.034, 50.023};
    int size = sizeof(original) / sizeof(original[0]);
    int precision = 3; // 3 decimal places

    // Call the compression function
    bitPackCompression(original, size, precision);

    return 0;
}
