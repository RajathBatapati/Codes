#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUES 5000

int main() {
    // Open the compressed CSV file
    FILE *input_file = fopen("output.csv", "r");
    if (input_file == NULL) {
        printf("Error opening the input file.\n");
        return 1;
    }

    // Read values from the compressed CSV file 
    int int_values[MAX_VALUES];
    int num_values = 0;

    char line[256];
    while (fgets(line, sizeof(line), input_file)) {
        printf("Read line: %s", line);
        char *token = strtok(line, ",");
        int value = atoi(token);
        if (num_values < MAX_VALUES) {
            int_values[num_values] = value;
            num_values++;
        } else {
            printf("Error: Too many values in input file.\n");
            return 1;
        }
    }

    // Close the input file
    fclose(input_file);

    // Perform XOR operation between consecutive pairs 
    int results[MAX_VALUES];
    results[0] = int_values[0];
    for (int i = 1; i < num_values; i++) {
        results[i] = int_values[i] ^ results[i - 1];
    }

    // Open the decompressed CSV file
    FILE *output_file = fopen("decompressed.csv", "w");
    if (output_file == NULL) {
        printf("Error opening the output file.\n");
        return 1;
    }

    // Write the XOR results to the output file
    for (int i = 0; i < num_values; i++) {
        float result = (float)results[i] / 1000.0;
        fprintf(output_file, "%f\n", result);
    }

    // Close the output file
    fclose(output_file);

    printf("Results have been saved to decompressed.csv.\n");

    return 0;
}
