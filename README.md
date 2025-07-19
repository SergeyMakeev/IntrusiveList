# Intrusive List Implementation

A modern, C++17 compatible intrusive doubly-linked list implementation. This is a single-header, header-only library that provides efficient intrusive containers with sentinel circular list optimization.

## Quick Start

```bash
# Clone or download the intrusive_list.hpp file
# Include it in your project
#include "intrusive_list/intrusive_list.hpp"

# Or build the complete tests
mkdir build && cd build
cmake ..
cmake --build .
ctest                    # Run all tests
```

## Features

- **C++17 Compatible**: Uses modern C++ features like `auto` template parameters
- **Header-Only**: Single file implementation - just include `intrusive_list.hpp`
- **STL-Compatible**: Provides standard iterators and container interface
- **Move Semantics**: Full support for move constructors and move assignment
- **Type Safety**: Compile-time checks ensure correct usage
- **Automatic Cleanup**: Objects are automatically unlinked when destroyed
- **Multiple Lists**: Objects can be members of multiple lists simultaneously
- **High Performance**: Sentinel circular list eliminates redundant null checks
- **Cross-Platform**: Tested on Windows (MSVC), Linux (GCC), and macOS (Clang)
- **Comprehensive Tests**: 20+ unit tests covering all functionality
- **CMake Integration**: Modern CMake build system with automatic dependency management

## Basic Usage

```cpp
#include "intrusive_list/intrusive_list.hpp"

// 1. Define your class with a list_node member
struct MyObject {
    int value;
    dod::list_node link;  // The intrusive link
    
    MyObject(int v) : value(v) {}
};

// 2. Create a list specifying the link member pointer
dod::intrusive_list<&MyObject::link> my_list;

// 3. Use like a standard container
MyObject obj1(42);
MyObject obj2(84);

my_list.push_back(obj1);
my_list.push_back(obj2);

// 4. Iterate using range-based for loops or iterators
for (const auto& obj : my_list) {
    std::cout << obj.value << std::endl;
}
```

## Key Differences from Standard Containers

### Object Ownership
- **Standard containers**: Own the objects (copy/move them in)
- **Intrusive containers**: Don't own objects, just link existing objects

### Memory Management
- **Standard containers**: Allocate memory for elements
- **Intrusive containers**: No dynamic allocation, use existing object memory

### Usage Pattern
```cpp
// Standard container (owns objects)
std::list<MyObject> std_list;
std_list.emplace_back(42);  // Creates object inside the container

// Intrusive container (links existing objects)
dod::intrusive_list<&MyObject::link> int_list;
MyObject obj(42);           // Object exists independently
int_list.push_back(obj);    // Links the existing object
```

## API Reference

### Container Operations
- `push_front(obj)` - Add object to front
- `push_back(obj)` - Add object to back  
- `pop_front()` - Remove first object
- `pop_back()` - Remove last object
- `insert(pos, obj)` - Insert object before position
- `erase(pos)` - Remove object at position
- `erase(obj)` - Remove specific object
- `clear()` - Remove all objects
- `empty()` - Check if list is empty
- `swap(other)` - Swap with another list

### Element Access
- `front()` - Get reference to first object
- `back()` - Get reference to last object

### Iterators
- `begin()`, `end()` - Get iterators
- `cbegin()`, `cend()` - Get const iterators

### Utility
- `can_insert(obj)` - Check if object can be safely inserted
- `node_to_object(node)` - Convert list_node pointer to object pointer (public utility)

## Multiple Lists Example

Objects can be members of multiple lists by having multiple `list_node` members:

```cpp
struct Employee {
    std::string name;
    dod::list_node dept_link;     // For department list
    dod::list_node project_link;  // For project list
};

// Same employee can be in both lists
dod::intrusive_list<&Employee::dept_link> engineering;
dod::intrusive_list<&Employee::project_link> project_alpha;

Employee alice("Alice");
engineering.push_back(alice);   // Alice is in engineering dept
project_alpha.push_back(alice); // Alice also works on project alpha
```

## Performance Optimizations

This implementation uses several optimization techniques:

### Sentinel Circular List
- **No null pointer checks**: Sentinels eliminate branches in hot paths
- **Simplified edge cases**: Empty and non-empty lists handled uniformly
- **Self-unlinking nodes**: Nodes can remove themselves without list reference

### Optimized Operations
- **Single condition in `is_linked()`**: Only checks one pointer since both are always null or non-null together
- **Direct pointer arithmetic**: `front()` and `back()` avoid iterator overhead
- **Template consolidation**: Single `node_to_object()` handles both const and non-const cases
- **Eliminated redundant checks**: Move operations and unlink avoid unnecessary null checks

