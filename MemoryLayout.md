# Memory Layout and Program Execution

## Table of Contents
1. [Function Loading and Execution](#function-loading-and-execution)
2. [Stack vs Heap Details](#stack-vs-heap-details)
3. [Step-by-Step Program Execution](#step-by-step-program-execution)
4. [Memory Addresses in Action](#memory-addresses-in-action)

---

## Function Loading and Execution

When a program starts, the operating system loads it into memory with distinct segments:

```
Memory Layout Overview:
┌─────────────────┐ ← 0x7FFF... (High Memory)
│     Stack       │ (grows downward ↓)
│                 │
├─────────────────┤
│   Free Space    │
├─────────────────┤
│     Heap        │ (grows upward ↑)
│                 │
├─────────────────┤
│ Data Segment    │ (global/static variables)
├─────────────────┤
│ Code Segment    │ (program instructions)
│  - main()       │
│  - foo()        │
│  - malloc()     │
│  - free()       │
└─────────────────┘ ← 0x0000... (Low Memory)
```

### Function Loading Process:
1. **Program loader** reads executable file
2. **Code segment** loaded with all function instructions
3. **Entry point** set to main() function address
4. **Instruction Pointer (IP)** initialized to main() address

---

## Stack vs Heap Details

### Stack Storage:
- **Local variables** (int x, char buffer[100])
- **Function parameters** (passed by value/reference)
- **Return addresses** (where to return after function call)
- **CPU register states** (saved during function calls)

### Heap Storage:
- **malloc() allocated blocks**
- **Dynamic arrays** (created at runtime)
- **Structures with unknown size** at compile time
- **Large data structures** that exceed stack limits

---

## Step-by-Step Program Execution

Let's trace through this example program:

```c
#include <stdio.h>
#include <stdlib.h>

void foo(int param) {                    // Called by main(), param receives stack_var value (5)
    int local_var = param * 2;           // Stack allocation: local_var = 5 * 2 = 10
    int *heap_ptr = malloc(sizeof(int)); // Heap allocation: request 4 bytes, returns heap address
    *heap_ptr = local_var + 10;          // Store value 20 (10+10) at heap address
    
    printf("foo: local=%d, heap=%d\n", local_var, *heap_ptr); // Print: local=10, heap=20
    
    free(heap_ptr);                      // Deallocate heap memory, make address available
}                                        // Return to main(), local variables destroyed

int main() {                             // Entry point: called by OS runtime
    int stack_var = 5;                   // Stack allocation: store literal 5 in main's frame
    foo(stack_var);                      // Function call: pass stack_var value to foo()
    return 0;                            // Return success status to OS, program terminates
}
```

### Step 1: Program Start
**Instruction Pointer**: 0x1000 (main function start)

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │ ←IP│                 │    │                 │
│ 0x1010: int...  │    │                 │    │                 │
│ 0x1020: foo()   │    │                 │    │                 │
│ 0x1030: return  │    │                 │    │                 │
│ 0x2000: foo()   │    │                 │    │                 │
│ 0x2010: int...  │    │                 │    │                 │
│ 0x2020: malloc  │    │                 │    │                 │
│ 0x2030: printf  │    │                 │    │                 │
│ 0x2040: free()  │    │                 │    │                 │
│ 0x2050: return  │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Step 2: Execute "int stack_var = 5;" (main's local variable)
**Instruction Pointer**: 0x1010
**Action**: Allocate 4 bytes on main's stack frame and store literal value 5

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │                 │    │                 │
│ 0x1010: int...  │ ←IP│                 │    │                 │
│ 0x1020: foo()   │    │                 │    │                 │
│ 0x1030: return  │    │                 │    │                 │
│ 0x2000: foo()   │    │ 0x7FF0: stack_  │    │                 │
│ 0x2010: int...  │    │         var=5   │    │                 │
│ 0x2020: malloc  │    └─────────────────┘    │                 │
│ 0x2030: printf  │                           │                 │
│ 0x2040: free()  │                           │                 │
│ 0x2050: return  │                           │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 3: Prepare for foo(stack_var) call (main calls foo)
**Instruction Pointer**: 0x1020
**Action**: Copy stack_var value (5) to parameter, push return address (0x1030) on stack

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FE0: param=5 │    │                 │
│ 0x1010: int...  │    │ 0x7FE8: ret addr│    │                 │
│ 0x1020: foo()   │ ←IP│         =0x1030 │    │                 │
│ 0x1030: return  │    │ 0x7FF0: stack_  │    │                 │
│ 0x2000: foo()   │    │         var=5   │    │                 │
│ 0x2010: int...  │    └─────────────────┘    │                 │
│ 0x2020: malloc  │                           │                 │
│ 0x2030: printf  │                           │                 │
│ 0x2040: free()  │                           │                 │
│ 0x2050: return  │                           │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 4: Jump to foo() function (control transfer to foo)
**Instruction Pointer**: 0x2000 (foo function start)
**Action**: Begin executing foo with param=5, main's execution paused

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FE0: param=5 │    │                 │
│ 0x1010: int...  │    │ 0x7FE8: ret addr│    │                 │
│ 0x1020: foo()   │    │         =0x1030 │    │                 │
│ 0x1030: return  │    │ 0x7FF0: stack_  │    │                 │
│ 0x2000: foo()   │ ←IP│         var=5   │    │                 │
│ 0x2010: int...  │    └─────────────────┘    │                 │
│ 0x2020: malloc  │                           │                 │
│ 0x2030: printf  │                           │                 │
│ 0x2040: free()  │                           │                 │
│ 0x2050: return  │                           │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 5: Execute "int local_var = param * 2;" (foo's local variable)
**Instruction Pointer**: 0x2010
**Action**: Allocate 4 bytes in foo's frame, calculate 5*2=10, store result

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FD8: local_  │    │                 │
│ 0x1010: int...  │    │         var=10  │    │                 │
│ 0x1020: foo()   │    │ 0x7FE0: param=5 │    │                 │
│ 0x1030: return  │    │ 0x7FE8: ret addr│    │                 │
│ 0x2000: foo()   │    │         =0x1030 │    │                 │
│ 0x2010: int...  │ ←IP│ 0x7FF0: stack_  │    │                 │
│ 0x2020: malloc  │    │         var=5   │    │                 │
│ 0x2030: printf  │    └─────────────────┘    │                 │
│ 0x2040: free()  │                           │                 │
│ 0x2050: return  │                           │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 6: Execute malloc(sizeof(int)) (heap allocation)
**Instruction Pointer**: 0x2020
**Action**: Request 4 bytes from heap manager, receive address 0x8000

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FD0: heap_ptr│    │ 0x8000: [unin- │
│ 0x1010: int...  │    │         =0x8000 │    │         itial-  │
│ 0x1020: foo()   │    │ 0x7FD8: local_  │    │         ized]   │
│ 0x1030: return  │    │         var=10  │    │                 │
│ 0x2000: foo()   │    │ 0x7FE0: param=5 │    │                 │
│ 0x2010: int...  │    │ 0x7FE8: ret addr│    │                 │
│ 0x2020: malloc  │ ←IP│         =0x1030 │    │                 │
│ 0x2030: printf  │    │ 0x7FF0: stack_  │    │                 │
│ 0x2040: free()  │    │         var=5   │    │                 │
│ 0x2050: return  │    └─────────────────┘    │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 7: Execute "*heap_ptr = local_var + 10;" (store to heap)
**Instruction Pointer**: 0x2030
**Action**: Calculate 10+10=20, store value 20 at heap address 0x8000

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FD0: heap_ptr│    │ 0x8000: [20]   │
│ 0x1010: int...  │    │         =0x8000─┼────┼──→             │
│ 0x1020: foo()   │    │ 0x7FD8: local_  │    │                 │
│ 0x1030: return  │    │         var=10  │    │                 │
│ 0x2000: foo()   │    │ 0x7FE0: param=5 │    │                 │
│ 0x2010: int...  │    │ 0x7FE8: ret addr│    │                 │
│ 0x2020: malloc  │    │         =0x1030 │    │                 │
│ 0x2030: printf  │ ←IP│ 0x7FF0: stack_  │    │                 │
│ 0x2040: free()  │    │         var=5   │    │                 │
│ 0x2050: return  │    └─────────────────┘    │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 8: Execute printf() statement (output to console)
**Instruction Pointer**: 0x2030 (printf is a function call)
**Action**: Read local_var (10) and *heap_ptr (20), format and print output

**Output**: `foo: local=10, heap=20`

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FD0: heap_ptr│    │ 0x8000: [20]   │
│ 0x1010: int...  │    │         =0x8000─┼────┼──→             │
│ 0x1020: foo()   │    │ 0x7FD8: local_  │    │                 │
│ 0x1030: return  │    │         var=10  │    │                 │
│ 0x2000: foo()   │    │ 0x7FE0: param=5 │    │                 │
│ 0x2010: int...  │    │ 0x7FE8: ret addr│    │                 │
│ 0x2020: malloc  │    │         =0x1030 │    │                 │
│ 0x2030: printf  │    │ 0x7FF0: stack_  │    │                 │
│ 0x2040: free()  │ ←IP│         var=5   │    │                 │
│ 0x2050: return  │    └─────────────────┘    │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 9: Execute free(heap_ptr) (heap deallocation)
**Instruction Pointer**: 0x2040
**Action**: Return heap block at 0x8000 to heap manager, mark as available

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │ 0x7FD0: heap_ptr│    │ 0x8000: [freed] │
│ 0x1010: int...  │    │         =0x8000 │    │         (avail- │
│ 0x1020: foo()   │    │ 0x7FD8: local_  │    │         able)   │
│ 0x1030: return  │    │         var=10  │    │                 │
│ 0x2000: foo()   │    │ 0x7FE0: param=5 │    │                 │
│ 0x2010: int...  │    │ 0x7FE8: ret addr│    │                 │
│ 0x2020: malloc  │    │         =0x1030 │    │                 │
│ 0x2030: printf  │    │ 0x7FF0: stack_  │    │                 │
│ 0x2040: free()  │    │         var=5   │    │                 │
│ 0x2050: return  │ ←IP└─────────────────┘    │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 10: Return from foo() (control back to main)
**Instruction Pointer**: 0x2050 → 0x1030 (return address from stack)
**Action**: Destroy foo's stack frame, resume main's execution

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │                 │    │ 0x8000: [freed] │
│ 0x1010: int...  │    │                 │    │                 │
│ 0x1020: foo()   │    │                 │    │                 │
│ 0x1030: return  │ ←IP│ 0x7FF0: stack_  │    │                 │
│ 0x2000: foo()   │    │         var=5   │    │                 │
│ 0x2010: int...  │    └─────────────────┘    │                 │
│ 0x2020: malloc  │                           │                 │
│ 0x2030: printf  │                           │                 │
│ 0x2040: free()  │                           │                 │
│ 0x2050: return  │                           │                 │
└─────────────────┘                           └─────────────────┘
```

### Step 11: Execute return 0 from main() (program termination)
**Instruction Pointer**: 0x1030
**Action**: Return success status (0) to OS, destroy main's frame

```
Code Segment:          Stack:                 Heap:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ 0x1000: main()  │    │                 │    │                 │
│ 0x1010: int...  │    │                 │    │                 │
│ 0x1020: foo()   │    │                 │    │                 │
│ 0x1030: return  │ ←IP│     [empty]     │    │     [freed]     │
│ 0x2000: foo()   │    │                 │    │                 │
│ 0x2010: int...  │    │                 │    │                 │
│ 0x2020: malloc  │    │                 │    │                 │
│ 0x2030: printf  │    │                 │    │                 │
│ 0x2040: free()  │    │                 │    │                 │
│ 0x2050: return  │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

**Program terminates** - OS reclaims all memory

---

## Memory Addresses in Action

### Address Relationships:
```
Memory Type    | Address Range | Content
───────────────┼───────────────┼─────────────────────
Code Segment   | 0x1000-0x2FFF | Program instructions
Data Segment   | 0x3000-0x3FFF | Global/static variables  
Heap          | 0x8000-0x8FFF | malloc() allocated blocks
Stack         | 0x7000-0x7FFF | Local vars, parameters
```

### Key Observations:

1. **Instruction Pointer Movement**:
   - Increments sequentially: 0x1000 → 0x1010 → 0x1020
   - Jumps on function calls: 0x1020 → 0x2000
   - Returns via saved address: 0x2050 → 0x1030

2. **Stack Growth** (downward):
   - main() frame: 0x7FF0
   - foo() frame: 0x7FD0-0x7FE8
   - Parameters passed by copying values

3. **Heap Allocation**:
   - malloc() returns: 0x8000
   - Pointer stored on stack: 0x7FD0
   - Actual data lives at: 0x8000

4. **Memory Cleanup**:
   - Stack: Automatic (frame popped on return)
   - Heap: Manual (free() required)
   - Code: Persistent (loaded once)

### Memory Layout Summary:
- **Stack variables** exist only during function execution
- **Heap variables** persist until explicitly freed
- **Pointers** bridge stack and heap (address stored on stack, data on heap)
- **Instruction Pointer** controls execution flow through code segment
- **Memory addresses** provide precise locations for debugging and understanding program behavior