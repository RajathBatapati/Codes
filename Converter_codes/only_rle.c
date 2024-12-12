#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to perform Run-Length Encoding on an array of numbers
int* runLengthEncode(const int* input, size_t size, size_t* resultSize) {
    int* compressed = (int*)malloc(2 * size * sizeof(int)); // Allocate memory for the worst-case scenario
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
        fprintf(file, "%d\n", data[i]); // Each number on a new line
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

    int* data = (int*)malloc(4000 * sizeof(int)); // Allocate memory for the expected number of data points
    if (data == NULL) {
        perror("Error allocating memory");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    char line[256];

    // Read the data, assuming one integer per line
    while (fgets(line, sizeof(line), file)) {
        data[i] = atoi(line);
        i++;
        if (i >= 4000) { // Ensure we don't read more than the allocated size
            data = realloc(data, (i + 1000) * sizeof(int)); // Reallocate memory if needed
            if (data == NULL) {
                perror("Error reallocating memory");
                fclose(file);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(file);

    *dataSize = i; // Actual size of the data read
    return data;
}

// Function to generate the output filename based on the input filename
void getOutputFilename(char* outputFilename, const char* inputFilename) {
    const char* dot = strrchr(inputFilename, '.');
    if (dot == NULL) {
        snprintf(outputFilename, 256, "%s_comp.csv", inputFilename);
    } else {
        size_t baseLength = dot - inputFilename;
        snprintf(outputFilename, baseLength + 6, "%s", inputFilename);
        snprintf(outputFilename + baseLength, 256 - baseLength, "_comp.csv");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1.csv> <file2.csv> ... <fileN.csv>\n", argv[0]);
        return 1;
    }

    for (int k = 1; k < argc; ++k) {
        size_t dataSize;
        int* original = readCSV(argv[k], &dataSize);

        // Perform Run-Length Encoding
        size_t rleSize;
        int* rleCompressed = runLengthEncode(original, dataSize, &rleSize);

        // Create output filename
        char outputFilename[256];
        getOutputFilename(outputFilename, argv[k]);

        // Write RLE compressed results to the output file
        writeCSV(outputFilename, rleCompressed, rleSize);

        // Free allocated memory
        free(original);
        free(rleCompressed);
    }

    return 0;
}
