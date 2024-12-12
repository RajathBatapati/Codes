#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUES 5000

// Function to perform XOR operation on an array of numbers
void xorCompress(const double* original, int* compressed, size_t size) {
    for (size_t i = 0; i < size - 1; ++i) {
        compressed[i] = (int)(original[i] * 1000) ^ (int)(original[i + 1] * 1000);
    }
}

// Function to perform delta compression on an array of numbers
void deltaCompress(const int* original, int* compressed, size_t size) {
    compressed[0] = original[0];
    for (size_t i = 1; i < size; ++i) {
        compressed[i] = original[i] - original[i - 1];
    }
}

// Function to write data to a CSV file
void writeCSV(const char* filename, const int* data, size_t size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; ++i) {
        fprintf(file, "%d\n", data[i]);
    }

    fclose(file);
}

// Function to read data from a CSV file
double* readCSV(const char* filename, size_t* dataSize) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    double* data = (double*)malloc(MAX_VALUES * sizeof(double));
    if (data == NULL) {
        perror("Error allocating memory");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    while (fscanf(file, "%lf,", &data[i]) != EOF) {
        i++;
        if (i >= MAX_VALUES) {
            data = realloc(data, (i + 1000) * sizeof(double));
            if (data == NULL) {
                perror("Error reallocating memory");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(file);

    *dataSize = i;
    return data;
}

// Function to generate the output filename based on the input filename
void getOutputFilename(char* outputFilename, const char* inputFilename, const char* suffix) {
    const char* dot = strrchr(inputFilename, '.');
    if (dot == NULL) {
        snprintf(outputFilename, 256, "%s%s.csv", inputFilename, suffix);
    } else {
        size_t baseLength = dot - inputFilename;
        snprintf(outputFilename, baseLength + strlen(suffix) + 1, "%s", inputFilename);
        snprintf(outputFilename + baseLength, 256 - baseLength, "%s.csv", suffix);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_file1> <input_file2> ... <input_fileN>\n", argv[0]);
        return 1;
    }

    for (int k = 1; k < argc; ++k) {
        size_t dataSize;
        double* original = readCSV(argv[k], &dataSize);

        // Perform XOR compression
        int* xorCompressed = (int*)malloc((dataSize - 1) * sizeof(int));
        if (xorCompressed == NULL) {
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        xorCompress(original, xorCompressed, dataSize);

        // Save XOR compressed data
        char xor_filename[256];
        getOutputFilename(xor_filename, argv[k], "_xor");
        writeCSV(xor_filename, xorCompressed, dataSize - 1);

        // Perform delta compression on XOR compressed data
        int* deltaCompressed = (int*)malloc((dataSize - 1) * sizeof(int));
        if (deltaCompressed == NULL) {
            perror("Error allocating memory");
            exit(EXIT_FAILURE);
        }
        deltaCompress(xorCompressed, deltaCompressed, dataSize - 1);

        // Save delta compressed data
        char delta_filename[256];
        getOutputFilename(delta_filename, argv[k], "_comp");
        writeCSV(delta_filename, deltaCompressed, dataSize - 1);

        // Free allocated memory
        free(original);
        free(xorCompressed);
        free(deltaCompressed);
    }

    return 0;
}
