#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Simple function to demonstrate stack frame and local variable
void foo() {
    int x = 123; // local variable on stack
    printf("In foo(), x = %d\n", x);
}


void memory_example() {
    // malloc allocates uninitialized memory (contains garbage values)
    int *malloc_array = (int*)malloc(5 * sizeof(int));
    if (malloc_array == NULL) {
        printf("malloc failed\n");
        return;
    }
    
    printf("malloc array (uninitialized): ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", malloc_array[i]);  // Will print garbage values
    }
    printf("\n");
    
    // Initialize malloc array manually
    for (int i = 0; i < 5; i++) {
        malloc_array[i] = i + 1;
    }
    
    printf("malloc array (after init): ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", malloc_array[i]);
    }
    printf("\n");
    
    // calloc allocates zero-initialized memory (all values are 0)
    int *calloc_array = (int*)calloc(5, sizeof(int));
    if (calloc_array == NULL) {
        printf("calloc failed\n");
        free(malloc_array);
        return;
    }
    
    printf("calloc array (zero-initialized): ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", calloc_array[i]);  // Will print all zeros
    }
    printf("\n");
    
    // Key differences:
    // 1. malloc(size) vs calloc(count, size)
    // 2. malloc doesn't initialize, calloc initializes to zero
    // 3. calloc is slightly slower due to zero initialization
    
    // Always free allocated memory to prevent memory leaks
    free(malloc_array);
    free(calloc_array);
}

int main() {
    foo(); // first function call to show stack frame creation
    memory_example();
    return 0;
}