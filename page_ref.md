# Memory Page Management and System Integration

## Table of Contents
1. [Pages and Virtual Memory Overview](#pages-and-virtual-memory-overview)
2. [Intel Registers and Memory Management](#intel-registers-and-memory-management)
3. [OS Page Management for Program Segments](#os-page-management-for-program-segments)
4. [How It All Comes Together](#how-it-all-comes-together)
5. [Page-Level View of memorylayout.md Example](#page-level-view-of-memorylayoutmd-example)

---

## Pages and Virtual Memory Overview

### What are Memory Pages?
- **Page**: Fixed-size block of virtual memory (typically 4KB on x86-64)
- **Frame**: Corresponding fixed-size block of physical RAM
- **Page Table**: OS data structure mapping virtual pages to physical frames

### Virtual Memory Benefits:
1. **Isolation**: Each process sees its own virtual address space
2. **Protection**: OS controls read/write/execute permissions per page
3. **Demand Paging**: Load pages from disk only when needed
4. **Memory Overcommit**: Virtual memory can exceed physical RAM

### Page Structure (4KB pages):
```
Virtual Address (64-bit):
┌─────┬─────────────┬──────────────┐
│ ... │  Page Number│ Page Offset  │
└─────┴─────────────┴──────────────┘
       (bits 12-63)   (bits 0-11)

Physical Address:
┌─────────────┬──────────────┐
│Frame Number │ Page Offset  │
└─────────────┴──────────────┘
```

---

## Intel Registers and Memory Management

### Key Intel x86-64 Registers for Memory:

#### 1. **General Purpose Registers** (Data Movement):
```
RAX, RBX, RCX, RDX     ; 64-bit general purpose
RSI, RDI               ; Source/Destination index
R8-R15                 ; Additional 64-bit registers
```

#### 2. **Stack and Frame Registers**:
```
RSP     ; Stack Pointer - points to top of stack
RBP     ; Base Pointer - frame reference for local variables
```

#### 3. **Instruction Pointer**:
```
RIP     ; Points to next instruction to execute
```

#### 4. **Memory Management Registers** (System Level):
```
CR3     ; Page Directory Base Register (points to page table)
CR0     ; Control register (enables paging, protection)
CR4     ; Extended control (enables large pages, etc.)
```

### Register-to-Memory Relationship:
- **RSP/RBP**: Always contain virtual addresses within stack pages
- **RIP**: Contains virtual address within code pages  
- **RAX-R15**: Can hold virtual addresses pointing to any segment
- **CR3**: Points to physical address of page table

---

## OS Page Management for Program Segments

### How OS Maps Program Segments to Pages:

#### 1. **Code Segment Pages**:
```
Virtual Pages:          Physical Frames:         Permissions:
┌─────────────────┐    ┌─────────────────┐      ┌─────────────┐
│ 0x1000-0x1FFF  │ →  │ Frame #45       │      │ READ+EXEC   │
│ 0x2000-0x2FFF  │ →  │ Frame #67       │      │ READ+EXEC   │
│ 0x3000-0x3FFF  │ →  │ Frame #23       │      │ READ+EXEC   │
└─────────────────┘    └─────────────────┘      └─────────────┘
```
- **Read-only + Executable**: Prevents code modification
- **Shared**: Multiple processes can share same code pages
- **Copy-on-Write**: Not applicable (read-only)

#### 2. **Data Segment Pages**:
```
Virtual Pages:          Physical Frames:         Permissions:
┌─────────────────┐    ┌─────────────────┐      ┌─────────────┐
│ 0x3000-0x3FFF  │ →  │ Frame #89       │      │ READ+WRITE  │
│ 0x4000-0x4FFF  │ →  │ Frame #12       │      │ READ+WRITE  │
└─────────────────┘    └─────────────────┘      └─────────────┘
```
- **Read+Write**: Global/static variables can be modified
- **Process-specific**: Each process has its own data pages

#### 3. **Stack Pages**:
```
Virtual Pages:          Physical Frames:         Permissions:
┌─────────────────┐    ┌─────────────────┐      ┌─────────────┐
│ 0x7FF000-7FFFFF│ →  │ Frame #156      │      │ READ+WRITE  │
│ 0x7FE000-7FEFFF│ →  │ Frame #201      │      │ READ+WRITE  │
│ 0x7FD000-7FDFFF│ →  │ Not allocated   │      │ GUARD PAGE  │
└─────────────────┘    └─────────────────┘      └─────────────┘
```
- **Grows downward**: New pages allocated on stack overflow
- **Guard pages**: Detect stack overflow (causes segfault)
- **Process-specific**: Each thread has its own stack pages

#### 4. **Heap Pages**:
```
Virtual Pages:          Physical Frames:         Permissions:
┌─────────────────┐    ┌─────────────────┐      ┌─────────────┐
│ 0x8000-0x8FFF  │ →  │ Frame #78       │      │ READ+WRITE  │
│ 0x9000-0x9FFF  │ →  │ Not allocated   │      │ On-demand   │
│ 0xA000-0xAFFF  │ →  │ Frame #134      │      │ READ+WRITE  │
└─────────────────┘    └─────────────────┘      └─────────────┘
```
- **Grows upward**: malloc() requests new pages from OS
- **Sparse allocation**: Pages allocated only when needed
- **Fragmentation**: Physical frames may be non-contiguous

---

## How It All Comes Together

### The Complete System Integration:

#### 1. **Program Execution Flow**:
```
CPU Instruction → RIP (virtual addr) → Page Table → Physical Frame → RAM
     ↓
Intel Registers ← Data Load/Store ← Virtual Address ← Program Logic
```

#### 2. **Memory Access Process**:
1. **CPU** executes instruction with virtual address
2. **MMU** (Memory Management Unit) checks **TLB** (Translation Lookaside Buffer)
3. If TLB miss, **Page Table** lookup converts virtual → physical
4. **Permission check**: Read/Write/Execute allowed?
5. **Physical access**: Read/write actual RAM frame
6. **Register update**: Result stored in CPU register

#### 3. **Register Usage in Memory Operations**:
```asm
; Example: malloc() call
mov     $4, %rdi        ; rdi = size parameter (lives in register)
call    malloc          ; rip jumps to malloc code page
                        ; rsp decrements (stack page)
mov     %rax, -16(%rbp) ; rax contains heap virtual address
                        ; stored in stack page at rbp-16
```

#### 4. **OS Memory Management Integration**:
```
Process Virtual Memory Layout:
┌─────────────────────────────────────┐ ← 0x7FFFFFFFFFFF
│          Stack Pages                │   (Multiple 4KB pages)
│    ┌─────────────────────────────┐  │   RSP, RBP point here
│    │ 0x7FF000: Local variables  │  │   
│    │ 0x7FE000: Function params  │  │
│    │ 0x7FD000: Guard page       │  │
│    └─────────────────────────────┘  │
├─────────────────────────────────────┤
│          Free Virtual Space         │   (Unmapped pages)
├─────────────────────────────────────┤
│          Heap Pages                 │   (Dynamic allocation)
│    ┌─────────────────────────────┐  │   malloc() returns addresses here
│    │ 0x8000: Allocated blocks   │  │   
│    │ 0x9000: Free space         │  │
│    │ 0xA000: More blocks        │  │
│    └─────────────────────────────┘  │
├─────────────────────────────────────┤
│          Data Pages                 │   (Global variables)
│    └─────────────────────────────┘  │
├─────────────────────────────────────┤
│          Code Pages                 │   (Program instructions)
│    ┌─────────────────────────────┐  │   RIP points here
│    │ 0x1000: main() function    │  │   
│    │ 0x2000: foo() function     │  │
│    │ 0x3000: library functions  │  │
│    └─────────────────────────────┘  │
└─────────────────────────────────────┘ ← 0x0000000000000000
```

---

## Page-Level View of memorylayout.md Example

### Revisiting the Example with Page Management:

#### Initial State - Program Loading:
```
Page Table Entries Created:
Virtual Page    → Physical Frame   Permissions   Content
0x1000-0x1FFF  → Frame #100       R+X           main() code
0x2000-0x2FFF  → Frame #101       R+X           foo() code  
0x3000-0x3FFF  → Frame #102       R+X           libc (malloc, printf)
0x7FF000-0x7FFFFF → Frame #200    R+W           Stack page (main)
```

#### Step 1: "int stack_var = 5" in main():
```asm
movl    $5, -4(%rbp)    ; Store 5 at virtual address 0x7FFFC
```
**Page Management**:
- Virtual address `0x7FFFC` is in page `0x7FF000-0x7FFFFF`
- MMU translates to physical frame #200, offset 0xFFC
- Permission check: READ+WRITE ✓

#### Step 2: Function call "foo(stack_var)":
```asm
mov     -4(%rbp), %edi  ; Load from stack page to register
call    foo             ; RIP jumps to code page 0x2000
```
**Page Management**:
- Stack grows, may need new page allocation
- New page table entry created if stack overflows current page

#### Step 3: malloc(sizeof(int)) in foo():
```asm
mov     $4, %edi        ; Size in register
call    malloc          ; Jump to libc code page
```
**Page Management**:
1. **malloc()** requests heap page from OS via `brk()` or `mmap()` system call
2. **OS** allocates new virtual page `0x8000-0x8FFF`
3. **Page table** entry created: `0x8000-0x8FFF → Frame #300, R+W`
4. **malloc()** returns virtual address `0x8000` in RAX register

#### Step 4: "*heap_ptr = local_var + 10":
```asm
mov     %eax, (%rdx)    ; Store value at heap address in rdx
```
**Page Management**:
- Virtual address in RDX (e.g., `0x8000`) is in heap page
- MMU translates `0x8000` → Frame #300, offset 0x000
- Permission check: READ+WRITE ✓
- Value written to physical RAM frame #300

#### Step 5: free(heap_ptr):
```asm
mov     -16(%rbp), %rdi ; Load heap pointer to rdi
call    free            ; Jump to libc free() function
```
**Page Management**:
- **free()** marks heap block as available (doesn't immediately release page)
- Page remains mapped in page table for future malloc() calls
- OS may eventually release page with `munmap()` if heap shrinks

### Key Page Management Insights:

1. **Virtual Address Translation**: Every memory access goes through page table
2. **Permission Enforcement**: OS enforces R/W/X at page granularity
3. **Lazy Allocation**: Pages allocated only when first accessed
4. **Register-Memory Bridge**: Registers hold virtual addresses that get translated
5. **Stack Growth**: Automatic page allocation on stack overflow
6. **Heap Management**: malloc/free work with OS page allocator
7. **Code Sharing**: Multiple processes can share read-only code pages
8. **Memory Protection**: Page-level isolation prevents buffer overflows between segments

### Performance Considerations:
- **TLB**: Caches recent virtual→physical translations (critical for performance)
- **Page Faults**: OS handles missing pages (expensive operation)
- **Working Set**: Active pages should fit in physical RAM
- **Cache Locality**: Related data should be on same page when possible