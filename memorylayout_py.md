# Python Memory Management and Variable Storage

## Table of Contents
1. [Python vs C Memory Management](#python-vs-c-memory-management)
2. [Python Variable Types and Storage](#python-variable-types-and-storage)
3. [Python Memory Model](#python-memory-model)
4. [Memory Safety and Garbage Collection](#memory-safety-and-garbage-collection)
5. [When to Worry About Memory in Python](#when-to-worry-about-memory-in-python)
6. [Practical Examples](#practical-examples)

---

## Python vs C Memory Management

### Why Python is Much Simpler:

**C Memory Management:**
- Manual memory allocation (`malloc`/`free`)
- Direct pointer manipulation
- Stack vs heap management
- Memory leaks if `free()` is forgotten
- Segmentation faults from invalid pointers

**Python Memory Management:**
- Automatic memory allocation
- No explicit pointers (everything is a reference)
- Automatic garbage collection
- Memory-safe (no direct memory access)
- No manual `free()` calls needed

```python
# Python - Simple and Safe
def python_example():
    local_var = 10                    # Automatic allocation
    heap_list = [1, 2, 3, 4]        # Automatic heap allocation
    return heap_list                  # No explicit cleanup needed
```

```c
// C - Manual and Error-Prone
void c_example() {
    int local_var = 10;              // Stack allocation
    int *heap_ptr = malloc(16);      // Manual heap allocation
    heap_ptr[0] = 1;
    heap_ptr[1] = 2;
    heap_ptr[2] = 3;
    heap_ptr[3] = 4;
    free(heap_ptr);                  // Manual cleanup required!
}
```

---

## Python Variable Types and Storage

### 1. Everything is an Object
In Python, **all data is stored as objects on the heap**. Variables are just references (names) pointing to these objects.

```python
# All of these create objects on the heap
number = 42           # int object
text = "hello"        # str object  
data = [1, 2, 3]      # list object
person = {"name": "Alice", "age": 30}  # dict object
```

### 2. Python Variable Types and Their Storage:

#### **Immutable Types** (stored on heap, but content cannot change):
```python
# Numbers
integer = 100         # int object - heap allocated
floating = 3.14       # float object - heap allocated
complex_num = 3+4j    # complex object - heap allocated

# Strings
name = "Python"       # str object - heap allocated
multiline = """This is
a multiline string""" # str object - heap allocated

# Tuples
coordinates = (10, 20)     # tuple object - heap allocated
nested = (1, (2, 3), 4)    # nested tuple objects - heap allocated

# Other immutable types
is_valid = True       # bool object - heap allocated
nothing = None        # NoneType object - heap allocated (singleton)
data = b'binary'      # bytes object - heap allocated
```

#### **Mutable Types** (stored on heap, content can change):
```python
# Lists
numbers = [1, 2, 3, 4]         # list object - heap allocated
mixed = [1, "hello", 3.14]     # list with mixed types - heap allocated

# Dictionaries
person = {"name": "Alice"}     # dict object - heap allocated
nested_dict = {"data": {"x": 10}}  # nested dict objects - heap allocated

# Sets
unique_numbers = {1, 2, 3, 4}  # set object - heap allocated

# Custom Objects
class Person:
    def __init__(self, name):
        self.name = name           # Instance stored on heap
        
alice = Person("Alice")        # Person object - heap allocated
```

### 3. Where Variables Are Stored:

```
Python Memory Layout (Conceptual):
┌─────────────────────────────────┐
│           Call Stack            │  ← Function frames, local name bindings
│  ┌─────────────────────────────┐ │
│  │ main():                     │ │
│  │   number → ref to int(42)   │ │  ← Variable names (references)
│  │   text → ref to str("hi")   │ │
│  └─────────────────────────────┘ │
│  ┌─────────────────────────────┐ │
│  │ function():                 │ │
│  │   local_var → ref to obj    │ │
│  └─────────────────────────────┘ │
├─────────────────────────────────┤
│            Python Heap          │  ← All objects live here
│  ┌─────────────────────────────┐ │
│  │ int(42)     [refcount: 1]   │ │  ← Integer object
│  │ str("hi")   [refcount: 1]   │ │  ← String object  
│  │ list([1,2,3]) [refcount: 1] │ │  ← List object
│  │ dict({...}) [refcount: 2]   │ │  ← Dictionary object
│  └─────────────────────────────┘ │
└─────────────────────────────────┘
```

---

## Python Memory Model

### 1. Reference Counting
Python tracks how many references point to each object:

```python
import sys

# Create an object
data = [1, 2, 3]
print(f"References to data: {sys.getrefcount(data)}")  # Usually 2 (data + getrefcount param)

# Create another reference
backup = data
print(f"References to data: {sys.getrefcount(data)}")  # Usually 3

# Remove a reference
backup = None
print(f"References to data: {sys.getrefcount(data)}")  # Back to 2
```

### 2. Object Lifetime Example:

```python
def demonstrate_references():
    # Step 1: Create object, reference count = 1
    numbers = [1, 2, 3, 4]
    
    # Step 2: Create second reference, reference count = 2
    backup = numbers
    
    # Step 3: Modify through one reference (both see changes)
    numbers.append(5)
    print(backup)  # [1, 2, 3, 4, 5] - same object!
    
    # Step 4: Remove one reference, reference count = 1
    backup = None
    
    # Step 5: Function ends, 'numbers' goes out of scope
    # Reference count becomes 0, object is garbage collected
    return "done"

# When function ends, the list object is automatically freed
```

### 3. Memory Sharing and Optimization:

```python
# Small integers are cached (shared objects)
a = 100
b = 100
print(a is b)  # True - same object in memory

# Large integers are separate objects
x = 1000
y = 1000  
print(x is y)  # May be False - separate objects

# String interning for common strings
str1 = "hello"
str2 = "hello"
print(str1 is str2)  # Usually True - shared object
```

---

## Memory Safety and Garbage Collection

### 1. Garbage Collection Overview:
Python uses **automatic garbage collection** with multiple strategies:

#### **Reference Counting (Primary)**:
```python
def ref_counting_demo():
    data = {"key": "value"}  # Object created, refcount = 1
    
    # Add reference
    backup = data            # refcount = 2
    
    # Remove reference  
    data = None              # refcount = 1
    
    # Remove last reference
    backup = None            # refcount = 0 → IMMEDIATE CLEANUP
```

#### **Cycle Detection (Secondary)**:
```python
def cycle_demo():
    # Create a reference cycle
    parent = {"children": []}
    child = {"parent": parent}
    parent["children"].append(child)
    
    # Even when parent and child go out of scope,
    # they reference each other (cycle)
    # Python's cycle detector will find and clean this up
    return "Cycle will be cleaned up automatically"
```

### 2. Garbage Collection in Action:

```python
import gc

def memory_demo():
    # Create objects
    data = [i for i in range(10000)]  # Large list
    processed = [x*2 for x in data]   # Another large list
    
    # Objects exist in memory
    print(f"Objects before cleanup: {len(gc.get_objects())}")
    
    # Clear references
    data = None
    processed = None
    
    # Force garbage collection
    collected = gc.collect()
    print(f"Objects collected: {collected}")
    print(f"Objects after cleanup: {len(gc.get_objects())}")
```

### 3. Memory Management Comparison:

| Aspect | C | Python |
|--------|---|---------|
| **Allocation** | Manual (`malloc`) | Automatic (object creation) |
| **Deallocation** | Manual (`free`) | Automatic (garbage collection) |
| **Memory Leaks** | Common if `free()` forgotten | Rare (only with cycles in older Python) |
| **Dangling Pointers** | Common error | Impossible (references are safe) |
| **Memory Corruption** | Buffer overflows possible | Not possible (bounds checking) |
| **Performance** | Fast, direct access | Slower, but safer |

---

## When to Worry About Memory in Python

### **Generally, You DON'T Need to Worry About Memory Leaks**

Python's garbage collection handles most memory management automatically. However, there are some scenarios to be aware of:

### 1. **Large Data Processing**:
```python
# Problem: Loading huge datasets
def process_large_file():
    # This loads entire file into memory
    with open("huge_file.txt", "r") as f:
        all_data = f.read()  # Could use gigabytes!
    
    # Better: Process line by line
    with open("huge_file.txt", "r") as f:
        for line in f:  # Only one line in memory at a time
            process(line)
```

### 2. **Global Variables and Caches**:
```python
# Problem: Unbounded cache growth
cache = {}  # Global cache

def expensive_function(key):
    if key not in cache:
        cache[key] = compute_expensive_result(key)
    return cache[key]

# Cache grows forever - potential memory issue!

# Better: Use bounded cache
from functools import lru_cache

@lru_cache(maxsize=1000)  # Automatic size limit
def expensive_function(key):
    return compute_expensive_result(key)
```

### 3. **Resource Management**:
```python
# Problem: Not closing resources
def bad_file_handling():
    f = open("file.txt", "r")
    data = f.read()
    # File handle not closed - resource leak!
    return data

# Better: Use context managers
def good_file_handling():
    with open("file.txt", "r") as f:  # Automatically closed
        data = f.read()
    return data
```

### 4. **Event Listeners and Callbacks**:
```python
# Problem: Circular references with callbacks
class EventManager:
    def __init__(self):
        self.listeners = []
    
    def add_listener(self, obj, callback):
        self.listeners.append((obj, callback))
        # obj might reference EventManager → circular reference

# Better: Use weak references
import weakref

class EventManager:
    def __init__(self):
        self.listeners = []
    
    def add_listener(self, obj, callback):
        self.listeners.append((weakref.ref(obj), callback))
        # Weak reference breaks the cycle
```

---

## Practical Examples

### Example 1: Simple Function (Compare to C)

**Python Version:**
```python
def python_memory_example():
    # All objects automatically allocated on heap
    local_number = 42           # int object created
    text_data = "Hello World"   # str object created  
    number_list = [1, 2, 3, 4]  # list object created
    
    print(f"Number: {local_number}")
    print(f"Text: {text_data}")
    print(f"List: {number_list}")
    
    # No cleanup needed - all handled automatically
    return number_list

# When function ends:
# - local_number: reference removed, int object may be garbage collected
# - text_data: reference removed, str object may be garbage collected  
# - number_list: returned, so object survives with new reference
```

**Equivalent C Version:**
```c
int* c_memory_example() {
    int local_number = 42;              // Stack allocated
    char text_data[] = "Hello World";   // Stack allocated
    int* number_list = malloc(4 * sizeof(int));  // Manual heap allocation
    
    number_list[0] = 1;
    number_list[1] = 2; 
    number_list[2] = 3;
    number_list[3] = 4;
    
    printf("Number: %d\n", local_number);
    printf("Text: %s\n", text_data);
    printf("List: [%d, %d, %d, %d]\n", 
           number_list[0], number_list[1], number_list[2], number_list[3]);
    
    // local_number and text_data automatically freed (stack)
    // number_list must be manually freed by caller or memory leak!
    return number_list;
}
```

### Example 2: Memory Usage Monitoring

```python
import sys
import gc

def monitor_memory():
    # Check memory usage
    def get_object_count():
        return len(gc.get_objects())
    
    print(f"Initial objects: {get_object_count()}")
    
    # Create some objects
    data = [i**2 for i in range(1000)]
    strings = [f"string_{i}" for i in range(500)]
    
    print(f"After creation: {get_object_count()}")
    
    # Check size of specific objects
    print(f"Size of data list: {sys.getsizeof(data)} bytes")
    print(f"Size of strings list: {sys.getsizeof(strings)} bytes")
    
    # Clear references
    data = None
    strings = None
    
    # Force garbage collection
    collected = gc.collect()
    print(f"Objects collected: {collected}")
    print(f"Final objects: {get_object_count()}")

monitor_memory()
```

### Example 3: Understanding Reference Behavior

```python
def reference_behavior_demo():
    # Immutable objects
    a = 100
    b = a        # b references same int object as a
    print(f"a is b: {a is b}")  # True - same object
    
    a = 200      # a now references different int object
    print(f"a: {a}, b: {b}")    # a: 200, b: 100
    print(f"a is b: {a is b}")  # False - different objects
    
    # Mutable objects
    list1 = [1, 2, 3]
    list2 = list1    # list2 references same list object as list1
    print(f"list1 is list2: {list1 is list2}")  # True - same object
    
    list1.append(4)  # Modify the shared object
    print(f"list1: {list1}")    # [1, 2, 3, 4]
    print(f"list2: {list2}")    # [1, 2, 3, 4] - same object!
    
    list2 = [5, 6, 7]  # list2 now references different object
    print(f"list1: {list1}")    # [1, 2, 3, 4]
    print(f"list2: {list2}")    # [5, 6, 7]

reference_behavior_demo()
```

---

## Key Takeaways

1. **Memory Management**: Python handles memory automatically - no manual allocation/deallocation needed
2. **All Data is Objects**: Everything lives on the heap as objects, variables are just references
3. **Reference Counting**: Objects are cleaned up when no references remain
4. **Memory Safety**: No buffer overflows, dangling pointers, or memory corruption
5. **Performance Trade-off**: Slightly slower than C, but much safer and easier
6. **Rare Memory Issues**: Only concern yourself with very large datasets, unbounded caches, or resource leaks
7. **Garbage Collection**: Automatic cleanup handles even complex circular references

**Bottom Line**: Focus on your algorithm and logic - Python handles the memory management complexity for you!