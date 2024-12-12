#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUES 5000

// Function to perform Run-Length Encoding on an array of numbers
int* runLengthEncode(const int* input, size_t size, size_t* resultSize) {
    int* compressed = (int*)malloc(2 * size * sizeof(int));
    size_t i, j = 0;

    for (i = 0; i < size; ++i) {
        int count = 1;
        while (i < size - 1 && input[i] == input[i + 1]) {
            count++;
            i++;
        }

        compressed[j++] = input[i];
        compressed[j++] = count;
    }

    *resultSize = j;
    return compressed;
}

// Function to write data to a CSV file
void writeCSV(const char* filename, const int* data, size_t size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; ++i) {
        fprintf(file, "%d\n", data[i]);  // Each number on a new line
    }

    fclose(file);
}

int main() {
    // Open the input CSV file
    FILE *input_file = fopen("column_10.csv", "r");
    if (input_file == NULL) {
        printf("Error opening the input file.\n");
        return 1;
    }

    // Read values from the input CSV file 
    int int_values[MAX_VALUES];
    int num_values = 0;

    char line[256];
    while (fgets(line, sizeof(line), input_file)) {
        if (num_values >= MAX_VALUES) {
            printf("Error: Too many values in input file.\n");
            return 1;
        }
        char *token = strtok(line, ",");
        float value = atof(token);
        int_values[num_values] = (int)(value * 1000);
        num_values++;
    }

    // Close the input file
    fclose(input_file);

    // Perform XOR operation between consecutive pairs 
    int xor_results[MAX_VALUES - 1];
    for (int i = 0; i < num_values - 1; i++) {
        xor_results[i] = int_values[i] ^ int_values[i + 1];
    }

    // Perform Run-Length Encoding on the XOR results
    size_t rleSize;
    int* rleCompressed = runLengthEncode(xor_results, num_values - 1, &rleSize);

    // Open the output CSV file
    const char* outputFilename = "column_10_comp.csv";

    // Write the RLE results to the output file
    writeCSV(outputFilename, rleCompressed, rleSize);

    // Free allocated memory
    free(rleCompressed);

    printf("Results have been saved to column_23_comp.csv.\n");

    return 0;
}
