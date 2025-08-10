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

```asm
.section .text
.globl main

foo:
    push    %rbp                    ; save caller's frame pointer
    mov     %rsp, %rbp              ; set up new frame pointer
    sub     $24, %rsp               ; allocate 24 bytes for local variables
    
    mov     %edi, -20(%rbp)         ; store param in local stack slot
    mov     -20(%rbp), %eax         ; load param into eax
    add     %eax, %eax              ; multiply by 2 (param * 2)
    mov     %eax, -4(%rbp)          ; store result in local_var
    
    mov     $4, %edi                ; sizeof(int) = 4 bytes
    call    malloc                  ; call malloc(4)
    mov     %rax, -16(%rbp)         ; store malloc return (heap_ptr)
    
    mov     -4(%rbp), %eax          ; load local_var
    add     $10, %eax               ; add 10 to local_var
    mov     -16(%rbp), %rdx         ; load heap_ptr
    mov     %eax, (%rdx)            ; store (local_var + 10) at *heap_ptr
    
    mov     -16(%rbp), %rdx         ; load heap_ptr for printf
    mov     (%rdx), %esi            ; load *heap_ptr (heap value)
    mov     -4(%rbp), %edx          ; load local_var
    mov     $.LC0, %edi             ; load format string address
    mov     $0, %eax                ; clear eax (printf requirement)
    call    printf                  ; call printf
    
    mov     -16(%rbp), %rdi         ; load heap_ptr
    call    free                    ; call free(heap_ptr)
    
    add     $24, %rsp               ; deallocate local variables
    pop     %rbp                    ; restore caller's frame pointer
    ret                             ; return to caller

main:
    push    %rbp                    ; save caller's frame pointer
    mov     %rsp, %rbp              ; set up new frame pointer
    sub     $16, %rsp               ; allocate 16 bytes for local variables
    
    movl    $5, -4(%rbp)            ; store 5 in stack_var
    mov     -4(%rbp), %edi          ; load stack_var as parameter
    call    foo                     ; call foo(stack_var)
    
    mov     $0, %eax                ; set return value to 0
    add     $16, %rsp               ; deallocate local variables
    pop     %rbp                    ; restore caller's frame pointer
    ret                             ; return to OS

.section .rodata
.LC0:
    .string "foo: local=%d, heap=%d\n"
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

### Step 2: Execute "movl $5, -4(%rbp)"
**Instruction Pointer**: 0x1010

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

### Step 3: Prepare for foo(stack_var) call
**Instruction Pointer**: 0x1020

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

### Step 4: Jump to foo() function
**Instruction Pointer**: 0x2000 (foo function start)

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

### Step 5: Execute "add %eax, %eax" (param * 2)
**Instruction Pointer**: 0x2010

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

### Step 6: Execute call malloc
**Instruction Pointer**: 0x2020

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

### Step 7: Execute "mov %eax, (%rdx)" (store value at heap address)
**Instruction Pointer**: 0x2030

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

### Step 8: Execute call printf
**Instruction Pointer**: 0x2030 (printf is a function call)

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

### Step 9: Execute call free
**Instruction Pointer**: 0x2040

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

### Step 10: Return from foo()
**Instruction Pointer**: 0x2050 → 0x1030 (return address from stack)

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

### Step 11: Execute ret from main()
**Instruction Pointer**: 0x1030

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