## Important Notes

### Safety Considerations
1. **Objects must outlive the list**: The list doesn't manage object lifetime
2. **No duplicate insertion**: An object can only be in one instance of a specific list type at a time
3. **Automatic unlinking**: Objects are automatically removed from lists when destroyed
4. **Debug assertions**: Strategic assertions catch programming errors in debug builds

### Performance Benefits
- **Cache-friendly**: No pointer chasing to separate allocations
- **No allocations**: Zero dynamic memory allocation
- **Constant time operations**: All operations are O(1) except clear()
- **Minimal branches**: Optimized for modern CPU branch prediction

### Thread Safety
- **Not thread-safe**: Like standard containers, requires external synchronization for concurrent access

## Compilation

```bash
# Linux/macOS with GCC
g++ -std=c++17 -Wall -Wextra your_code.cpp -o your_program

# Linux/macOS with Clang  
clang++ -std=c++17 -Wall -Wextra your_code.cpp -o your_program

# Windows with MSVC (Visual Studio Developer Command Prompt)
cl /std:c++17 /EHsc your_code.cpp
```

### Requirements

- **C++17 or later**: Uses modern C++ features
- **CMake 3.14+**: For building tests (uses FetchContent for Google Test)
- **Compiler Support**: GCC 7+, Clang 5+, MSVC 2017+

## Test Suite

The implementation includes a comprehensive test suite with 20+ test cases covering:

- **Basic Operations**: Construction, insertion, removal, iteration
- **Move Semantics**: Move constructors and assignment operators  
- **Multiple Lists**: Objects in multiple lists simultaneously
- **Edge Cases**: Empty lists, single elements, automatic cleanup
- **Stress Testing**: Performance with 1000+ objects
- **Iterator Safety**: Const and non-const iterator behavior
- **Memory Safety**: Automatic unlinking on destruction
- **Node Move Semantics**: Moving nodes between list positions

All tests pass on Windows (MSVC), Linux (GCC), and macOS (Clang).

## Performance Characteristics

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| `push_front()` | O(1) | O(1) |
| `push_back()` | O(1) | O(1) |
| `pop_front()` | O(1) | O(1) |
| `pop_back()` | O(1) | O(1) |
| `insert()` | O(1) | O(1) |
| `erase()` | O(1) | O(1) |
| `clear()` | O(n) | O(1) |
| `front()` | O(1) | O(1) |
| `back()` | O(1) | O(1) |
| Iteration | O(n) | O(1) |

**Memory overhead**: Only 2 pointers per object (16 bytes on 64-bit systems)
**Cache performance**: Excellent due to no additional allocations
**Branch prediction**: Optimized to minimize conditional branches

## Design Philosophy

This implementation prioritizes:
1. **Performance**: Maximum speed through sentinel circular list optimization
2. **Safety**: Compile-time checks and runtime assertions for debug builds
3. **Simplicity**: Clean, readable code with minimal complexity
4. **Modern C++**: Uses C++17 features for type safety and performance

The implementation uses a sentinel circular doubly-linked list design that eliminates most null pointer checks and provides optimal performance for intrusive container operations.

## Files Included

- **`intrusive_list/intrusive_list.hpp`** - Main header-only implementation
- **`test_intrusive_list.cpp`** - Comprehensive unit test suite (20 tests)
- **`intrusive_list/CMakeLists.txt`** - CMake configuration for the library
- **`CMakeLists.txt`** - Root CMake build configuration with Google Test integration
- **`README.md`** - This documentation file
- **`USAGE.md`** - Additional usage examples and patterns
- **`LICENSE`** - License file

## Building and Testing

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build everything
cmake --build . --config Release

# Run tests
ctest

# Or run tests directly  
./Release/test_intrusive_list        # Linux/macOS
.\Release\test_intrusive_list.exe    # Windows
```

### Manual Compilation

If you prefer to compile manually, the library is header-only:

```cpp
#include "intrusive_list/intrusive_list.hpp"
// Your code here - no linking required
```

## Example Usage Patterns

### Simple List
```cpp
struct Item {
    int data;
    dod::list_node link;
};

dod::intrusive_list<&Item::link> items;
Item item1{42};
items.push_back(item1);
```

### RAII Pattern
```cpp
class ManagedItem {
    dod::list_node link_;
public:
    ~ManagedItem() {
        // Automatically unlinked from any lists
    }
};
```

### High-Performance Iteration
```cpp
// Range-based for (recommended)
for (auto& item : my_list) {
    process(item);
}

// Direct iteration for maximum performance
for (auto it = my_list.begin(); it != my_list.end(); ++it) {
    process(*it);
}
```
