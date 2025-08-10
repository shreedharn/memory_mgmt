# Pointer Arithmetic in C - Comprehensive Guide

## Table of Contents
1. [Pointer Fundamentals](#pointer-fundamentals)
2. [Array of Pointers vs Pointer to Array](#array-of-pointers-vs-pointer-to-array)
3. [Size Allocations and Memory Layout](#size-allocations-and-memory-layout)
4. [Referencing and Dereferencing](#referencing-and-dereferencing)
5. [Parameter Passing with Pointers](#parameter-passing-with-pointers)
6. [String Manipulation with Pointers](#string-manipulation-with-pointers)
7. [Function Pointers](#function-pointers)
8. [Pointer Arithmetic Operations](#pointer-arithmetic-operations)
9. [Memory Leakage Examples](#memory-leakage-examples)
10. [Memory Corruption Examples](#memory-corruption-examples)

---

## Pointer Fundamentals

### What is a Pointer?
A pointer is a variable that stores the **memory address** of another variable.

```c
#include <stdio.h>

int main() {
    int value = 42;           // Regular variable: stores actual value
    int *ptr = &value;        // Pointer variable: stores address of value
    
    printf("value = %d\n", value);          // Prints: 42
    printf("address of value = %p\n", &value);    // Prints: 0x7fff5c...
    printf("ptr = %p\n", ptr);              // Prints: 0x7fff5c... (same address)
    printf("*ptr = %d\n", *ptr);            // Prints: 42 (value at address)
    
    return 0;
}
```

### Memory Diagram:
```
Stack Memory:
┌─────────────────┐ ← 0x7fff5c68 (higher address)
│ ptr = 0x7fff5c64│ ← ptr variable (8 bytes on 64-bit)
├─────────────────┤ ← 0x7fff5c64 
│ value = 42      │ ← value variable (4 bytes)
├─────────────────┤ ← 0x7fff5c60
│ [other data]    │
└─────────────────┘ ← lower addresses

Relationship: ptr → points to → value
              │                  ↑
              └─ contains ────────┘
                 address 0x7fff5c64
```

---

## Array of Pointers vs Pointer to Array

### Array of Pointers
An array where each element is a pointer to another location.

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Array of pointers: each element is a pointer to int
    int *arr_of_ptrs[3];                    // 3 pointers to int
    
    // Allocate memory and assign
    arr_of_ptrs[0] = malloc(sizeof(int));   // Heap allocation
    arr_of_ptrs[1] = malloc(sizeof(int));   // Heap allocation  
    arr_of_ptrs[2] = malloc(sizeof(int));   // Heap allocation
    
    // Assign values
    *arr_of_ptrs[0] = 10;
    *arr_of_ptrs[1] = 20;
    *arr_of_ptrs[2] = 30;
    
    printf("Values: %d, %d, %d\n", *arr_of_ptrs[0], *arr_of_ptrs[1], *arr_of_ptrs[2]);
    printf("Addresses: %p, %p, %p\n", arr_of_ptrs[0], arr_of_ptrs[1], arr_of_ptrs[2]);
    
    // Clean up
    free(arr_of_ptrs[0]);
    free(arr_of_ptrs[1]);
    free(arr_of_ptrs[2]);
    
    return 0;
}
```

### Memory Diagram - Array of Pointers:
```
Stack (arr_of_ptrs):                   Heap:
┌─────────────────┐                    ┌─────────────────┐
│ [0]: 0x8000     │ ────────────────→  │ 0x8000: 10      │
├─────────────────┤                    ├─────────────────┤
│ [1]: 0x8010     │ ──────────────┐    │ 0x8004: ???     │
├─────────────────┤               │    ├─────────────────┤
│ [2]: 0x8020     │ ──────────┐   │    │ 0x8008: ???     │
└─────────────────┘           │   │    ├─────────────────┤
                              │   │    │ 0x800C: ???     │
                              │   │    ├─────────────────┤
                              │   └─→  │ 0x8010: 20      │
                              │        ├─────────────────┤
                              │        │ ...             │
                              │        ├─────────────────┤
                              └─────→  │ 0x8020: 30      │
                                       └─────────────────┘
```

### Pointer to Array
A single pointer that points to an entire array.

```c
#include <stdio.h>

int main() {
    int arr[3] = {10, 20, 30};           // Regular array
    int (*ptr_to_arr)[3] = &arr;         // Pointer to array of 3 ints
    
    // Accessing elements through pointer to array
    printf("First element: %d\n", (*ptr_to_arr)[0]);   // 10
    printf("Second element: %d\n", (*ptr_to_arr)[1]);  // 20
    printf("Third element: %d\n", (*ptr_to_arr)[2]);   // 30
    
    // Alternative access method
    printf("Using pointer arithmetic: %d\n", *(*ptr_to_arr + 1)); // 20
    
    printf("Array address: %p\n", &arr);
    printf("Pointer value: %p\n", ptr_to_arr);
    
    return 0;
}
```

### Memory Diagram - Pointer to Array:
```
Stack Memory:
┌─────────────────┐
│ ptr_to_arr =    │ ← pointer to entire array
│ 0x7fff5c60      │ ────────┐
├─────────────────┤         │
│ arr[2] = 30     │ ←───────┘
├─────────────────┤ ← 0x7fff5c68
│ arr[1] = 20     │
├─────────────────┤ ← 0x7fff5c64
│ arr[0] = 10     │
└─────────────────┘ ← 0x7fff5c60

Key difference: ptr_to_arr points to the WHOLE array as a unit
```

---

## Size Allocations and Memory Layout

### Size Calculations

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Different pointer types and their sizes
    int *int_ptr;
    char *char_ptr;
    double *double_ptr;
    int *ptr_array[5];              // Array of 5 int pointers
    int (*array_ptr)[5];            // Pointer to array of 5 ints
    
    printf("=== POINTER SIZES ===\n");
    printf("int* size: %zu bytes\n", sizeof(int_ptr));        // 8 on 64-bit
    printf("char* size: %zu bytes\n", sizeof(char_ptr));      // 8 on 64-bit
    printf("double* size: %zu bytes\n", sizeof(double_ptr));  // 8 on 64-bit
    
    printf("\n=== ARRAY SIZES ===\n");
    printf("Array of pointers size: %zu bytes\n", sizeof(ptr_array));    // 5 * 8 = 40
    printf("Pointer to array size: %zu bytes\n", sizeof(array_ptr));     // 8 bytes
    
    // What they point to
    int sample_array[5] = {1, 2, 3, 4, 5};
    array_ptr = &sample_array;
    
    printf("\n=== POINTED-TO DATA SIZES ===\n");
    printf("Size of pointed-to array: %zu bytes\n", sizeof(*array_ptr)); // 5 * 4 = 20
    printf("Size of one element: %zu bytes\n", sizeof(**array_ptr));     // 4 bytes
    
    return 0;
}
```

### Memory Layout Comparison:

```c
#include <stdio.h>
#include <stdlib.h>

void demonstrate_memory_layout() {
    // Array of pointers
    int *arr_ptrs[3];
    for (int i = 0; i < 3; i++) {
        arr_ptrs[i] = malloc(sizeof(int));
        *arr_ptrs[i] = i * 10;
    }
    
    // Pointer to array
    int arr[3] = {0, 10, 20};
    int (*ptr_to_arr)[3] = &arr;
    
    printf("=== ARRAY OF POINTERS ===\n");
    for (int i = 0; i < 3; i++) {
        printf("arr_ptrs[%d] = %p, value = %d\n", i, arr_ptrs[i], *arr_ptrs[i]);
    }
    
    printf("\n=== POINTER TO ARRAY ===\n");
    printf("ptr_to_arr = %p\n", ptr_to_arr);
    for (int i = 0; i < 3; i++) {
        printf("(*ptr_to_arr)[%d] = %d\n", i, (*ptr_to_arr)[i]);
    }
    
    // Clean up array of pointers
    for (int i = 0; i < 3; i++) {
        free(arr_ptrs[i]);
    }
}
```

### Memory Usage Diagram:
```
Array of Pointers (24 bytes stack + 12 bytes heap):
Stack:                           Heap:
┌─────────────────┐              ┌─────────────────┐
│ arr_ptrs[0]: 8B │ ──────────→  │ int: 4B         │
├─────────────────┤              ├─────────────────┤
│ arr_ptrs[1]: 8B │ ──────────→  │ int: 4B         │  
├─────────────────┤              ├─────────────────┤
│ arr_ptrs[2]: 8B │ ──────────→  │ int: 4B         │
└─────────────────┘              └─────────────────┘
Total: 24B stack + 12B heap = 36B

Pointer to Array (8 bytes stack + 12 bytes stack):
Stack:
┌─────────────────┐
│ ptr_to_arr: 8B  │ ──────────┐
├─────────────────┤           │
│ arr[0]: 4B      │ ←─────────┘
├─────────────────┤
│ arr[1]: 4B      │
├─────────────────┤
│ arr[2]: 4B      │
└─────────────────┘
Total: 20B stack only
```

---

## Referencing and Dereferencing

### Basic Referencing (&) and Dereferencing (*)

```c
#include <stdio.h>

int main() {
    int value = 100;
    int *ptr;
    
    // Referencing: get address of variable
    ptr = &value;                    // ptr now contains address of value
    
    printf("=== REFERENCING ===\n");
    printf("value = %d\n", value);
    printf("&value = %p\n", &value);        // Address of value
    printf("ptr = %p\n", ptr);              // Same address stored in ptr
    
    // Dereferencing: get value at address
    printf("\n=== DEREFERENCING ===\n");
    printf("*ptr = %d\n", *ptr);            // Value at address stored in ptr
    
    // Modifying through pointer
    *ptr = 200;                      // Change value through pointer
    printf("\nAfter *ptr = 200:\n");
    printf("value = %d\n", value);          // Original variable changed!
    printf("*ptr = %d\n", *ptr);
    
    return 0;
}
```

### Multiple Levels of Indirection

```c
#include <stdio.h>

int main() {
    int value = 42;
    int *ptr = &value;               // Pointer to int
    int **ptr_to_ptr = &ptr;         // Pointer to pointer to int
    int ***ptr_to_ptr_to_ptr = &ptr_to_ptr;  // Triple pointer
    
    printf("=== MULTI-LEVEL POINTERS ===\n");
    printf("value = %d\n", value);
    printf("*ptr = %d\n", *ptr);
    printf("**ptr_to_ptr = %d\n", **ptr_to_ptr);
    printf("***ptr_to_ptr_to_ptr = %d\n", ***ptr_to_ptr_to_ptr);
    
    // All refer to the same value!
    printf("\n=== ADDRESSES ===\n");
    printf("&value = %p\n", &value);
    printf("ptr = %p\n", ptr);
    printf("*ptr_to_ptr = %p\n", *ptr_to_ptr);
    printf("**ptr_to_ptr_to_ptr = %p\n", **ptr_to_ptr_to_ptr);
    
    return 0;
}
```

### Memory Diagram - Multiple Indirection:
```
Stack Memory:
┌─────────────────────┐ ← 0x7fff5c78
│ ptr_to_ptr_to_ptr = │   (contains 0x7fff5c70)
│ 0x7fff5c70          │
├─────────────────────┤ ← 0x7fff5c70
│ ptr_to_ptr =        │   (contains 0x7fff5c68)
│ 0x7fff5c68          │
├─────────────────────┤ ← 0x7fff5c68  
│ ptr = 0x7fff5c64    │   (contains 0x7fff5c64)
├─────────────────────┤ ← 0x7fff5c64
│ value = 42          │   (actual value)
└─────────────────────┘

Dereferencing chain:
ptr_to_ptr_to_ptr → ptr_to_ptr → ptr → value
       ***              **       *     (42)
```

---

## Parameter Passing with Pointers

### Pass by Value vs Pass by Reference

```c
#include <stdio.h>

// Function that attempts to modify value (FAILS)
void modify_by_value(int x) {
    x = 999;                         // Only modifies local copy
    printf("Inside modify_by_value: x = %d\n", x);
}

// Function that successfully modifies value
void modify_by_reference(int *x) {
    *x = 999;                        // Modifies value at address
    printf("Inside modify_by_reference: *x = %d\n", *x);
}

// Function that modifies the pointer itself
void modify_pointer_itself(int **x) {
    static int new_value = 888;
    *x = &new_value;                 // Changes where pointer points
    printf("Inside modify_pointer_itself: **x = %d\n", **x);
}

int main() {
    int value = 100;
    int *ptr = &value;
    
    printf("Original value = %d\n", value);
    
    // Pass by value - no change
    modify_by_value(value);
    printf("After modify_by_value: value = %d\n", value);      // Still 100
    
    // Pass by reference - value changes
    modify_by_reference(&value);
    printf("After modify_by_reference: value = %d\n", value);  // Now 999
    
    // Reset for next example
    value = 100;
    ptr = &value;
    
    // Modify pointer itself
    printf("Before: ptr points to %p, *ptr = %d\n", ptr, *ptr);
    modify_pointer_itself(&ptr);
    printf("After: ptr points to %p, *ptr = %d\n", ptr, *ptr);
    printf("Original value = %d\n", value);                    // Still 100
    
    return 0;
}
```

### Array Passing

```c
#include <stdio.h>

// Arrays are always passed as pointers
void print_array(int *arr, int size) {        // Same as: int arr[]
    printf("Inside function:\n");
    printf("sizeof(arr) = %zu\n", sizeof(arr));    // Size of pointer (8 bytes)
    
    for (int i = 0; i < size; i++) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }
}

void modify_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] *= 2;                           // Modifies original array
    }
}

int main() {
    int numbers[5] = {1, 2, 3, 4, 5};
    
    printf("Original array:\n");
    printf("sizeof(numbers) = %zu\n", sizeof(numbers));  // 20 bytes (5 * 4)
    
    print_array(numbers, 5);                  // Array name = address of first element
    
    printf("\nModifying array:\n");
    modify_array(numbers, 5);
    
    printf("After modification:\n");
    for (int i = 0; i < 5; i++) {
        printf("numbers[%d] = %d\n", i, numbers[i]);
    }
    
    return 0;
}
```

### Memory Flow During Function Calls:
```
main() stack frame:                 print_array() stack frame:
┌─────────────────┐                 ┌─────────────────┐
│ numbers[0] = 1  │ ← 0x7fff5c60    │ arr = 0x7fff5c60│ ← Points back to main's array
│ numbers[1] = 2  │                 │ size = 5        │
│ numbers[2] = 3  │                 └─────────────────┘
│ numbers[3] = 4  │                          ↑
│ numbers[4] = 5  │                          │
└─────────────────┘                          │
         │                                   │
         └─── passed as address ─────────────┘

No copying of array data - only address is passed!
```

---

## String Manipulation with Pointers

### String Literals vs Character Arrays

```c
#include <stdio.h>
#include <string.h>

int main() {
    // String literal (read-only, in code segment)
    char *str1 = "Hello, World!";        // Pointer to string literal
    
    // Character array (modifiable, on stack)
    char str2[] = "Hello, World!";       // Array initialized with string
    char str3[50] = "Hello";             // Array with extra space
    
    printf("=== STRING STORAGE ===\n");
    printf("str1 (literal): %p\n", str1);
    printf("str2 (array): %p\n", str2);
    printf("str3 (array): %p\n", str3);
    
    printf("\n=== SIZES ===\n");
    printf("sizeof(str1): %zu (pointer size)\n", sizeof(str1));    // 8 bytes
    printf("sizeof(str2): %zu (array size)\n", sizeof(str2));      // 14 bytes
    printf("sizeof(str3): %zu (declared size)\n", sizeof(str3));   // 50 bytes
    printf("strlen(str1): %zu (string length)\n", strlen(str1));   // 13 chars
    
    // Modification attempts
    // str1[0] = 'h';                    // CRASH! Cannot modify string literal
    str2[0] = 'h';                       // OK - modifying array
    str3[5] = ',';                       // OK - modifying array
    
    printf("\nAfter modification:\n");
    printf("str2: %s\n", str2);
    printf("str3: %s\n", str3);
    
    return 0;
}
```

### String Pointer Arithmetic

```c
#include <stdio.h>

// Function to find character in string
char* find_char(char *str, char target) {
    while (*str != '\0') {               // While not end of string
        if (*str == target) {
            return str;                  // Return pointer to found character
        }
        str++;                           // Move pointer to next character
    }
    return NULL;                         // Not found
}

// Function to copy string manually
void string_copy(char *dest, char *src) {
    while (*src != '\0') {               // Copy until null terminator
        *dest = *src;                    // Copy character
        dest++;                          // Advance destination pointer
        src++;                           // Advance source pointer
    }
    *dest = '\0';                        // Add null terminator
}

int main() {
    char message[] = "Find the letter 'e'";
    char buffer[100];
    
    printf("Original: %s\n", message);
    
    // Find character
    char *found = find_char(message, 'e');
    if (found != NULL) {
        printf("Found 'e' at position: %ld\n", found - message);
        printf("Remaining string: %s\n", found);
    }
    
    // Copy string
    string_copy(buffer, message);
    printf("Copied string: %s\n", buffer);
    
    // Demonstrate pointer arithmetic
    char *ptr = message;
    printf("\n=== POINTER WALKING ===\n");
    while (*ptr != '\0') {
        printf("Address: %p, Character: '%c'\n", ptr, *ptr);
        ptr++;
    }
    
    return 0;
}
```

### String Memory Layout:
```
String Literal (Code Segment):
┌─────────────────┐ ← 0x400000 (read-only)
│'H'│'e'│'l'│'l'│'o'│','│' '│'W'│'o'│'r'│'l'│'d'│'!'│'\0'│
└─────────────────┘
 ↑
 │
str1 (on stack): [0x400000] ← pointer to literal

Character Array (Stack):
┌─────────────────┐ ← 0x7fff5c60 (modifiable)  
│'H'│'e'│'l'│'l'│'o'│','│' '│'W'│'o'│'r'│'l'│'d'│'!'│'\0'│
└─────────────────┘
 ↑
str2 points here (str2 = 0x7fff5c60)
```

---

## Function Pointers

### Basic Function Pointers

```c
#include <stdio.h>

// Sample functions to point to
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int subtract(int a, int b) {
    return a - b;
}

int main() {
    // Declare function pointer
    int (*operation)(int, int);          // Pointer to function returning int, taking 2 ints
    
    int x = 10, y = 5;
    
    // Point to different functions
    operation = add;                     // No & needed (function name is address)
    printf("add: %d + %d = %d\n", x, y, operation(x, y));
    
    operation = multiply;
    printf("multiply: %d * %d = %d\n", x, y, operation(x, y));
    
    operation = subtract;
    printf("subtract: %d - %d = %d\n", x, y, operation(x, y));
    
    // Function addresses
    printf("\n=== FUNCTION ADDRESSES ===\n");
    printf("add function at: %p\n", add);
    printf("multiply function at: %p\n", multiply);
    printf("subtract function at: %p\n", subtract);
    
    return 0;
}
```

### Array of Function Pointers

```c
#include <stdio.h>

// Calculator functions
double add_d(double a, double b) { return a + b; }
double sub_d(double a, double b) { return a - b; }
double mul_d(double a, double b) { return a * b; }
double div_d(double a, double b) { return b != 0 ? a / b : 0; }

int main() {
    // Array of function pointers
    double (*operations[])(double, double) = {add_d, sub_d, mul_d, div_d};
    char *op_names[] = {"add", "subtract", "multiply", "divide"};
    
    double x = 12.5, y = 3.0;
    
    printf("Calculator with function pointer array:\n");
    printf("x = %.2f, y = %.2f\n\n", x, y);
    
    // Use array to call different functions
    for (int i = 0; i < 4; i++) {
        double result = operations[i](x, y);     // Call function through pointer
        printf("%s: %.2f\n", op_names[i], result);
    }
    
    return 0;
}
```

### Function Pointer as Parameter

```c
#include <stdio.h>

// Generic array processor
void process_array(int *arr, int size, int (*processor)(int)) {
    printf("Processing array with custom function:\n");
    for (int i = 0; i < size; i++) {
        arr[i] = processor(arr[i]);          // Apply function to each element
    }
}

// Different processing functions
int square(int x) { return x * x; }
int double_it(int x) { return x * 2; }
int negate(int x) { return -x; }

void print_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    int numbers[] = {1, 2, 3, 4, 5};
    int size = 5;
    
    printf("Original array: ");
    print_array(numbers, size);
    
    // Process with square function
    process_array(numbers, size, square);
    printf("After square: ");
    print_array(numbers, size);
    
    // Reset array
    int numbers2[] = {1, 2, 3, 4, 5};
    process_array(numbers2, size, double_it);
    printf("After double: ");
    print_array(numbers2, size);
    
    return 0;
}
```

### Function Pointer Memory Diagram:
```
Code Segment (Functions):
┌─────────────────┐ ← 0x401000
│ add() code      │
├─────────────────┤ ← 0x401020
│ multiply() code │
├─────────────────┤ ← 0x401040
│ subtract() code │
└─────────────────┘

Stack (Function Pointers):
┌─────────────────┐
│ operation =     │ ← Can point to any of the functions above
│ 0x401020        │   (currently points to multiply)
└─────────────────┘

When called: operation(10, 5)
1. Look up address in operation (0x401020)
2. Jump to that address
3. Execute multiply() function
4. Return result
```

---

## Pointer Arithmetic Operations

### Basic Arithmetic Operations

```c
#include <stdio.h>

int main() {
    int arr[] = {10, 20, 30, 40, 50};
    int *ptr = arr;                      // Point to first element
    
    printf("=== INITIAL STATE ===\n");
    printf("ptr = %p, *ptr = %d\n", ptr, *ptr);
    
    // Addition
    ptr++;                               // Move to next int (4 bytes forward)
    printf("\n=== AFTER ptr++ ===\n");
    printf("ptr = %p, *ptr = %d\n", ptr, *ptr);
    
    ptr += 2;                            // Move 2 ints forward (8 bytes)
    printf("\n=== AFTER ptr += 2 ===\n");
    printf("ptr = %p, *ptr = %d\n", ptr, *ptr);
    
    // Subtraction
    ptr--;                               // Move one int backward (4 bytes)
    printf("\n=== AFTER ptr-- ===\n");
    printf("ptr = %p, *ptr = %d\n", ptr, *ptr);
    
    ptr -= 2;                            // Move 2 ints backward (8 bytes)
    printf("\n=== AFTER ptr -= 2 ===\n");
    printf("ptr = %p, *ptr = %d\n", ptr, *ptr);
    
    // Pointer difference
    int *start = arr;
    int *end = arr + 4;
    printf("\n=== POINTER DIFFERENCE ===\n");
    printf("end - start = %ld elements\n", end - start);
    printf("Distance in bytes = %ld\n", (char*)end - (char*)start);
    
    return 0;
}
```

### Array Traversal with Pointers

```c
#include <stdio.h>

void traverse_with_indexing(int *arr, int size) {
    printf("Method 1 - Array indexing:\n");
    for (int i = 0; i < size; i++) {
        printf("arr[%d] = %d (at %p)\n", i, arr[i], &arr[i]);
    }
}

void traverse_with_arithmetic(int *arr, int size) {
    printf("\nMethod 2 - Pointer arithmetic:\n");
    for (int i = 0; i < size; i++) {
        printf("*(arr + %d) = %d (at %p)\n", i, *(arr + i), arr + i);
    }
}

void traverse_with_pointer_increment(int *arr, int size) {
    printf("\nMethod 3 - Pointer increment:\n");
    int *ptr = arr;
    for (int i = 0; i < size; i++) {
        printf("*ptr = %d (at %p)\n", *ptr, ptr);
        ptr++;                           // Move to next element
    }
}

int main() {
    int numbers[] = {100, 200, 300, 400, 500};
    int size = 5;
    
    traverse_with_indexing(numbers, size);
    traverse_with_arithmetic(numbers, size);
    traverse_with_pointer_increment(numbers, size);
    
    // Demonstrate equivalence
    printf("\n=== EQUIVALENCE DEMONSTRATION ===\n");
    int *ptr = numbers;
    printf("numbers[2] = %d\n", numbers[2]);
    printf("*(numbers + 2) = %d\n", *(numbers + 2));
    printf("*(ptr + 2) = %d\n", *(ptr + 2));
    printf("ptr[2] = %d\n", ptr[2]);             // Yes, pointers can use [] too!
    
    return 0;
}
```

### Different Data Type Arithmetic

```c
#include <stdio.h>

int main() {
    // Arrays of different types
    char char_arr[] = "HELLO";
    short short_arr[] = {100, 200, 300};
    int int_arr[] = {1000, 2000, 3000};
    double double_arr[] = {1.1, 2.2, 3.3};
    
    // Pointers to different types
    char *char_ptr = char_arr;
    short *short_ptr = short_arr;
    int *int_ptr = int_arr;
    double *double_ptr = double_arr;
    
    printf("=== INITIAL ADDRESSES ===\n");
    printf("char_ptr = %p\n", char_ptr);
    printf("short_ptr = %p\n", short_ptr);  
    printf("int_ptr = %p\n", int_ptr);
    printf("double_ptr = %p\n", double_ptr);
    
    // Increment each pointer
    char_ptr++;      // +1 byte
    short_ptr++;     // +2 bytes  
    int_ptr++;       // +4 bytes
    double_ptr++;    // +8 bytes
    
    printf("\n=== AFTER INCREMENT ===\n");
    printf("char_ptr = %p (moved %ld bytes)\n", char_ptr, (char*)char_ptr - char_arr);
    printf("short_ptr = %p (moved %ld bytes)\n", short_ptr, (char*)short_ptr - (char*)short_arr);
    printf("int_ptr = %p (moved %ld bytes)\n", int_ptr, (char*)int_ptr - (char*)int_arr);
    printf("double_ptr = %p (moved %ld bytes)\n", double_ptr, (char*)double_ptr - (char*)double_arr);
    
    return 0;
}
```

### Pointer Arithmetic Memory Visualization:
```
Array in memory: int arr[] = {10, 20, 30, 40, 50};

Memory layout (each int = 4 bytes):
┌─────────┬─────────┬─────────┬─────────┬─────────┐
│ 10      │ 20      │ 30      │ 40      │ 50      │
└─────────┴─────────┴─────────┴─────────┴─────────┘
0x1000    0x1004    0x1008    0x100C    0x1010
↑         ↑         ↑         ↑         ↑
arr[0]    arr[1]    arr[2]    arr[3]    arr[4]

Pointer arithmetic:
int *ptr = arr;          // ptr = 0x1000
ptr++;                   // ptr = 0x1004 (moved by sizeof(int) = 4 bytes)
ptr += 2;               // ptr = 0x100C (moved by 2 * sizeof(int) = 8 bytes)

Key: Pointer arithmetic is in terms of elements, not bytes!
     ptr + 1 means "next element", not "next byte"
```

---

## Memory Leakage Examples

### Classic Memory Leaks

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LEAK 1: Forgetting to free allocated memory
void memory_leak_basic() {
    printf("=== BASIC MEMORY LEAK ===\n");
    
    int *ptr = malloc(sizeof(int) * 1000);   // Allocate 4000 bytes
    if (ptr == NULL) return;
    
    // Use the memory
    for (int i = 0; i < 1000; i++) {
        ptr[i] = i;
    }
    
    printf("Allocated and used 4000 bytes\n");
    
    // LEAK: Forgot to call free(ptr)!
    // When function returns, ptr goes out of scope
    // but the allocated memory remains allocated
}

// LEAK 2: Losing reference to allocated memory
void memory_leak_lost_reference() {
    printf("\n=== LOST REFERENCE LEAK ===\n");
    
    char *buffer = malloc(1024);             // Allocate 1KB
    strcpy(buffer, "Initial data");
    
    printf("Buffer contains: %s\n", buffer);
    
    // LEAK: Reassign pointer without freeing old memory
    buffer = malloc(2048);                   // New allocation - old memory lost!
    strcpy(buffer, "New data");
    
    printf("Buffer now contains: %s\n", buffer);
    
    free(buffer);                            // Only frees the 2KB, 1KB is leaked
}

// LEAK 3: Exception/early return without cleanup
int memory_leak_early_return(int condition) {
    printf("\n=== EARLY RETURN LEAK ===\n");
    
    char *temp_buffer = malloc(512);
    if (temp_buffer == NULL) return -1;
    
    strcpy(temp_buffer, "Temporary data");
    
    if (condition < 0) {
        printf("Error condition - early return\n");
        return -1;                           // LEAK: temp_buffer not freed!
    }
    
    printf("Processing: %s\n", temp_buffer);
    free(temp_buffer);                       // Only reached if condition >= 0
    return 0;
}

// LEAK 4: Loop allocation without cleanup
void memory_leak_in_loop() {
    printf("\n=== LOOP ALLOCATION LEAK ===\n");
    
    for (int i = 0; i < 5; i++) {
        char *data = malloc(100);            // Allocate in each iteration
        sprintf(data, "Iteration %d", i);
        printf("%s\n", data);
        
        // LEAK: Never freed - accumulates 500 bytes total
    }
}

int main() {
    memory_leak_basic();
    memory_leak_lost_reference();
    memory_leak_early_return(-1);            // Triggers leak
    memory_leak_early_return(1);             // No leak
    memory_leak_in_loop();
    
    printf("\n=== PROGRAM ENDING ===\n");
    printf("All leaked memory will be reclaimed by OS\n");
    printf("But in long-running programs, this causes problems!\n");
    
    return 0;
}
```

### Proper Memory Management

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CORRECT: Proper cleanup
void proper_memory_management() {
    printf("=== PROPER MEMORY MANAGEMENT ===\n");
    
    int *numbers = malloc(sizeof(int) * 100);
    if (numbers == NULL) {
        printf("Allocation failed\n");
        return;
    }
    
    // Use memory
    for (int i = 0; i < 100; i++) {
        numbers[i] = i * i;
    }
    
    printf("Processed 100 numbers\n");
    
    free(numbers);                           // Always free allocated memory
    numbers = NULL;                          // Prevent accidental reuse
    printf("Memory properly freed\n");
}

// CORRECT: Multiple allocations with proper cleanup
int proper_multiple_allocations() {
    char *buffer1 = NULL, *buffer2 = NULL;
    int result = 0;
    
    buffer1 = malloc(256);
    if (buffer1 == NULL) {
        result = -1;
        goto cleanup;                        // Single cleanup point
    }
    
    buffer2 = malloc(512);
    if (buffer2 == NULL) {
        result = -1;
        goto cleanup;
    }
    
    // Use buffers
    strcpy(buffer1, "First buffer");
    strcpy(buffer2, "Second buffer with more data");
    
    printf("Buffer1: %s\n", buffer1);
    printf("Buffer2: %s\n", buffer2);
    
cleanup:
    if (buffer1) free(buffer1);              // Safe even if NULL
    if (buffer2) free(buffer2);
    return result;
}

int main() {
    proper_memory_management();
    proper_multiple_allocations();
    return 0;
}
```

### Memory Leak Detection Tips:
```
Tools for finding leaks:
┌─────────────────────────────────────────────────────────┐
│ Linux/Mac: valgrind --leak-check=full ./program        │
│ Windows: Visual Studio Diagnostic Tools                │
│ Static Analysis: cppcheck, clang static analyzer       │
│                                                         │
│ Manual techniques:                                      │
│ • Match every malloc() with free()                     │
│ • Set pointers to NULL after freeing                   │
│ • Use goto for single cleanup point                    │
│ • Consider RAII patterns (in C++)                      │
└─────────────────────────────────────────────────────────┘
```

---

## Memory Corruption Examples

### Buffer Overflows

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DANGEROUS: Buffer overflow on stack
void stack_buffer_overflow() {
    printf("=== STACK BUFFER OVERFLOW ===\n");
    
    char buffer[10];                         // 10 byte buffer
    char *dangerous_input = "This string is way too long for the buffer!";
    
    printf("Buffer size: %zu bytes\n", sizeof(buffer));
    printf("Input size: %zu bytes\n", strlen(dangerous_input));
    
    // CORRUPTION: Writes past end of buffer
    strcpy(buffer, dangerous_input);         // No bounds checking!
    
    printf("Buffer contents: %s\n", buffer);
    // Program may crash or behave unpredictably
}

// DANGEROUS: Heap buffer overflow
void heap_buffer_overflow() {
    printf("\n=== HEAP BUFFER OVERFLOW ===\n");
    
    char *buffer = malloc(20);               // 20 byte buffer
    if (buffer == NULL) return;
    
    // CORRUPTION: Write past allocated area
    for (int i = 0; i < 50; i++) {           // Writing 50 bytes into 20 byte buffer!
        buffer[i] = 'A' + (i % 26);
    }
    buffer[49] = '\0';
    
    printf("Buffer: %s\n", buffer);
    
    free(buffer);
    // May corrupt heap metadata, cause crashes later
}

int main() {
    // WARNING: These examples demonstrate dangerous code!
    // Uncomment to see corruption (may crash)
    
    // stack_buffer_overflow();
    // heap_buffer_overflow();
    
    printf("Buffer overflow examples commented out for safety\n");
    printf("Uncomment to see actual corruption behavior\n");
    
    return 0;
}
```

### Use After Free

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// DANGEROUS: Using freed memory
void use_after_free_example() {
    printf("=== USE AFTER FREE ===\n");
    
    char *buffer = malloc(50);
    if (buffer == NULL) return;
    
    strcpy(buffer, "Valid data");
    printf("Before free: %s\n", buffer);
    
    free(buffer);                            // Memory freed
    
    // CORRUPTION: Using freed memory
    printf("After free: %s\n", buffer);     // Undefined behavior!
    strcpy(buffer, "New data");              // Writing to freed memory!
    
    // buffer pointer still contains old address
    // but memory may be reused by other allocations
}

// DANGEROUS: Double free
void double_free_example() {
    printf("\n=== DOUBLE FREE ===\n");
    
    int *ptr = malloc(sizeof(int) * 10);
    if (ptr == NULL) return;
    
    ptr[0] = 42;
    printf("Value: %d\n", ptr[0]);
    
    free(ptr);                               // First free - OK
    printf("Memory freed once\n");
    
    // CORRUPTION: Freeing already freed memory
    free(ptr);                               // Second free - CORRUPTION!
    
    printf("This line may not be reached\n");
}

int main() {
    // WARNING: These examples show dangerous code!
    // May cause crashes or unpredictable behavior
    
    printf("Use-after-free and double-free examples:\n");
    printf("These are commented out as they may crash the program\n\n");
    
    // Uncomment to see corruption (will likely crash):
    // use_after_free_example();
    // double_free_example();
    
    return 0;
}
```

### Wild Pointers

```c
#include <stdio.h>
#include <stdlib.h>

// DANGEROUS: Uninitialized pointer
void wild_pointer_example() {
    printf("=== WILD POINTER ===\n");
    
    int *ptr;                                // Uninitialized pointer!
    
    // CORRUPTION: Using uninitialized pointer
    printf("Wild pointer value: %p\n", ptr);     // Random address
    // *ptr = 42;                               // May crash or corrupt random memory
    
    printf("Assignment commented out for safety\n");
}

// DANGEROUS: Pointer to local variable
int* return_local_pointer() {
    int local_var = 123;                     // Stack variable
    
    // DANGEROUS: Returning address of local variable
    return &local_var;                       // local_var destroyed when function returns!
}

void dangling_pointer_example() {
    printf("\n=== DANGLING POINTER ===\n");
    
    int *ptr = return_local_pointer();       // Points to destroyed stack location
    
    printf("Dangling pointer: %p\n", ptr);
    // printf("Value: %d\n", *ptr);            // Undefined behavior!
    
    printf("Dereference commented out for safety\n");
}

int main() {
    wild_pointer_example();
    dangling_pointer_example();
    
    return 0;
}
```

### Memory Corruption Prevention

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// SAFE: Bounds-checked string copy
void safe_string_copy(char *dest, const char *src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) return;
    
    strncpy(dest, src, dest_size - 1);       // Leave space for null terminator
    dest[dest_size - 1] = '\0';              // Ensure null termination
}

// SAFE: Memory management with checks
void safe_memory_management() {
    printf("=== SAFE MEMORY MANAGEMENT ===\n");
    
    char *buffer = malloc(32);
    if (buffer == NULL) {
        printf("Allocation failed\n");
        return;
    }
    
    // Safe string operations
    safe_string_copy(buffer, "This is a safe string copy", 32);
    printf("Buffer: %s\n", buffer);
    
    // Clear memory before freeing (security practice)
    memset(buffer, 0, 32);
    
    free(buffer);
    buffer = NULL;                           // Prevent accidental reuse
    
    printf("Memory safely managed\n");
}

// SAFE: Array bounds checking
void safe_array_access(int *arr, int size, int index) {
    if (arr == NULL) {
        printf("Error: NULL pointer\n");
        return;
    }
    
    if (index < 0 || index >= size) {
        printf("Error: Index %d out of bounds [0, %d)\n", index, size);
        return;
    }
    
    printf("arr[%d] = %d\n", index, arr[index]);
}

int main() {
    safe_memory_management();
    
    int numbers[] = {10, 20, 30, 40, 50};
    safe_array_access(numbers, 5, 2);       // Valid access
    safe_array_access(numbers, 5, 10);      // Bounds error caught
    safe_array_access(NULL, 5, 0);          // NULL pointer caught
    
    return 0;
}
```

### Memory Corruption Visualization:
```
Buffer Overflow Example:
Intended:
┌─────────────────┐ ← buffer[10]
│ [0][1][2]...[9] │
└─────────────────┘

Actual (overflow):
┌─────────────────┬─────────────────┬─────────────────┐
│ [0][1][2]...[9] │ OVERFLOW DATA   │ OTHER VARIABLES │
└─────────────────┴─────────────────┴─────────────────┘
                   ↑                 ↑
                   Corrupts memory   May corrupt stack frame,
                   past buffer end   return addresses, etc.

Result: Program crash, security vulnerabilities, or silent data corruption
```

This comprehensive guide covers all the essential pointer arithmetic concepts with practical examples and memory diagrams. Each section builds on the previous ones, helping you understand how pointers work at the memory level while connecting to the concepts from `memorylayout.md`.

<function_calls>
<invoke name="TodoWrite">
<parameter name="todos">[{"id": "1", "content": "Create pointer_arithmetic.md with foundational concepts", "status": "completed"}, {"id": "2", "content": "Add array of pointers vs pointer to array examples", "status": "completed"}, {"id": "3", "content": "Include size allocation examples with memory diagrams", "status": "completed"}, {"id": "4", "content": "Add referencing and dereferencing examples", "status": "completed"}, {"id": "5", "content": "Create parameter passing examples", "status": "completed"}, {"id": "6", "content": "Add string manipulation examples", "status": "completed"}, {"id": "7", "content": "Include function pointer examples", "status": "completed"}, {"id": "8", "content": "Add memory leakage and corruption examples", "status": "completed"}, {"id": "9", "content": "Cover pointer arithmetic operations", "status": "completed"}]