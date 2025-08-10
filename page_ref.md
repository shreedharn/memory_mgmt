# Memory Page Management and System Integration

## Table of Contents
1. [Operating System Memory Management Fundamentals](#operating-system-memory-management-fundamentals)
2. [Pages and Virtual Memory Overview](#pages-and-virtual-memory-overview)
3. [Page Swapping and Storage](#page-swapping-and-storage)
4. [Stack Frame Management](#stack-frame-management)
5. [Intel Registers and Memory Management](#intel-registers-and-memory-management)
6. [OS Page Management for Program Segments](#os-page-management-for-program-segments)
7. [How It All Comes Together](#how-it-all-comes-together)
8. [Page-Level View of memorylayout.md Example](#page-level-view-of-memorylayoutmd-example)

---

## Operating System Memory Management Fundamentals

### Why Virtual Memory Exists

Without virtual memory, programs would:
1. **Compete for physical RAM**: Multiple programs would need to coordinate memory usage
2. **Risk corruption**: One program could accidentally overwrite another's memory
3. **Be limited by RAM size**: Couldn't run programs larger than available physical memory
4. **Lack security**: No memory protection between processes

### OS Memory Management Goals
```
┌─────────────────────────────────────────────────────────────┐
│                    OS Memory Manager                        │
├─────────────────────────────────────────────────────────────┤
│ 1. ISOLATION:     Each process has private address space   │
│ 2. PROTECTION:    Read/Write/Execute permissions per page  │
│ 3. EFFICIENCY:    Share memory when safe (code, libraries) │
│ 4. ABSTRACTION:   Hide physical memory complexity          │
│ 5. OVERCOMMIT:    Allow virtual memory > physical RAM      │
└─────────────────────────────────────────────────────────────┘
```

### The Virtual Memory Solution

**Problem**: Multiple processes need memory isolation and protection

**Solution**: Each process gets its own virtual address space

```
Process A View:           Process B View:           Physical Reality:
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│ 0x0000-0x0FFF  │      │ 0x0000-0x0FFF  │      │ Frame #1: Proc A│
│ 0x1000-0x1FFF  │ ──┐  │ 0x1000-0x1FFF  │ ──┐  │ Frame #2: Proc B│
│ 0x2000-0x2FFF  │   │  │ 0x2000-0x2FFF  │   │  │ Frame #3: Proc A│
│ ...            │   └──│ ...            │   └──│ Frame #4: Proc B│
│ Same addresses │      │ Same addresses │      │ Different frames│
│ Different data │      │ Different data │      │ No conflicts    │
└─────────────────┘      └─────────────────┘      └─────────────────┘
```

---

## Pages and Virtual Memory Overview

### What are Memory Pages?
- **Page**: Fixed-size block of virtual memory (typically 4KB on x86-64)
- **Frame**: Corresponding fixed-size block of physical RAM
- **Page Table**: OS data structure mapping virtual pages to physical frames

### Virtual Memory Benefits Explained:

#### 1. **Process Isolation**
```
Virtual Address 0x1000 in different processes:

Process A: 0x1000 → Physical Frame #45 (contains Process A's data)
Process B: 0x1000 → Physical Frame #67 (contains Process B's data)
Process C: 0x1000 → Physical Frame #23 (contains Process C's data)

Result: Same virtual address, completely isolated data
```

#### 2. **Memory Protection**
```
Page Permissions (enforced by hardware MMU):
┌─────────────┬─────────┬─────────┬─────────┐
│ Virtual Page│  Read   │  Write  │ Execute │
├─────────────┼─────────┼─────────┼─────────┤
│ Code pages  │   ✓     │    ✗    │    ✓    │
│ Data pages  │   ✓     │    ✓    │    ✗    │
│ Stack pages │   ✓     │    ✓    │    ✗    │
│ Heap pages  │   ✓     │    ✓    │    ✗    │
└─────────────┴─────────┴─────────┴─────────┘
```

#### 3. **Demand Paging**
- Pages loaded from disk **only when accessed**
- Reduces startup time and memory usage
- Enables running programs larger than physical RAM

#### 4. **Memory Overcommit**
- Total virtual memory can exceed physical RAM
- OS swaps less-used pages to disk
- Appears as unlimited memory to applications

### Page Structure (4KB pages):
```
Virtual Address Translation (4KB pages):

Virtual Address (64-bit):
┌─────┬─────────────┬──────────────┐
│ ... │  Page Number│ Page Offset  │
└─────┴─────────────┴──────────────┘
       (bits 12-63)   (bits 0-11)
                │             │
                │             └─── Same in physical (0-4095)
                │
                ▼ (Page Table Lookup)
         ┌─────────────┐
         │   Page      │
         │   Table     │ ──────┐
         │   Entry     │       │
         └─────────────┘       ▼
                        ┌─────────────┬──────────────┐
                        │Frame Number │ Page Offset  │
                        └─────────────┴──────────────┘
                        Physical Address (RAM)

Example: Virtual 0x1234 → Page 0x1, Offset 0x234 → Frame 0x67, Offset 0x234 → Physical 0x67234
```

---

## Page Swapping and Storage

### Where Pages and Frames Live

```
Storage Hierarchy:
┌───────────────────────────────────────────────────────────────────┐
│                           PHYSICAL RAM                            │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐  │
│  │ Frame #1    │ │ Frame #2    │ │ Frame #3    │  │
│  │ (4KB)       │ │ (4KB)       │ │ (4KB)       │  │
│  │ Process A   │ │ Process B   │ │ Process A   │  │
│  └─────────────┘ └─────────────┘ └─────────────┘  │
│  Fast access (nanoseconds) | Limited size (GB)              │
└───────────────────────────────────────────────────────────────────┘
                               │
                               ▼ (When RAM is full)
┌───────────────────────────────────────────────────────────────────┐
│                       SWAP SPACE (DISK)                        │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐  │
│  │ Swapped     │ │ Swapped     │ │ Swapped     │  │
│  │ Page #1     │ │ Page #2     │ │ Page #3     │  │
│  │ (Process C) │ │ (Process D) │ │ (Process E) │  │
│  └─────────────┘ └─────────────┘ └─────────────┘  │
│  Slow access (milliseconds) | Large size (TB)                │
└───────────────────────────────────────────────────────────────────┘
```

### When and How Pages Are Swapped

#### Scenario 1: RAM is Full, New Page Needed
```
BEFORE SWAP:                    AFTER SWAP:
RAM: [A1][B1][C1] (FULL)       RAM: [A1][B1][NEW]
Disk: [old pages]               Disk: [old pages][C1]

Steps:
1. OS selects victim page (C1) using LRU algorithm
2. If C1 is "dirty" (modified), write C1 to disk
3. Mark C1's page table entry as "swapped out"
4. Load NEW page into C1's frame
5. Update NEW page's page table entry
```

#### Scenario 2: Process Accesses Swapped-Out Page
```
Step 1: Page Fault Occurs
CPU tries to access virtual address 0x2000
MMU checks page table: "Page swapped out" → **PAGE FAULT**

Step 2: OS Page Fault Handler
┌──────────────────────────────────────────────────┐
│ 1. Pause process execution                       │
│ 2. Find free RAM frame (or evict another page)  │
│ 3. Read page from disk into RAM frame           │
│ 4. Update page table: virtual → physical       │
│ 5. Resume process execution                      │
└──────────────────────────────────────────────────┘
```

#### Complete Swapping Example
```
Initial State:
RAM:  [Page A][Page B][Page C]
Disk: [Page D][Page E]

Process needs Page D:
1. ┌─────────────────────────────────────────────────┐
   │ Page D access → PAGE FAULT (not in RAM)        │
   └─────────────────────────────────────────────────┘
2. OS selects Page A to evict (least recently used)
3. Write Page A to disk: RAM: [Page A][Page B][Page C] → Disk
4. Read Page D from disk into Page A's frame

Final State:
RAM:  [Page D][Page B][Page C]
Disk: [Page A][Page E]
```

---

## Stack Frame Management

### Stack Frames: Creation and Destruction

Each function call creates a **stack frame** containing:
- Local variables
- Function parameters  
- Return address
- Saved register values

### Stack Frame Lifecycle During Function Calls

```
Function Call Sequence: main() calls foo() calls bar()

1. INITIAL STATE (main only):
   Stack Pages in RAM:
   ┌───────────────────────────────────────┐ ← Stack grows down
   │                                       │
   │ ┌─────────────────────────────────┐ │
   │ │         main() frame           │ │ ← RSP, RBP
   │ │ - local variables: stack_var=5 │ │
   │ └─────────────────────────────────┘ │
   │                                       │
   └───────────────────────────────────────┘

2. AFTER main() calls foo():
   ┌───────────────────────────────────────┐
   │ ┌─────────────────────────────────┐ │ ← New RSP
   │ │          foo() frame            │ │
   │ │ - param=5                      │ │
   │ │ - local_var=10                 │ │
   │ │ - heap_ptr=0x8000              │ │
   │ │ - return address (to main)     │ │
   │ └─────────────────────────────────┘ │
   │ ┌─────────────────────────────────┐ │ ← Old RBP
   │ │         main() frame           │ │
   │ │ - local variables: stack_var=5 │ │
   │ └─────────────────────────────────┘ │
   └───────────────────────────────────────┘

3. AFTER foo() calls bar():
   ┌───────────────────────────────────────┐
   │ ┌─────────────────────────────────┐ │ ← RSP
   │ │          bar() frame            │ │
   │ │ - bar's local variables        │ │
   │ │ - return address (to foo)      │ │
   │ └─────────────────────────────────┘ │
   │ ┌─────────────────────────────────┐ │
   │ │          foo() frame            │ │
   │ │ - param, local_var, heap_ptr   │ │
   │ └─────────────────────────────────┘ │
   │ ┌─────────────────────────────────┐ │
   │ │         main() frame           │ │
   │ └─────────────────────────────────┘ │
   └───────────────────────────────────────┘
```

### Stack Frame Destruction (Function Returns)

```
Return Sequence: bar() returns to foo() returns to main()

1. bar() executes 'ret' instruction:
   • RSP moves up (stack shrinks)
   • bar()'s frame memory becomes "available" 
   • Control returns to foo()
   ┌───────────────────────────────────────┐
   │ ┌─────────────────────────────────┐ │
   │ │      [freed - old bar() frame]   │ │
   │ └─────────────────────────────────┘ │
   │ ┌─────────────────────────────────┐ │ ← RSP back here
   │ │          foo() frame            │ │
   │ └─────────────────────────────────┘ │
   └───────────────────────────────────────┘

2. foo() executes 'ret' instruction:
   • Both foo() and bar() frames now "freed"
   • Control returns to main()
   ┌───────────────────────────────────────┐
   │    [freed - old foo() and bar() frames]    │
   │ ┌─────────────────────────────────┐ │ ← RSP back to main
   │ │         main() frame           │ │
   │ └─────────────────────────────────┘ │
   └───────────────────────────────────────┘
```

### Stack Overflow and Page Allocation

When stack grows too large:
```
Stack Growth Detection:
┌────────────────────────────────────────────────────────────────┐
│                       Stack Pages                        │
│  ┌──────────────────────────────────────────────────┐  │
│  │               New Page               │  │ ← OS allocates
│  │      (allocated on demand)       │  │   when RSP 
│  └──────────────────────────────────────────────────┘  │   goes too low
│  ┌──────────────────────────────────────────────────┐  │
│  │          Current Stack Page         │  │ ← RSP here
│  │   [main][foo][bar] frames         │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │              GUARD PAGE             │  │ ← Detects overflow
│  │        (causes SIGSEGV if touched)   │  │   (no permissions)
│  └──────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
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