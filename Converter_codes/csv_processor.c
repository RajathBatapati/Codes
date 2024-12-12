#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 4000
#define COLUMN_COUNT 69

void split_csv(const char *input_file) {
    FILE *input_fp = fopen(input_file, "r");
    if (!input_fp) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    FILE *output_fp[COLUMN_COUNT];
    char output_filename[100];

    // Open output files
    for (int i = 0; i < COLUMN_COUNT; ++i) {
        snprintf(output_filename, sizeof(output_filename), "column_%d.csv", i + 1);
        output_fp[i] = fopen(output_filename, "w");
        if (!output_fp[i]) {
            perror("Error opening output file");
            for (int j = 0; j < i; ++j) {
                fclose(output_fp[j]);
            }
            fclose(input_fp);
            exit(EXIT_FAILURE);
        }
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), input_fp)) {
        char *token;
        int column_index = 0;
        token = strtok(line, ",");

        while (token) {
            if (column_index < COLUMN_COUNT) {
                fprintf(output_fp[column_index], "%s\n", token);
            }
            token = strtok(NULL, ",");
            column_index++;
        }

        if (column_index != COLUMN_COUNT) {
            fprintf(stderr, "Warning: Expected %d columns, but found %d columns in line: %s\n", COLUMN_COUNT, column_index, line);
        }
    }

    // Close output files
    for (int i = 0; i < COLUMN_COUNT; ++i) {
        fclose(output_fp[i]);
    }

    fclose(input_fp);
}

int main() {
    const char *input_file = "ME237.csv";
    printf("Processing file: %s\n", input_file);
    split_csv(input_file);
    printf("Processing completed.\n");
    return EXIT_SUCCESS;
}
