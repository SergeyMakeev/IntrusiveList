# Intrusive List Implementation

A modern, C++17 compatible intrusive doubly-linked list implementation. This is a single-header, header-only library that provides efficient intrusive containers.

## Quick Start

```bash
# Clone or download the intrusive_list.hpp file
# Include it in your project
#include "intrusive_list.hpp"

# Or build the complete example and tests
mkdir build && cd build
cmake ..
cmake --build .
ctest                    # Run all tests
```

## Features

- **C++17 Compatible**: Uses modern C++ features like `auto` template parameters and constexpr
- **Header-Only**: Single file implementation - just include `intrusive_list.hpp`
- **STL-Compatible**: Provides standard iterators and container interface
- **Move Semantics**: Full support for move constructors and move assignment
- **Type Safety**: Compile-time checks ensure correct usage
- **Automatic Cleanup**: Objects are automatically unlinked when destroyed
- **Multiple Lists**: Objects can be members of multiple lists simultaneously
- **Cross-Platform**: Tested on Windows (MSVC), Linux (GCC), and macOS (Clang)
- **Comprehensive Tests**: 20+ unit tests with Google Test framework
- **CMake Integration**: Modern CMake build system with automatic dependency management

## Basic Usage

```cpp
#include "intrusive_list.hpp"

// 1. Define your class with a list_node member
struct MyObject {
    int value;
    intrusive::list_node link;  // The intrusive link
    
    MyObject(int v) : value(v) {}
};

// 2. Create a list specifying the object type and link member
intrusive::intrusive_list<MyObject, &MyObject::link> my_list;

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
intrusive::intrusive_list<MyObject, &MyObject::link> int_list;
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

## Multiple Lists Example

Objects can be members of multiple lists by having multiple `list_node` members:

```cpp
struct Employee {
    std::string name;
    intrusive::list_node dept_link;     // For department list
    intrusive::list_node project_link;  // For project list
};

// Same employee can be in both lists
intrusive::intrusive_list<Employee, &Employee::dept_link> engineering;
intrusive::intrusive_list<Employee, &Employee::project_link> project_alpha;

Employee alice("Alice");
engineering.push_back(alice);   // Alice is in engineering dept
project_alpha.push_back(alice); // Alice also works on project alpha
```

## Important Notes

### Safety Considerations
1. **Objects must outlive the list**: The list doesn't manage object lifetime
2. **No duplicate insertion**: An object can only be in one instance of a specific list type at a time
3. **Automatic unlinking**: Objects are automatically removed from lists when destroyed

### Performance Benefits
- **Cache-friendly**: No pointer chasing to separate allocations
- **No allocations**: Zero dynamic memory allocation
- **Constant time operations**: Most operations are O(1)

### Thread Safety
- **Not thread-safe**: Like standard containers, requires external synchronization for concurrent access

```bash
# Linux/macOS with GCC
g++ -std=c++17 -Wall -Wextra example.cpp -o example

# Linux/macOS with Clang  
clang++ -std=c++17 -Wall -Wextra example.cpp -o example

# Windows with MSVC (Visual Studio Developer Command Prompt)
cl /std:c++17 /EHsc example.cpp
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

All tests pass on Windows (MSVC), Linux (GCC), and macOS (Clang).

## Example Program

The included `example.cpp` demonstrates:

- Basic list operations (push, pop, insert, erase)
- Iterator usage and range-based for loops
- Multiple lists with the same objects
- Move semantics and automatic cleanup
- Performance with large numbers of objects

Run the example to see the intrusive list in action:

```bash
# After building with CMake
./example                    # Linux/macOS
.\Debug\example.exe         # Windows
```

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
| Iteration | O(n) | O(1) |

**Memory overhead**: Only 2 pointers per object (16 bytes on 64-bit systems)
**Cache performance**: Excellent due to no additional allocations

## Design Philosophy

This implementation prioritizes:
1. **Simplicity**: Clean, readable code
2. **Performance**: Minimal overhead
3. **Safety**: Compile-time checks and runtime assertions
4. **Usability**: STL-like interface that's familiar to C++ developers

Based on the original draft implementation but modernized and simplified for easier understanding and use.

## Files Included

- **`intrusive_list.hpp`** - Main header-only implementation
- **`example.cpp`** - Complete usage example demonstrating all features
- **`test_intrusive_list.cpp`** - Comprehensive unit test suite (20 tests)
- **`CMakeLists.txt`** - CMake build configuration with Google Test integration
- **`README.md`** - This documentation file

## Building and Testing

### Using CMake (Recommended)

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build everything (tests and example)
cmake --build .

# Run tests
ctest

# Or run tests directly
./intrusive_list_test        # Linux/macOS
.\Debug\intrusive_list_test.exe  # Windows

# Run example
./example                    # Linux/macOS
.\Debug\example.exe         # Windows
```

### Manual Compilation

If you prefer to compile manually: