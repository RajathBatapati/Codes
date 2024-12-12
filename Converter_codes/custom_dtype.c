#include <stdio.h>
#include <stdint.h>
#include <stddef.h> // Include this for offsetof

// Define the struct with packed attribute
typedef struct {
    uint8_t bytes[2];
}my_struct_t;

int main() {
    // Create an instance of the struct
    my_struct_t myStruct;

    // Print the size of the struct
    printf("Size of my_struct_t: %zu bytes\n", sizeof(myStruct));
    //printf("Offset of var1: %zu\n", offsetof(my_struct_t, var1));
    //printf("Offset of var2: %zu\n", offsetof(my_struct_t, var2));

    return 0;
}
