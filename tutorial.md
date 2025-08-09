# Memory Management in C: Stack vs Heap Tutorial

## Table of Contents
1. [Stack vs Heap Overview](#stack-vs-heap-overview)
2. [Function Call Mechanism](#function-call-mechanism)
3. [Local Variables and Stack Frames](#local-variables-and-stack-frames)
4. [Function Exit and Stack Cleanup](#function-exit-and-stack-cleanup)
5. [Instruction Pointer](#instruction-pointer)
6. [Dynamic Memory Allocation](#dynamic-memory-allocation)
7. [Pointer Examples](#pointer-examples)
8. [Memory Layout Visualization](#memory-layout-visualization)

---

## Stack vs Heap Overview

### Stack
- **Fast allocation/deallocation** - managed automatically
- **LIFO (Last In, First Out)** structure
- **Limited size** (typically 1-8MB)
- **Automatic cleanup** when variables go out of scope
- Stores: local variables, function parameters, return addresses

### Heap
- **Slower allocation/deallocation** - managed manually
- **No specific order** - can allocate/free in any sequence
- **Large size** (limited by available RAM)
- **Manual cleanup** required (malloc/free, calloc/free)
- Stores: dynamically allocated memory

```
Memory Layout:
┌─────────────────┐ ← High Memory Address
│     Stack       │ (grows downward)
│  ┌───────────┐  │
│  │ Function  │  │
│  │ Frames    │  │
│  └───────────┘  │
├─────────────────┤
│   Free Space    │
├─────────────────┤
│     Heap        │ (grows upward)
│  ┌───────────┐  │
│  │ Dynamic   │  │
│  │ Memory    │  │
│  └───────────┘  │
├─────────────────┤
│ Data Segment    │ (global/static variables)
├─────────────────┤
│ Code Segment    │ (program instructions)
└─────────────────┘ ← Low Memory Address
```

---

## Function Call Mechanism

When a function is called, the following happens:

1. **Arguments pushed** onto the stack
2. **Return address pushed** (where to return after function completes)
3. **New stack frame created** for the function
4. **Control transferred** to the function

```
Before function call:
Stack:
┌─────────────────┐
│   main() vars   │
└─────────────────┘

After function call foo(int x, int y):
Stack:
┌─────────────────┐
│   foo() frame   │
│   - local vars  │
│   - parameters  │
├─────────────────┤
│  return address │
├─────────────────┤
│   main() vars   │
└─────────────────┘
```

---

## Local Variables and Stack Frames

Local variables are created on the stack within the function's stack frame:

```c
void example_function() {
    int a = 10;        // Created on stack
    char buffer[100];  // Created on stack
    float f = 3.14;    // Created on stack
    
    // All variables exist in this function's stack frame
}
```

**Stack Frame Contents:**
```
┌─────────────────┐ ← Stack Pointer (SP)
│ Local Variables │
│   - int a       │
│   - char[100]   │
│   - float f     │
├─────────────────┤
│   Parameters    │
├─────────────────┤
│ Return Address  │
├─────────────────┤
│ Previous Frame  │
└─────────────────┘ ← Frame Pointer (FP)
```

---

## Function Exit and Stack Cleanup

When a function exits:

1. **Local variables are destroyed** (stack frame is popped)
2. **Control returns** to the calling function
3. **Stack pointer moves back** to previous position
4. **Memory is automatically reclaimed**

```
Before function exit:
┌─────────────────┐
│   foo() frame   │ ← Will be destroyed
├─────────────────┤
│  return address │
├─────────────────┤
│   main() vars   │
└─────────────────┘

After function exit:
┌─────────────────┐
│   main() vars   │ ← Stack pointer back here
└─────────────────┘
```

---

## Instruction Pointer

The **Instruction Pointer (IP)** or **Program Counter (PC)**:
- **Lives in the CPU register**
- **Points to the current instruction** being executed
- **Automatically incremented** after each instruction
- **Modified by function calls, jumps, and returns**

```
Memory:
┌─────────────────┐
│ Instruction 1   │ ← IP points here initially
│ Instruction 2   │
│ Function Call   │ ← IP jumps to function
│ Instruction 3   │
└─────────────────┘

Function:
┌─────────────────┐
│ Function Start  │ ← IP points here during function
│ Function Body   │
│ Return         │ ← IP returns to caller
└─────────────────┘
```

---

## Dynamic Memory Allocation

### malloc() Example

```c
#include <stdlib.h>

void malloc_example() {
    // Allocate memory on heap
    int *ptr = malloc(sizeof(int));
    
    if (ptr != NULL) {
        *ptr = 42;          // Use the memory
        printf("%d\n", *ptr);
        free(ptr);          // Free the memory
        ptr = NULL;         // Avoid dangling pointer
    }
}
```

**Memory State:**
```
Stack (malloc_example):
┌─────────────────┐
│ int *ptr        │ ← Points to heap memory
└─────────────────┘

Heap:
┌─────────────────┐
│ int (value: 42) │ ← Allocated by malloc
└─────────────────┘
```

**After free():**
```
Stack (malloc_example):
┌─────────────────┐
│ int *ptr = NULL │ ← No longer points to heap
└─────────────────┘

Heap:
┌─────────────────┐
│    (freed)      │ ← Memory returned to system
└─────────────────┘
```

---

## Pointer Examples

### 1. Simple Pointer

```c
void simple_pointer_example() {
    int value = 100;    // Variable on stack
    int *ptr = &value;  // Pointer on stack, points to value
    
    printf("Value: %d\n", *ptr);  // Dereference pointer
}
```

**Memory Layout:**
```
Stack:
┌─────────────────┐
│ int *ptr        │ ← Contains address of 'value'
│    │            │
│    └─────────┐  │
│              ▼  │
│ int value=100   │ ← Actual data
└─────────────────┘
```

### 2. Pointer to Array

```c
void pointer_to_array_example() {
    int arr[5] = {1, 2, 3, 4, 5};  // Array on stack
    int (*ptr_to_array)[5] = &arr;  // Pointer to entire array
    
    // Access elements: (*ptr_to_array)[index]
    printf("First element: %d\n", (*ptr_to_array)[0]);
}
```

**Memory Layout:**
```
Stack:
┌─────────────────┐
│ int (*ptr)[5]   │ ← Points to entire array
│    │            │
│    └─────────┐  │
│              ▼  │
│ int arr[5]      │ ← [1][2][3][4][5]
│ [0][1][2][3][4] │
└─────────────────┘
```

### 3. Array of Pointers

```c
void array_of_pointers_example() {
    int a = 10, b = 20, c = 30;
    int *ptr_array[3];              // Array of 3 pointers
    
    ptr_array[0] = &a;
    ptr_array[1] = &b;
    ptr_array[2] = &c;
    
    for (int i = 0; i < 3; i++) {
        printf("Value %d: %d\n", i, *ptr_array[i]);
    }
}
```

**Memory Layout:**
```
Stack:
┌─────────────────┐
│ int *ptr_array[3]│
│   [0] ─────┐     │
│   [1] ───┐ │     │
│   [2] ─┐ │ │     │
│        │ │ │     │
│ int c=30←┘ │ │   │
│ int b=20←──┘ │   │
│ int a=10←────┘   │
└─────────────────┘
```

### 4. Dynamic Array with malloc

```c
void dynamic_array_example() {
    int size = 5;
    int *dynamic_array = malloc(size * sizeof(int));
    
    if (dynamic_array != NULL) {
        // Initialize array
        for (int i = 0; i < size; i++) {
            dynamic_array[i] = i * 2;
        }
        
        // Use array
        for (int i = 0; i < size; i++) {
            printf("%d ", dynamic_array[i]);
        }
        
        free(dynamic_array);           // Free heap memory
        dynamic_array = NULL;          // Avoid dangling pointer
    }
}
```

**Memory Layout:**
```
Stack:
┌─────────────────┐
│ int *dynamic_   │ ← Points to heap
│     array       │
│    │            │
└────┼────────────┘
     │
     ▼
Heap:
┌─────────────────┐
│ [0][2][4][6][8] │ ← Allocated by malloc
└─────────────────┘
```

---

## Memory Layout Visualization

### Complete Program Memory Layout

```c
#include <stdio.h>
#include <stdlib.h>

int global_var = 100;           // Data segment
static int static_var = 200;    // Data segment

void function_example() {
    int local = 50;             // Stack
    int *heap_ptr = malloc(sizeof(int));  // Heap pointer on stack,
                                         // allocated memory on heap
    *heap_ptr = 75;
    
    printf("Local: %d, Heap: %d\n", local, *heap_ptr);
    free(heap_ptr);
}

int main() {
    function_example();
    return 0;
}
```

**Complete Memory Layout:**
```
┌─────────────────┐ ← 0xFFFFFFFF (High Memory)
│     Stack       │
│ ┌─────────────┐ │
│ │main() frame │ │
│ └─────────────┘ │
│ ┌─────────────┐ │
│ │func() frame │ │
│ │ int local   │ │
│ │ int *ptr    │─┼─┐
│ └─────────────┘ │ │
├─────────────────┤ │
│                 │ │
│   Free Space    │ │
│                 │ │
├─────────────────┤ │
│     Heap        │ │
│ ┌─────────────┐ │ │
│ │ malloc data │←┼─┘
│ │ (int: 75)   │ │
│ └─────────────┘ │
├─────────────────┤
│ Data Segment    │
│ global_var=100  │
│ static_var=200  │
├─────────────────┤
│ Code Segment    │
│ main() code     │
│ function_example│
│ printf, malloc  │
└─────────────────┘ ← 0x00000000 (Low Memory)
```

---

## Key Takeaways

1. **Stack**: Fast, automatic, limited size - for local variables and function calls
2. **Heap**: Slower, manual management, large size - for dynamic allocation
3. **Function calls** create new stack frames that are automatically cleaned up
4. **Instruction pointer** lives in CPU and tracks current execution location
5. **Always free** dynamically allocated memory to prevent memory leaks
6. **Pointers** can point to stack variables, heap memory, or other pointers
7. **Understanding memory layout** helps debug segmentation faults and memory leaks