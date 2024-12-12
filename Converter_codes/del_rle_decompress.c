#include <stdio.h>
#include <stdlib.h>

// Function to perform delta decompression on an array of numbers
int* deltaDecompress(const int* compressed, size_t size) {
    int* decompressed = (int*)malloc(size * sizeof(int));
    decompressed[0] = compressed[0]; // The first value remains unchanged

    for (size_t i = 1; i < size; ++i) {
        decompressed[i] = decompressed[i - 1] + compressed[i]; // Add delta to the previous value
    }

    return decompressed;
}

// Function to write data to a CSV file
void writeCSV(const char* filename, const int* data, size_t size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; ++i) {
        fprintf(file, "%d,\n", data[i]);
    }

    fclose(file);
}

int main() {
    // Read delta compressed data from input_delta.csv
    const char* deltaInputFilename = "output_delta.csv";
    FILE* deltaInputFile = fopen(deltaInputFilename, "r");
    if (deltaInputFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int deltaCompressed[1731]; // Assuming delta compressed data size won't exceed this
    size_t deltaSize = 0;
    while (fscanf(deltaInputFile, "%d,\n", &deltaCompressed[deltaSize]) == 1) {
        deltaSize++;
    }
    fclose(deltaInputFile);

    // Perform delta decompression
    int* deltaDecompressed = deltaDecompress(deltaCompressed, deltaSize);

    // Write decompressed data to decompressed.csv
    writeCSV("decompressed.csv", deltaDecompressed, deltaSize);

    // Free allocated memory
    free(deltaDecompressed);

    return 0;
}
 