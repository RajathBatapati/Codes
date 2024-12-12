#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VALUES 5000

// Function to perform delta compression on an array of numbers
int* deltaCompress(const int* original, size_t size) {
    int* compressed = (int*)malloc(size * sizeof(int));
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
int* runLengthEncode(const int* input, size_t size, size_t* resultSize) {
    int* compressed = (int*)malloc(2 * size * sizeof(int));
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
int* readCSV(const char* filename, size_t* dataSize) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int* data = (int*)malloc(MAX_VALUES * sizeof(int));
    if (data == NULL) {
        perror("Error allocating memory");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    while (fscanf(file, "%d,", &data[i]) != EOF) {
        i++;
        if (i >= MAX_VALUES) {
            data = realloc(data, (i + 1000) * sizeof(int));
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
        int* original = readCSV(argv[k], &dataSize);

        // Perform delta compression
        int* deltaCompressed = deltaCompress(original, dataSize);

        // Save delta compressed data
        char delta_filename[256];
        getOutputFilename(delta_filename, argv[k], "_delta_comp");
        writeCSV(delta_filename, deltaCompressed, dataSize);

        // Perform Run-Length Encoding on the delta compressed result
        size_t rleSize;
        int* rleCompressed = runLengthEncode(deltaCompressed, dataSize, &rleSize);

        // Write Run-Length Encoded results to output CSV
        char comp_filename[256];
        getOutputFilename(comp_filename, argv[k], "_comp");
        writeCSV(comp_filename, rleCompressed, rleSize);

        // Free allocated memory
        free(original);
        free(deltaCompressed);
        free(rleCompressed);
    }

    return 0;
}
