#include <stdio.h>
#include <stdlib.h>

// Function to perform delta compression on an array of numbers
double* deltaCompress(const double* original, size_t size) {
    double* compressed = (double*)malloc(size * sizeof(double));
    if (compressed == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    compressed[0] = original[0];

    for (size_t i = 1; i < size; ++i) {
        compressed[i] = original[i] - original[i - 1];
    }

    return compressed;
}

// Function to perform Run-Length Encoding on an array of numbers
double* runLengthEncode(const double* input, size_t size, size_t* resultSize) {
    double* compressed = (double*)malloc(2 * size * sizeof(double));
    if (compressed == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
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
void writeCSV(const char* filename, const double* data, size_t size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; ++i) {
        fprintf(file, "%.2lf", data[i]);
        if (i < size - 1) {
            fprintf(file, ",");
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

// Function to process a single file for delta + RLE compression
void process_file(const char* inputFilename) {
    // Open the input CSV file
    FILE* inputFile = fopen(inputFilename, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        return;
    }

    double original[MAX_VALUES];
    size_t num_values = 0;
    while (fscanf(inputFile, "%lf,", &original[num_values]) != EOF) {
        num_values++;
        if (num_values >= MAX_VALUES) {
            printf("Error: Too many values in input file %s.\n", inputFilename);
            fclose(inputFile);
            return;
        }
    }
    fclose(inputFile);

    // Perform delta compression
    double* deltaCompressed = deltaCompress(original, num_values);

    // Save delta compressed data
    char delta_filename[256];
    snprintf(delta_filename, sizeof(delta_filename), "%s_delta_comp.csv", inputFilename);
    writeCSV(delta_filename, deltaCompressed, num_values);

    // Perform Run-Length Encoding on the delta compressed result
    size_t rleSize;
    double* rleCompressed = runLengthEncode(deltaCompressed, num_values, &rleSize);

    // Write Run-Length Encoded results to output CSV
    char comp_filename[256];
    snprintf(comp_filename, sizeof(comp_filename), "%s_comp.csv", inputFilename);
    writeCSV(comp_filename, rleCompressed, rleSize);

    // Free allocated memory
    free(deltaCompressed);
    free(rleCompressed);

    printf("Delta + RLE compressed results for %s saved to %s and %s.\n", inputFilename, delta_filename, comp_filename);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file1> <input_file2> ... <input_fileN>\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        process_file(argv[i]);
    }

    return 0;
}
