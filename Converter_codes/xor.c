#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUES 5000

int main() {
    // Open the input CSV file
    FILE *input_file = fopen("column_11.csv", "r");
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
        //printf("Read line: %s", line);
        char *token = strtok(line, ",");
        float value = atof(token);
        int_values[num_values] = (int)(value * 1000);
        num_values++;
    }

    // Close the input file
    fclose(input_file);

    

    // Perform XOR operation between consecutive pairs 
    int results[MAX_VALUES - 1];
    for (int i = 0; i < num_values - 1; i++) {
        results[i] = int_values[i] ^ int_values[i + 1];
    }

    // Open the output CSV file
    FILE *output_file = fopen("column_11_comp.csv", "w");
    if (output_file == NULL) {
        printf("Error opening the output file.\n");
        return 1;
    }

    // Write the XOR results to the output file
    for (int i = 0; i < 1731; i++) {
        fprintf(output_file, "%d\n", results[i]);
    }

    // Close the output file
    fclose(output_file);

    printf("Results have been saved to output.csv.\n");

    return 0;
}
