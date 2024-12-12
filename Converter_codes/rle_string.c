#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to perform Run-Length Encoding on an array of strings
char** runLengthEncode(const char** input, size_t size, size_t* resultSize) {
    char** compressed = (char**)malloc(2 * size * sizeof(char*)); // Allocate memory for the worst-case scenario
    if (compressed == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    size_t i, j = 0;
    for (i = 0; i < size; ++i) {
        int count = 1;
        while (i < size - 1 && strcmp(input[i], input[i + 1]) == 0) {
            count++;
            i++;
        }

        compressed[j] = strdup(input[i]);
        compressed[j + 1] = (char*)malloc(12); // Allocate memory for the count string
        snprintf(compressed[j + 1], 12, "%d", count);
        j += 2;
    }

    *resultSize = j;
    return compressed;
}

// Function to write data to a CSV file
void writeCSV(const char* filename, char** data, size_t size) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; i += 2) {
        fprintf(file, "%s\n%s\n", data[i], data[i + 1]); // Write the string and its count on separate lines
    }

    fclose(file);
}

// Function to read data from a CSV file
char** readCSV(const char* filename, size_t* dataSize) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char** data = (char**)malloc(4000 * sizeof(char*)); // Allocate memory for the expected number of data points
    if (data == NULL) {
        perror("Error allocating memory");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    char line[256];

    // Read the data, assuming one string per line
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        data[i] = strdup(line);
        i++;
        if (i >= 4000) { // Ensure we don't read more than the allocated size
            char** temp = realloc(data, (i + 1000) * sizeof(char*)); // Reallocate memory if needed
            if (temp == NULL) {
                for (size_t j = 0; j < i; j++) {
                    free(data[j]);
                }
                free(data);
                perror("Error reallocating memory");
                fclose(file);
                exit(EXIT_FAILURE);
            }
            data = temp;
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
        char** original = readCSV(argv[k], &dataSize);

        // Perform Run-Length Encoding
        size_t rleSize;
        char** rleCompressed = runLengthEncode((const char**)original, dataSize, &rleSize);

        // Create output filename
        char outputFilename[256];
        getOutputFilename(outputFilename, argv[k]);

        // Write RLE compressed results to the output file
        writeCSV(outputFilename, rleCompressed, rleSize);

        // Free allocated memory
        for (size_t i = 0; i < dataSize; i++) {
            free(original[i]);
        }
        free(original);
        for (size_t i = 0; i < rleSize; i++) {
            free(rleCompressed[i]);
        }
        free(rleCompressed);
    }

    return 0;
}
